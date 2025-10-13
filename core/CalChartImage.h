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

#include <cstdint>
#include <memory>
#include <vector>

namespace CalChart::Draw {
struct OpaqueImageData;
}

namespace CalChart {

class Reader;

// Image has the complexity that while there is a platform independent way for representing their info,
// when it comes to drawing with a specific implementation (like wxWidgets), conversions and scaling are
// necessary.  To avoid that we allow an optional "Rendered" object stored along with the data.
struct ImageData {
    int width{};
    int height{};
    std::vector<unsigned char> data;
    std::vector<unsigned char> alpha;
    std::shared_ptr<Draw::OpaqueImageData> render;
};

struct ImageInfo {
    int left{};
    int top{};
    int scaledWidth{};
    int scaledHeight{};
    ImageData data;
};

auto CreateImageInfo(Reader) -> std::pair<ImageInfo, Reader>;
auto Serialize(ImageInfo const&) -> std::vector<std::byte>;

}
