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
    return { ImageInfo{ left, top, scaled_width, scaled_height, ImageData{ image_width, image_height, data, alpha, nullptr } }, reader };
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
}
