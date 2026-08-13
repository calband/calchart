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

namespace CalChart {

auto CreateImageInfo(Reader reader) -> std::pair<ImageInfo, Reader>
{
    auto left = reader.Get<int32_t>();
    auto top = reader.Get<int32_t>();
    auto scaled_width = reader.Get<int32_t>();
    auto scaled_height = reader.Get<int32_t>();
    auto image_width = reader.Get<int32_t>();
    auto image_height = reader.Get<int32_t>();
    auto data = reader.GetVector<unsigned char>();
    auto alpha = reader.GetVector<unsigned char>();
    scaled_width = (scaled_width == 0) ? image_width : scaled_width;
    scaled_height = (scaled_height == 0) ? image_height : scaled_height;
    return { ImageInfo{
                 left, top, scaled_width, scaled_height, ImageData{ image_width, image_height, data, alpha, nullptr } },
        reader };
}

auto Serialize(ImageInfo const& image) -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, uint32_t(image.left));
    Parser::Append(result, uint32_t(image.top));
    Parser::Append(result, uint32_t(image.scaledWidth));
    Parser::Append(result, uint32_t(image.scaledHeight));
    Parser::Append(result, uint32_t(image.data.width));
    Parser::Append(result, uint32_t(image.data.height));
    // we know data size, but let's put it in anyways
    Parser::Append(result, uint32_t(image.data.data.size()));
    Parser::Append(result, image.data.data);
    // alpha could be zero
    Parser::Append(result, uint32_t(image.data.alpha.size()));
    Parser::Append(result, image.data.alpha);
    return result;
}

auto ImageInfoToJSON(ImageInfo const& image) -> nlohmann::json
{
    auto byteData = std::vector<std::byte>{};
    byteData.reserve(image.data.data.size());
    std::transform(image.data.data.begin(), image.data.data.end(), std::back_inserter(byteData),
        [](unsigned char c) { return static_cast<std::byte>(c); });
    auto byteAlpha = std::vector<std::byte>{};
    byteAlpha.reserve(image.data.alpha.size());
    std::transform(image.data.alpha.begin(), image.data.alpha.end(), std::back_inserter(byteAlpha),
        [](unsigned char c) { return static_cast<std::byte>(c); });

    return nlohmann::json{
        { "left", image.left },
        { "top", image.top },
        { "scaledWidth", image.scaledWidth },
        { "scaledHeight", image.scaledHeight },
        {
            "data",
            {
                { "width", image.data.width },
                { "height", image.data.height },
                { "data", EncodeBase64(byteData) },
                { "alpha", EncodeBase64(byteAlpha) },
            },
        },
    };
}

auto ImageInfoFromJSON(nlohmann::json const& json) -> ImageInfo
{
    if (!json.is_object()) {
        throw std::invalid_argument("ImageInfo JSON must be an object");
    }

    if (!json.contains("left") || !json.contains("top") || !json.contains("scaledWidth")
        || !json.contains("scaledHeight") || !json.contains("data")) {
        throw std::invalid_argument("ImageInfo JSON missing required fields");
    }

    auto const& dataJson = json.at("data");
    if (!dataJson.is_object() || !dataJson.contains("width") || !dataJson.contains("height")
        || !dataJson.contains("data") || !dataJson.contains("alpha")) {
        throw std::invalid_argument("ImageInfo data JSON missing required fields");
    }

    ImageInfo image;
    image.left = json.at("left").get<int>();
    image.top = json.at("top").get<int>();
    image.scaledWidth = json.at("scaledWidth").get<int>();
    image.scaledHeight = json.at("scaledHeight").get<int>();

    image.data.width = dataJson.at("width").get<int>();
    image.data.height = dataJson.at("height").get<int>();
    auto byteData = DecodeBase64(dataJson.at("data").get<std::string>());
    auto byteAlpha = DecodeBase64(dataJson.at("alpha").get<std::string>());
    image.data.data.reserve(byteData.size());
    std::transform(byteData.begin(), byteData.end(), std::back_inserter(image.data.data),
        [](std::byte b) { return static_cast<unsigned char>(b); });
    image.data.alpha.reserve(byteAlpha.size());
    std::transform(byteAlpha.begin(), byteAlpha.end(), std::back_inserter(image.data.alpha),
        [](std::byte b) { return static_cast<unsigned char>(b); });
    image.data.render = nullptr;

    return image;
}
}
