/*
 * animate.cpp
 * Classes for animating shows
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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
#include "CalChartAnimationCommand.h"
#include "CalChartRanges.h"

namespace CalChart {

// make things copiable
AnimationSheet::AnimationSheet(AnimationSheet const& other)
    : mPoints(other.mPoints)
    , mCommands(other.mCommands.size())
    , name(other.name)
    , numbeats(other.numbeats)
{
    std::transform(other.mCommands.cbegin(), other.mCommands.cend(), mCommands.begin(), [](auto&& a) {
        AnimationCommands result(a.size());
        std::transform(a.cbegin(), a.cend(), result.begin(), [](auto&& b) {
            return b->clone();
        });
        return result;
    });
}

AnimationSheet& AnimationSheet::operator=(AnimationSheet other)
{
    swap(other);
    return *this;
}

AnimationSheet::AnimationSheet(AnimationSheet&& other) noexcept
    : mPoints(std::move(other.mPoints))
    , mCommands(std::move(other.mCommands))
    , name(std::move(other.name))
    , numbeats(std::move(other.numbeats))
{
}

AnimationSheet& AnimationSheet::operator=(AnimationSheet&& other) noexcept
{
    AnimationSheet tmp{ std::move(other) };
    swap(tmp);
    return *this;
}

void AnimationSheet::swap(AnimationSheet& other) noexcept
{
    using std::swap;
    swap(mPoints, other.mPoints);
    swap(mCommands, other.mCommands);
    swap(name, other.name);
    swap(numbeats, other.numbeats);
}

auto AnimationSheet::toOnlineViewerJSON(int whichMarcher, Coord startPosition) const -> std::vector<nlohmann::json>
{
    return CalChart::Ranges::ToVector<nlohmann::json>(
        mCommands.at(whichMarcher) | std::views::transform([&startPosition](auto command) mutable {
            auto result = command->toOnlineViewerJSON(startPosition);
            command->ApplyForward(startPosition);
            return result;
        }));
}

}
