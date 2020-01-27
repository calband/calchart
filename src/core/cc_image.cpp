/*
 * cc_image.cpp
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

#include "cc_image.h"
#include "cc_fileformat.h"

namespace CalChart {

ImageData::ImageData(int left, int top, int scaled_width, int scaled_height, int image_width, int image_height, std::vector<unsigned char> const& data, std::vector<unsigned char> const& alpha)
    : left(left)
    , top(top)
    , scaled_width(scaled_width)
    , scaled_height(scaled_height)
    , image_width(image_width)
    , image_height(image_height)
    , data(data)
    , alpha(alpha)
{
}

ImageData::ImageData(uint8_t const*& d)
{
    left = get_big_long(d);
    d += 4;
    top = get_big_long(d);
    d += 4;
    scaled_width = get_big_long(d);
    d += 4;
    scaled_height = get_big_long(d);
    d += 4;
    image_width = get_big_long(d);
    d += 4;
    image_height = get_big_long(d);
    d += 4;
    auto data_size = get_big_long(d);
    d += 4;
    data.assign(d, d + data_size);
    d += data_size;
    auto alpha_size = get_big_long(d);
    d += 4;
    alpha.assign(d, d + alpha_size);
    d += alpha_size;
}
}
