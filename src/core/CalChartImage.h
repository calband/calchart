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
#include <vector>

namespace CalChart {

class Reader;

struct ImageData {
    int image_width, image_height;
    std::vector<unsigned char> data;
    std::vector<unsigned char> alpha;
};

struct ImageInfo {
    int left, top;
    int scaled_width, scaled_height;
    ImageData data;
};

auto CreateImageInfo(Reader) -> std::pair<ImageInfo, Reader>;
auto Serialize(ImageInfo const&) -> std::vector<std::byte>;

}
