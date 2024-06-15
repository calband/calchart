#pragma once
/*
 * CalChartAnimation.h
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

#include "CalChartAnimationCommand.h"
#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include "CalChartPoint.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <ranges>
#include <set>
#include <string>
#include <vector>

/**
 * Animation Sheet
 * Continuities can be broken down into 3 distinct types:
 *  MarkTime: A direction to be facing
 *  Moving: A vector along which to be moving (indicating how far to move each point)
 *  Rotate: A point which to rotate, radius, start and end angles
 * AnimationCommand is an object that represents a particular part of a continuity.  When we decompose
 * continuities into these parts, we can then "transform" a point from a starting position to the end of the
 * Animation by "stepping" it along each AnimationCommand
 */

namespace CalChart {

class AnimationCommand;

using AnimationCommands = std::vector<std::shared_ptr<AnimationCommand>>;

// AnimationSheet is a snapshot of CalChartSheet
class AnimationSheet {
public:
    AnimationSheet(std::vector<Coord> const& thePoints, std::vector<AnimationCommands> const& theCommands, std::string const& s, unsigned beats)
        : mPoints(thePoints)
        , mCommands(theCommands)
        , name(s)
        , numbeats(beats)
    {
    }

    // make things copiable
    AnimationSheet(AnimationSheet const&);
    AnimationSheet& operator=(AnimationSheet);
    AnimationSheet(AnimationSheet&&) noexcept;
    AnimationSheet& operator=(AnimationSheet&&) noexcept;
    void swap(AnimationSheet&) noexcept;

    auto GetName() const { return name; }
    auto GetNumBeats() const { return numbeats; }
    auto GetPoints() const { return mPoints; }
    auto GetCommands(int which) const { return mCommands.at(which); }
    auto GetCommandsBegin(int which) const
    {
        return mCommands.at(which).begin();
    }
    auto GetCommandsBeginIndex(int /*which*/) const
    {
        return std::vector<AnimationCommands>::size_type(0);
    }
    auto GetCommandsEnd(int which) const
    {
        return mCommands.at(which).end();
    }
    auto GetCommandsEndIndex(int which) const
    {
        return mCommands.at(which).size();
    }
    auto GetCommandsAt(int which, int index) const
    {
        return mCommands.at(which).at(index);
    }

    auto toOnlineViewerJSON(int whichMarcher, Coord startPosition) const -> std::vector<nlohmann::json>;
    auto toOnlineViewerJSON(std::vector<CalChart::Point> const& points) const -> std::vector<std::vector<nlohmann::json>>
    {
        auto results = std::vector<std::vector<nlohmann::json>>{};
        for (unsigned ptIndex = 0; ptIndex < points.size(); ptIndex++) {
            results.push_back(toOnlineViewerJSON(ptIndex, points[ptIndex].GetPos()));
        }
        return results;
    }

private:
    std::vector<Coord> mPoints; // should probably be const
    std::vector<AnimationCommands> mCommands;
    std::string name;
    unsigned numbeats;
};

}

namespace CalChart::Animate {

using beats_t = unsigned;

struct Info {
    CalChart::Coord::CollisionType mCollision = CalChart::Coord::CollisionType::none;
    MarcherInfo mMarcherInfo{};
};

// A sheet is a collection of all the Marcher's Commands.
// The Commands are the positions, directions, and style of each marcher at their beats.
// Because a Sheet sees all the points and where they are, the sheet can calculate all the
// collisions that exist.
class Sheet {
public:
    Sheet(std::string name, unsigned numBeats, std::vector<std::vector<Animate::Command>> const& commands);
    ~Sheet() = default;
    Sheet(Sheet const&) = default;
    auto operator=(Sheet const&) -> Sheet& = default;
    Sheet(Sheet&&) noexcept = default;
    auto operator=(Sheet&&) noexcept -> Sheet& = default;

    [[nodiscard]] auto GetName() const { return mName; }
    [[nodiscard]] auto GetNumBeats() const { return mNumBeats; }

    [[nodiscard]] auto MarcherInfoAtBeat(size_t whichMarcher, beats_t beat) const -> CalChart::Animate::MarcherInfo;

    [[nodiscard]] auto AllMarcherInfoAtBeat(beats_t beat) const
    {
        return std::views::iota(0UL, mCommands.size()) | std::views::transform([this, beat](auto whichMarcher) {
            return MarcherInfoAtBeat(whichMarcher, beat);
        });
    }

    [[nodiscard]] auto GetAllBeatsWithCollisions() const -> std::set<beats_t>;
    [[nodiscard]] auto GetAllMarchersWithCollisionAtBeat(beats_t beat) const -> std::set<size_t>;
    [[nodiscard]] auto CollisionAtBeat(size_t whichMarcher, beats_t beat) const -> Coord::CollisionType;

private:
    [[nodiscard]] auto FindAllCollisions() const -> std::map<std::tuple<size_t, beats_t>, Coord::CollisionType>;
    std::string mName;
    beats_t mNumBeats;
    std::vector<Animate::Commands> mCommands;
    std::map<std::tuple<size_t, beats_t>, Coord::CollisionType> mCollisions;
};

class Sheets {
public:
    explicit Sheets(std::vector<Sheet> const& sheets);
    [[nodiscard]] auto TotalBeats() const -> beats_t;
    [[nodiscard]] auto BeatToSheetOffsetAndBeat(beats_t beat) const -> std::tuple<size_t, beats_t>;
    [[nodiscard]] auto BeatForSheet(int whichSheet) const -> beats_t { return mSheets.at(whichSheet).GetNumBeats(); }
    [[nodiscard]] auto MarcherInfoAtBeat(beats_t beat, int whichMarcher) const -> MarcherInfo;
    [[nodiscard]] auto CollisionAtBeat(beats_t beat, int whichMarcher) const -> Coord::CollisionType;
    [[nodiscard]] auto BeatHasCollision(beats_t whichBeat) const -> bool;
    [[nodiscard]] auto GetSheetName(int whichSheet) const { return mSheets.at(whichSheet).GetName(); }

private:
    std::vector<Sheet> mSheets;
    std::vector<beats_t> mRunningBeatCount;
};

}
