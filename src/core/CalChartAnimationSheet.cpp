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

namespace CalChart::Animate {

namespace {
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, Command>)
    auto GetBeatsPerCont(Range&& range)
    {
        return range | std::views::transform([](auto cmd) { return NumBeats(cmd); });
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, Command>)
    auto GetRunningBeats(Range&& range)
    {
        auto allBeats = CalChart::Ranges::ToVector<beats_t>(GetBeatsPerCont(range));
        auto running = std::vector<beats_t>(allBeats.size());
        std::inclusive_scan(allBeats.begin(), allBeats.end(), running.begin());
        return running;
    }
}

Sheet::Sheet(std::string name, unsigned numBeats, std::vector<CompileResult> const& commands)
    : mName{ name }
    , mNumBeats{ numBeats }
    , mCommands{ CalChart::Ranges::ToVector<Commands>(commands | std::views::transform([](auto&& item) { return Commands(item.first); })) }
    , mErrors{ [](auto&& cmds) {
        auto enumerated = CalChart::Ranges::enumerate_view(cmds | std::views::transform([](auto&& item) { return item.second; }));
        return std::accumulate(std::begin(enumerated), std::end(enumerated), Errors{}, [](auto&& acc, auto&& item) {
            for (auto& error : std::get<1>(item)) {
                acc[error].insert(std::get<0>(item));
            }
            return acc;
        });
    }(commands) }
{
    mCollisions = FindAllCollisions();
}

auto Sheet::MarcherInfoAtBeat(size_t whichMarcher, beats_t beat) const -> MarcherInfo
{
    return mCommands.at(whichMarcher).MarcherInfoAtBeat(beat);
}

auto Sheet::CollisionAtBeat(size_t whichMarcher, beats_t beat) const -> CalChart::Coord::CollisionType
{
    if (auto where = mCollisions.find({ whichMarcher, beat }); where != mCollisions.end()) {
        return where->second;
    }
    return Coord::CollisionType::none;
}

auto Sheet::GetAllBeatsWithCollisions() const -> std::set<beats_t>
{
    return std::accumulate(mCollisions.begin(), mCollisions.end(), std::set<beats_t>{}, [](auto acc, auto item) {
        acc.insert(std::get<1>(std::get<0>(item)));
        return acc;
    });
}

auto Sheet::GetAllMarchersWithCollisionAtBeat(beats_t beat) const -> CalChart::SelectionList
{
    return std::accumulate(mCollisions.begin(), mCollisions.end(), CalChart::SelectionList{}, [beat](auto acc, auto item) {
        if (std::get<1>(std::get<0>(item)) == beat) {
            acc.insert(std::get<0>(std::get<0>(item)));
        }
        return acc;
    });
}

auto Sheet::toOnlineViewerJSON() const -> std::vector<std::vector<nlohmann::json>>
{
    return CalChart::Ranges::ToVector<std::vector<nlohmann::json>>(
        mCommands | std::views::transform([](auto&& which) {
            return which.toOnlineViewerJSON();
        }));
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
        results = std::accumulate(allCollisions.begin(), allCollisions.end(), results, [beat](auto acc, auto item) {
            auto [where, collision] = item;
            acc[{ where, beat }] = collision;
            return acc;
        });
    }
    return results;
}

auto Sheet::DebugAnimateInfoAtBeat(beats_t beat, bool ignoreCollision) const -> std::vector<std::string>
{
    return CalChart::Ranges::ToVector<std::string>(
        std::views::iota(0UL, mCommands.size()) | std::views::transform([this, beat, ignoreCollision](auto whichMarcher) {
            auto marcherInfo = MarcherInfoAtBeat(whichMarcher, beat);
            auto collision = CollisionAtBeat(whichMarcher, beat);
            if (ignoreCollision) {
                collision = CalChart::Coord::CollisionType::none;
            }
            std::ostringstream each_string;
            each_string << "pt " << whichMarcher << ": (" << marcherInfo.mPosition.x << ", "
                        << marcherInfo.mPosition.y << "), dir=" << CalChart::Degree{ marcherInfo.mFacingDirection }.getValue()
                        << ((collision != CalChart::Coord::CollisionType::none) ? ", collision!" : "");
            return each_string.str();
        }));
}

