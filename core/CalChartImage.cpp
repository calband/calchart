/*
 * CalChartImage.cpp
 */

/*
   Copyright (C) 2017-2024  Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CalChartImage.h"
#include "CalChartFileFormat.h"
#include "CalChartUtils.h"
#include <stdexcept>

namespace {

auto SerializeImageData(CalChart::ImageData const& image) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    CalChart::Parser::Append(result, uint32_t(image.width));
    CalChart::Parser::Append(result, uint32_t(image.height));
    // we know data size, but let's put it in anyways
    CalChart::Parser::Append(result, uint32_t(image.data.size()));
    CalChart::Parser::Append(result, image.data);
    // alpha could be zero
    CalChart::Parser::Append(result, uint32_t(image.alpha.size()));
    CalChart::Parser::Append(result, image.alpha);
    return result;
}

auto DeserializeImageData(CalChart::Reader& reader) -> std::pair<CalChart::ImageData, CalChart::Reader>
{
    auto image_width = reader.Get<int32_t>();
    auto image_height = reader.Get<int32_t>();
    auto data = reader.GetVector<unsigned char>();
    auto alpha = reader.GetVector<unsigned char>();

    return { CalChart::ImageData{ image_width, image_height, data, alpha, nullptr }, reader };
}

auto ImageDataToJson(CalChart::ImageData const& image) -> nlohmann::json
{
    auto byteData = std::vector<std::byte>{};
    byteData.reserve(image.data.size());
    std::transform(image.data.begin(), image.data.end(), std::back_inserter(byteData),
        [](unsigned char c) { return static_cast<std::byte>(c); });
    auto byteAlpha = std::vector<std::byte>{};
    byteAlpha.reserve(image.alpha.size());
    std::transform(image.alpha.begin(), image.alpha.end(), std::back_inserter(byteAlpha),
        [](unsigned char c) { return static_cast<std::byte>(c); });
    return nlohmann::json{
        { "width", image.width },
        { "height", image.height },
        { "data", CalChart::EncodeBase64(byteData) },
        { "alpha", CalChart::EncodeBase64(byteAlpha) },
    };
}

auto ImageDataFromJSON(nlohmann::json const& dataJson) -> CalChart::ImageData
{
    CalChart::ImageData data{};
    data.width = dataJson.at("width").get<int>();
    data.height = dataJson.at("height").get<int>();
    auto byteData = CalChart::DecodeBase64(dataJson.at("data").get<std::string>());
    auto byteAlpha = CalChart::DecodeBase64(dataJson.at("alpha").get<std::string>());
    data.data.reserve(byteData.size());
    std::transform(byteData.begin(), byteData.end(), std::back_inserter(data.data),
        [](std::byte b) { return static_cast<unsigned char>(b); });
    data.alpha.reserve(byteAlpha.size());
    std::transform(byteAlpha.begin(), byteAlpha.end(), std::back_inserter(data.alpha),
        [](std::byte b) { return static_cast<unsigned char>(b); });

    return data;
}

auto CreateImageInfoImpl(CalChart::Reader reader, CalChart::ImageData image, int left, int top, int scaled_width,
    int scaled_height) -> std::pair<CalChart::ImageInfo, CalChart::Reader>
{
    scaled_width = (scaled_width == 0) ? image.width : scaled_width;
    scaled_height = (scaled_height == 0) ? image.height : scaled_height;
    return { CalChart::ImageInfo{ left, top, scaled_width, scaled_height, std::move(image) }, reader };
}
}

namespace CalChart {

auto Serialize(ImageInfo const& image, ImageRegistry& registry) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, static_cast<int32_t>(image.left));
    Parser::Append(result, static_cast<int32_t>(image.top));
    Parser::Append(result, static_cast<int32_t>(image.scaledWidth));
    Parser::Append(result, static_cast<int32_t>(image.scaledHeight));
    auto image_id = registry.Register(image.data);
    Parser::Append(result, static_cast<uint32_t>(image_id.value));
    return result;
}

auto CreateImageInfo(Reader reader) -> std::pair<ImageInfo, Reader>
{
    auto left = reader.Get<int32_t>();
    auto top = reader.Get<int32_t>();
    auto scaled_width = reader.Get<int32_t>();
    auto scaled_height = reader.Get<int32_t>();
    auto [image, new_reader] = DeserializeImageData(reader);
    return CreateImageInfoImpl(new_reader, std::move(image), left, top, scaled_width, scaled_height);
}

auto CreateRegisteredImageInfo(Reader reader, ImageRegistry const& registry) -> std::pair<ImageInfo, Reader>
{
    auto left = reader.Get<int32_t>();
    auto top = reader.Get<int32_t>();
    auto scaled_width = reader.Get<int32_t>();
    auto scaled_height = reader.Get<int32_t>();
    auto image_id = reader.Get<uint32_t>();
    return CreateImageInfoImpl(reader, registry.Get(ImageKey{ image_id }), left, top, scaled_width, scaled_height);
}

auto ImageInfoToJSON(CalChart::ImageInfo const& image, CalChart::ImageRegistry& registry) -> nlohmann::json
{
    auto image_id = registry.Register(image.data);

    return nlohmann::json{
        { "left", image.left },
        { "top", image.top },
        { "scaledWidth", image.scaledWidth },
        { "scaledHeight", image.scaledHeight },
        { "image_id", image_id.value },
    };
}

auto ImageInfoFromJSON(nlohmann::json const& json, ImageRegistry const& registry) -> ImageInfo
{
    if (!json.is_object()) {
        throw std::invalid_argument("ImageInfo JSON must be an object");
    }

    if (!json.contains("left") || !json.contains("top") || !json.contains("scaledWidth")
        || !json.contains("scaledHeight") || !json.contains("image_id")) {
        throw std::invalid_argument("ImageInfo JSON missing required fields");
    }

    ImageInfo image;
    image.left = json.at("left").get<int>();
    image.top = json.at("top").get<int>();
    image.scaledWidth = json.at("scaledWidth").get<int>();
    image.scaledHeight = json.at("scaledHeight").get<int>();

    auto image_id = json.at("image_id").get<size_t>();
    image.data = registry.Get(ImageKey{ image_id });

    return image;
}

auto Serialize(ImageRegistry const& registry) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, static_cast<uint32_t>(registry.Size()));
    for (std::size_t i = 0; i < registry.Size(); ++i) {
        Parser::Append(result, SerializeImageData(registry.Get(ImageKey{ i })));
    }
    return result;
}

auto CreateImageRegistry(Reader reader) -> std::pair<ImageRegistry, Reader>
{
    auto images = reader.Get<uint32_t>();
    ImageRegistry registry;
    for (std::size_t i = 0; i < images; ++i) {
        auto [image_data, new_reader] = DeserializeImageData(reader);
        registry.Register(std::move(image_data));
        reader = new_reader;
    }
    return { std::move(registry), reader };
}

auto ImageRegistryToJSON(ImageRegistry const& registry) -> nlohmann::json
{
    nlohmann::json arr = nlohmann::json::array();
    for (std::size_t i = 0; i < registry.Size(); ++i) {
        arr.push_back(ImageDataToJson(registry.Get(ImageKey{ i })));
    }
    return arr;
}

auto ImageRegistryFromJSON(nlohmann::json const& json) -> ImageRegistry
{
    if (!json.is_array()) {
        throw std::invalid_argument("ImageRegistry JSON must be an array");
    }

    ImageRegistry registry;
    for (auto const& item : json) {
        auto image_data = ImageDataFromJSON(item);
        registry.Register(std::move(image_data));
    }
    return registry;
}

} // namespace CalChart
