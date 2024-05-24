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

#include "CalChartAngles.h"
#include "CalChartAnimationErrors.h"
#include "CalChartAnimationSheet.h"
#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"

#include <map>
#include <memory>
#include <vector>

namespace CalChart {

class AnimationCommand;
class AnimationSheet;
class Show;

namespace Animate {
    // For drawing:
    struct Info {
        int index{};
        CalChart::Coord::CollisionType mCollision = CalChart::Coord::CollisionType::none;
        CalChart::Radian mFacingDirection{};
        Coord mPosition{};
        MarchingStyle mStepStyle = MarchingStyle::HighStep;
    };

    inline auto FacingBack(Info const& info)
    {
        auto direction = CalChart::AngleToDirection(info.mFacingDirection);
        return direction == CalChart::Direction::SouthWest
            || direction == CalChart::Direction::West
            || direction == CalChart::Direction::NorthWest;
    }

    inline auto FacingFront(Info const& info)
    {
        auto direction = CalChart::AngleToDirection(info.mFacingDirection);
        return direction == CalChart::Direction::SouthEast
            || direction == CalChart::Direction::East
            || direction == CalChart::Direction::NorthEast;
    }

    inline auto FacingSide(Info const& info)
    {
        return !FacingBack(info) && !FacingFront(info);
    }

    inline auto CollisionWarning(Info const& info)
    {
        return info.mCollision == CalChart::Coord::CollisionType::warning;
    }

    inline auto CollisionIntersect(Info const& info)
    {
        return info.mCollision == CalChart::Coord::CollisionType::intersect;
    }

}

using beats_t = unsigned;

class Animation {
public:
    explicit Animation(const Show& show);
    ~Animation();

    void GotoSheet(unsigned i);
    void GotoTotalBeat(beats_t i);

    auto GetAnimateInfo(int which) const -> Animate::Info;

    auto GetAllAnimateInfo() const -> std::vector<Animate::Info>;

    int GetNumberSheets() const;
    auto GetCurrentSheet() const { return mCurrentSheetNumber; }
    auto GetNumberBeats() const { return mSheets.at(mCurrentSheetNumber).GetNumBeats(); }
    auto GetCurrentBeat() const { return mCurrentBeatNumber; }
    auto GetTotalNumberBeatsUpTo(int sheet) const -> beats_t;
    auto GetTotalNumberBeats() const { return GetTotalNumberBeatsUpTo(static_cast<int>(mSheets.size())); }
    auto GetTotalCurrentBeat() const -> beats_t;
    auto GetCurrentSheetName() const { return mSheets.at(mCurrentSheetNumber).GetName(); }
    std::vector<AnimationErrors> GetAnimationErrors() const;
    std::map<std::tuple<int, int, int>, Coord::CollisionType> GetCollisions() const { return mCollisions; }
    bool CurrentBeatHasCollision() const;

    // collection of position of each point, for debugging purposes
    std::pair<std::string, std::vector<std::string>> GetCurrentInfo() const;

    std::vector<Draw::DrawCommand> GenPathToDraw(unsigned whichSheet, unsigned point, Coord::units endRadius) const;

    /*!
     * @brief Generates JSON that could represent of all the marchers in an Online Viewer '.viewer' file.
     * @param pointsOverSheets All of the points in all of the sheets.
     * @return A JSON which could represent all the animations in a '.viewer' file.
     */
    [[nodiscard]] auto toOnlineViewerJSON(std::vector<std::vector<CalChart::Point>> const& pointsOverSheets) const -> std::vector<std::vector<std::vector<nlohmann::json>>>;

private:
    // Returns true if changes made
    bool PrevSheet();
    bool NextSheet();

    void GotoBeat(unsigned i);
    bool PrevBeat();
    bool NextBeat();

    void BeginCmd(unsigned i);
    void EndCmd(unsigned i);

    void RefreshSheet();
    void FindAllCollisions();

    std::vector<std::shared_ptr<AnimationCommand>> GetCommands(unsigned whichSheet, unsigned whichPoint) const;
    AnimationCommand& GetCommand(unsigned whichSheet, unsigned whichPoint) const;

    // There are two types of data, the ones that are set when we are created, and the ones that modify over time.
    std::vector<AnimationSheet> mSheets;
    std::vector<Coord> mPoints; // current position of these points
    std::vector<size_t> mCurrentCmdIndex; // pointer to the current command in the sheet

    // mapping of Which, Sheet, Beat to a collision
    std::map<std::tuple<int, int, int>, Coord::CollisionType> mCollisions;
    unsigned mCurrentSheetNumber{};
    beats_t mCurrentBeatNumber{};
    std::vector<int> mAnimSheetIndices;
    std::vector<AnimationErrors> mAnimationErrors;
};

}