namespace {
    // Because we need to draw sprites and other animations without overlap, sort them so the "closer" ones are first
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, Info>)
    auto SortForSprites(Range input)
    {
        std::ranges::sort(input, [](auto&& a, auto&& b) {
            return a.mMarcherInfo.mPosition < b.mMarcherInfo.mPosition;
        });
        return input;
    }

}

auto Sheet::AllAnimateInfoAtBeat(beats_t beat) const -> std::vector<Info>
{
    auto animates = CalChart::Ranges::ToVector<Info>(std::views::iota(0UL, mCommands.size()) | std::views::transform([this, beat](auto whichMarcher) -> Info {
        return {
            static_cast<int>(whichMarcher),
            CollisionAtBeat(whichMarcher, beat),
            MarcherInfoAtBeat(whichMarcher, beat)
        };
    }));
    return animates;
}

namespace {
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, Sheet>)
    auto GetBeatsPerSheet(Range&& range)
    {
        return range | std::views::transform([](auto sheet) { return sheet.GetNumBeats(); });
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, Sheet>)
    auto GetRunningBeats(Range&& range)
    {
        auto allBeats = CalChart::Ranges::ToVector<beats_t>(GetBeatsPerSheet(range));
        auto running = std::vector<beats_t>(allBeats.size());
        std::inclusive_scan(allBeats.begin(), allBeats.end(), running.begin());
        return running;
    }
}

Sheets::Sheets(std::vector<Sheet> const& sheets, std::vector<unsigned> const& showSheetToAnimationSheet)
    : mSheets(sheets)
    , mRunningBeatCount{ GetRunningBeats(sheets) }
    , mShowSheetToAnimationSheet{ showSheetToAnimationSheet }
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

auto Sheets::CollisionAtBeat(beats_t beat, int whichMarcher) const -> Coord::CollisionType
{
    // this is because Calchart-3.7 and earlier would skip the first beat for collision detection.
    if (beat == 0) {
        return {};
    }
    auto [which, newBeat] = BeatToSheetOffsetAndBeat(beat);
    if (which >= mSheets.size()) {
        return {};
    }
    return mSheets.at(which).CollisionAtBeat(whichMarcher, newBeat);
}

auto Sheets::BeatHasCollision(beats_t beat) const -> bool
{
    auto [which, newBeat] = BeatToSheetOffsetAndBeat(beat);
    if (which >= mSheets.size()) {
        return false;
    }
    return mSheets.at(which).GetAllBeatsWithCollisions().contains(beat);
}

auto Sheets::DebugAnimateInfoAtBeat(beats_t beat) const -> std::pair<std::string, std::vector<std::string>>
{
    auto [whichSheet, newBeat] = BeatToSheetOffsetAndBeat(beat);
    std::ostringstream output;
    output << GetSheetName(whichSheet) << " (" << whichSheet << " of " << mSheets.size() << ")\n";
    output << "beat " << newBeat << " of " << BeatForSheet(whichSheet) << "\n";
    auto each = mSheets.at(whichSheet).DebugAnimateInfoAtBeat(newBeat, beat == 0);
    return { output.str(), each };
}

auto Sheets::AllAnimateInfoAtBeat(beats_t whichBeat) const -> std::vector<Info>
{
    auto [whichSheet, newBeat] = BeatToSheetOffsetAndBeat(whichBeat);
    return mSheets.at(whichSheet).AllAnimateInfoAtBeat(newBeat);
}

auto Sheets::toOnlineViewerJSON() const -> std::vector<std::vector<std::vector<nlohmann::json>>>
{
    return CalChart::Ranges::ToVector<std::vector<std::vector<nlohmann::json>>>(
        mSheets | std::views::transform([](auto&& sheet) {
            return sheet.toOnlineViewerJSON();
        }));
}

}
