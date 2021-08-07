/*
 * CalChartImage.cpp
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

#include "CalChartImage.h"
#include "CalChartFileFormat.h"

namespace CalChart {

std::pair<ImageData, uint8_t const*> CreateImageData(uint8_t const* d)
{
    auto left = static_cast<int>(get_big_long(d));
    d += 4;
    auto top = static_cast<int>(get_big_long(d));
    d += 4;
    auto scaled_width = static_cast<int>(get_big_long(d));
    d += 4;
    auto scaled_height = static_cast<int>(get_big_long(d));
    d += 4;
    auto image_width = static_cast<int>(get_big_long(d));
    d += 4;
    auto image_height = static_cast<int>(get_big_long(d));
    d += 4;
    auto data_size = get_big_long(d);
    d += 4;
    auto data = std::vector<unsigned char>(d, d + data_size);
    d += data_size;
    auto alpha_size = get_big_long(d);
    d += 4;
    auto alpha = std::vector<unsigned char>(d, d + alpha_size);
    scaled_width = (scaled_width == 0) ? image_width : scaled_width;
    scaled_height = (scaled_height == 0) ? image_height : scaled_height;
    return { ImageData{ left, top, scaled_width, scaled_height, image_width, image_height, data, alpha }, d };
}

std::vector<uint8_t> Serialize(ImageData const& image)
{
    std::vector<uint8_t> result;
    Parser::Append(result, uint32_t(image.left));
    Parser::Append(result, uint32_t(image.top));
    Parser::Append(result, uint32_t(image.scaled_width));
    Parser::Append(result, uint32_t(image.scaled_height));
    Parser::Append(result, uint32_t(image.image_width));
    Parser::Append(result, uint32_t(image.image_height));
    // we know data size, but let's put it in anyways
    Parser::Append(result, uint32_t(image.data.size()));
    Parser::Append(result, image.data);
    // alpha could be zero
    Parser::Append(result, uint32_t(image.alpha.size()));
    Parser::Append(result, image.alpha);
    return result;
}
}
