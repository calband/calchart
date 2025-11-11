/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

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

#include "AnimationSprites.hpp"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "platconf.h"
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace {

// split the source image into a number of horizontal images
auto GenerateSpriteImages(wxImage const& image, int numberImages, int imageX, int imageY, double scale)
{
    return std::views::iota(0, numberImages) | std::views::transform([&image, imageX, imageY, scale](auto i) {
        auto newImage = image.GetSubImage({ i * imageX + 0, 0, imageX, imageY });
        return newImage.Scale(newImage.GetWidth() * scale, newImage.GetHeight() * scale);
    });
}

}

void AnimationSprites::RegenerateImages(CalChart::Configuration const& config)
{
    auto spriteScale = config.Get_SpriteBitmapScale();
    if (spriteScale == mScaleSize) {
        return;
    }
    mScaleSize = spriteScale;
#if defined(__APPLE__) && (__APPLE__)
    const static auto kImageDir = wxStandardPaths::Get().GetResourcesDir().Append("/default_sprite_strip.png");
#else
    const static auto kImageDir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR "resources" PATH_SEPARATOR "default_sprite_strip.png");
#endif
    auto image = []() {
        wxImage image;
        if (!image.LoadFile(kImageDir)) {
            wxLogError("Couldn't load image from " + kImageDir + ".");
            return image;
        }
        return image;
    }();
    // now slice up all the images
    constexpr auto image_X = 64;
    constexpr auto image_Y = 128;
    auto images = GenerateSpriteImages(image, mSpriteCalChartImages.size(), image_X, image_Y, mScaleSize);
    std::transform(images.begin(), images.end(), mSpriteCalChartImages.begin(), [](auto&& image) {
        return BitmapSize_t{ std::make_shared<wxCalChart::BitmapHolder>(image), wxCalChart::toCoord(image.GetSize()) };
    });
    std::transform(images.begin(), images.end(), mSelectedSpriteCalChartImages.begin(), [](auto&& image) {
        return BitmapSize_t{ std::make_shared<wxCalChart::BitmapHolder>(image.ConvertToGreyscale()), wxCalChart::toCoord(image.GetSize()) };
    });
}

auto AnimationSprites::GetImage(CalChart::Radian angle, CalChart::Animation::ImageBeat imageBeat, bool selected) const -> std::tuple<std::shared_ptr<CalChart::Draw::OpaqueImageData>, CalChart::Coord>
{
    auto imageIndex = CalChart::AngleToQuadrant(angle) + CalChart::toUType(imageBeat) * 8;
    return selected ? mSelectedSpriteCalChartImages[imageIndex] : mSpriteCalChartImages[imageIndex];
}
