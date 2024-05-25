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

#include <format>
#include <iostream>

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

namespace CalChart::Animate {

namespace {
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Command>)
    auto GetBeatsPerCont(Range&& range)
    {
        return range | std::views::transform([](auto cmd) { return NumBeats(cmd); });
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Command>)
    auto GetRunningBeats(Range&& range)
    {
        auto allBeats = CalChart::Ranges::ToVector<beats_t>(GetBeatsPerCont(range));
        auto running = std::vector<beats_t>(allBeats.size());
        std::inclusive_scan(allBeats.begin(), allBeats.end(), running.begin());
        return running;
    }
}

Sheet::Sheet(std::string name, unsigned numBeats, std::vector<std::vector<Animate::Command>> const& commands)
    : mName{ name }
    , mNumBeats{ numBeats }
    , mCommands{ CalChart::Ranges::ToVector<Animate::Commands>(commands | std::views::transform([](auto item) { return Animate::Commands(item); })) }
{
    mCollisions = FindAllCollisions();
}

auto Sheet::MarcherInfoAtBeat(size_t whichMarcher, beats_t beat) const -> CalChart::Animate::MarcherInfo
{
    return mCommands.at(whichMarcher).MarcherInfoAtBeat(beat);
}

namespace {
    // get all the positions at a beat:

}

auto Sheet::GetAllBeatsWithCollisions() const -> std::set<beats_t>
{
    return std::reduce(mCollisions.begin(), mCollisions.end(), std::set<beats_t>{}, [](auto acc, auto item) {
        acc.insert(std::get<1>(std::get<0>(item)));
        return acc;
    });
}

auto Sheet::GetAllMarchersWithCollisionAtBeat(beats_t beat) const -> std::set<size_t>
{
    return std::reduce(mCollisions.begin(), mCollisions.end(), std::set<size_t>{}, [beat](auto acc, auto item) {
        if (std::get<1>(std::get<0>(item)) == beat) {
            acc.insert(std::get<0>(std::get<0>(item)));
        }
        return acc;
    });
}

namespace {

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Coord>)
    auto FindAllCollisions(Range range) -> std::map<size_t, Coord::CollisionType>
    {
        auto points = CalChart::Ranges::ToVector<CalChart::Coord>(range);
        auto results = std::map<size_t, Coord::CollisionType>{};
        for (auto i : std::views::iota(0UL, points.size() - 1)) {
            for (auto j : std::views::iota(i + 1, points.size())) {
                auto collisionResult = points.at(i).DetectCollision(points.at(j));
                if (collisionResult != Coord::CollisionType::none) {
                    if (!results.contains(i) || results[i] < collisionResult) {
                        results[i] = collisionResult;
                    }
                    if (!results.contains(j) || results[j] < collisionResult) {
                        results[j] = collisionResult;
                    }
                }
            }
        }
        return results;
    }
}

auto Sheet::FindAllCollisions() const -> std::map<std::tuple<size_t, beats_t>, Coord::CollisionType>
{
    auto results = std::map<std::tuple<size_t, beats_t>, Coord::CollisionType>{};
    for (auto beat : std::views::iota(0U, GetNumBeats())) {
        auto allCollisions = Animate::FindAllCollisions(AllMarcherInfoAtBeat(beat) | std::views::transform([](auto info) { return info.mPosition; }));
        results = std::reduce(allCollisions.begin(), allCollisions.end(), results, [beat](auto acc, auto item) {
            auto [where, collision] = item;
            acc[{ where, beat }] = collision;
            return acc;
        });
    }
    return results;
}

namespace {
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Sheet>)
    auto GetBeatsPerSheet(Range&& range)
    {
        return range | std::views::transform([](auto sheet) { return sheet.GetNumBeats(); });
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Sheet>)
    auto GetRunningBeats(Range&& range)
    {
        auto allBeats = CalChart::Ranges::ToVector<beats_t>(GetBeatsPerSheet(range));
        auto running = std::vector<beats_t>(allBeats.size());
        std::inclusive_scan(allBeats.begin(), allBeats.end(), running.begin());
        return running;
    }
}

Sheets::Sheets(std::vector<Sheet> const& sheets)
    : mSheets(sheets)
    , mRunningBeatCount{ GetRunningBeats(sheets) }
{
}

auto Sheets::TotalBeats() const -> beats_t
{
    if (mRunningBeatCount.empty()) {
        return 0;
    }
    return mRunningBeatCount.back();
}

auto Sheets::BeatToSheetOffsetAndBeat(beats_t beat) const -> std::tuple<size_t, beats_t>
{
    auto where = std::ranges::find_if(mRunningBeatCount, [beat](auto thisBeat) { return beat < thisBeat; });
    if (where == mRunningBeatCount.end()) {
        return { mRunningBeatCount.size(), beat - TotalBeats() };
    }
    auto index = std::distance(mRunningBeatCount.begin(), where);
    return { index, beat - (*where - mSheets.at(index).GetNumBeats()) };
}

auto Sheets::MarcherInfoAtBeat(beats_t beat, int whichMarcher) const -> MarcherInfo
{
    auto [which, newBeat] = BeatToSheetOffsetAndBeat(beat);
    if (which >= mSheets.size()) {
        return {};
    }
    return mSheets.at(which).MarcherInfoAtBeat(whichMarcher, newBeat);
}

}
