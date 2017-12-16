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

std::vector<uint8_t> ImageData::Serialize() const
{
    std::vector<uint8_t> result;
    Parser::Append(result, uint32_t(left));
    Parser::Append(result, uint32_t(top));
    Parser::Append(result, uint32_t(scaled_width));
    Parser::Append(result, uint32_t(scaled_height));
    Parser::Append(result, uint32_t(image_width));
    Parser::Append(result, uint32_t(image_height));
    // we know data size, but let's put it in anyways
    Parser::Append(result, uint32_t(data.size()));
    Parser::Append(result, data);
    // alpha could be zero
    Parser::Append(result, uint32_t(alpha.size()));
    Parser::Append(result, alpha);
    return result;
}
}
