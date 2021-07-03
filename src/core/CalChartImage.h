#pragma once
/*
 * CalChartImage.h
 */

/*
   Copyright (C) 2017  Richard Michael Powell

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

#include <vector>
#include <cstdint>

namespace CalChart {

struct ImageData {
    int left, top;
    int scaled_width, scaled_height;
    int image_width, image_height;
    std::vector<unsigned char> data;
    std::vector<unsigned char> alpha;
    ImageData(int left, int top, int scaled_width, int scaled_height, int image_width, int image_height, std::vector<unsigned char> const& data, std::vector<unsigned char> const& alpha);
    ImageData(uint8_t const* d);
    std::vector<uint8_t> Serialize() const;
};
}
