/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartAnimation.h"
#include "CalChartDrawCommand.h"
#include <array>
#include <vector>
#include <wx/wx.h>

namespace wxCalChart {
struct BitmapHolder;
}
namespace CalChart {
class Configuration;
}

class AnimationSprites {
public:
    void RegenerateImages(CalChart::Configuration const& config);
    auto GetImage(CalChart::Radian angle, CalChart::Animation::ImageBeat imageBeat, bool selected) const -> std::tuple<std::shared_ptr<CalChart::Draw::OpaqueImageData>, CalChart::Coord>;

private:
    static constexpr auto kAngles = 8;
    double mScaleSize = 0;
    using BitmapSize_t = std::tuple<std::shared_ptr<wxCalChart::BitmapHolder>, CalChart::Coord>;
    std::array<BitmapSize_t, kAngles * CalChart::toUType(CalChart::Animation::ImageBeat::Size)> mSpriteCalChartImages;
    std::array<BitmapSize_t, kAngles * CalChart::toUType(CalChart::Animation::ImageBeat::Size)> mSelectedSpriteCalChartImages;
};