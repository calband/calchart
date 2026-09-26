#pragma once
/*
 * CalChartImage.h
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

#include "CalChartDataRegistry.hpp"

#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <vector>

namespace CalChart::Draw {
struct OpaqueImageData;
}

namespace CalChart {

class Reader;
struct ImageData;

// Image has the complexity that while there is a platform independent way for representing their info,
// when it comes to drawing with a specific implementation (like wxWidgets), conversions and scaling are
// necessary.  To avoid that we allow an optional "Rendered" object stored along with the data.
struct ImageData {
    int width{};
    int height{};
    std::vector<unsigned char> data;
    std::vector<unsigned char> alpha;
    std::shared_ptr<Draw::OpaqueImageData> render;

private:
    // Sample bytes instead of hashing the whole buffer — cheap, and any
    // collisions get caught by the exact equality check afterward.
    static std::size_t HashBytes(const std::vector<unsigned char>& bytes)
    {
        std::size_t h = bytes.size();
        std::size_t step = std::max<std::size_t>(1, bytes.size() / 256);
        for (std::size_t i = 0; i < bytes.size(); i += step) {
            h ^= static_cast<std::size_t>(bytes[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

struct ImageDataHash {
    std::size_t operator()(const ImageData& img) const noexcept
    {
        std::size_t h = std::hash<int>{}(img.width);
        h = h * 31 + std::hash<int>{}(img.height);
        h = h * 31 + HashBytes(img.data);
        h = h * 31 + HashBytes(img.alpha);
        return h;
    }

private:
    // Sample bytes instead of hashing the whole buffer — cheap, and any
    // collisions get caught by the exact equality check afterward.
    static std::size_t HashBytes(const std::vector<unsigned char>& bytes)
    {
        std::size_t h = bytes.size();
        std::size_t step = std::max<std::size_t>(1, bytes.size() / 256);
        for (std::size_t i = 0; i < bytes.size(); i += step) {
            h ^= static_cast<std::size_t>(bytes[i]) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }
};

struct ImageDataEqual {
    bool operator()(const ImageData& a, const ImageData& b) const noexcept
    {
        return a.width == b.width && a.height == b.height && a.data == b.data && a.alpha == b.alpha;
    }
};

using ImageRegistry = CalChart::DataRegistry<ImageData, ImageDataHash, ImageDataEqual>;
using ImageKey = CalChart::DataKey;

struct ImageInfo {
    int left{};
    int top{};
    int scaledWidth{};
    int scaledHeight{};
    ImageData data;
};

auto Serialize(ImageInfo const&, ImageRegistry&) -> std::vector<std::byte>;
auto CreateImageInfo(Reader) -> std::pair<ImageInfo, Reader>;
auto CreateRegisteredImageInfo(Reader, ImageRegistry const&) -> std::pair<ImageInfo, Reader>;

[[nodiscard]] auto ImageInfoToJSON(ImageInfo const&, ImageRegistry&) -> nlohmann::json;
[[nodiscard]] auto ImageInfoFromJSON(nlohmann::json const&, ImageRegistry const&) -> ImageInfo;

[[nodiscard]] auto Serialize(ImageRegistry const&) -> std::vector<std::byte>;
[[nodiscard]] auto CreateImageRegistry(Reader) -> std::pair<ImageRegistry, Reader>;
[[nodiscard]] auto ImageRegistryToJSON(ImageRegistry const&) -> nlohmann::json;
[[nodiscard]] auto ImageRegistryFromJSON(nlohmann::json const&) -> ImageRegistry;

} // namespace CalChart
