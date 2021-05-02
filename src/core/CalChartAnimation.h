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

#include "animate_types.h"
#include "CalChartAnimationSheet.h"
#include "CalChartCoord.h"

#include <map>
#include <memory>
#include <vector>

namespace CalChart {

class AnimationCommand;
class AnimationSheet;
struct DrawCommand;
class Show;
class AnimationErrors;

class Animation {
public:
    Animation(const Show& show);
    ~Animation();

    void GotoSheet(unsigned i);
    // Returns true if changes made
    bool PrevSheet();
    bool NextSheet();

    void GotoBeat(unsigned i);
    bool PrevBeat();
    bool NextBeat();
    void GotoTotalBeat(int i);

    // For drawing:
    struct animate_info_t {
        int index;
        CalChart::Coord::CollisionType mCollision;
        AnimateDir mDirection;
        float mRealDirection;
        Coord mPosition;
    };
    animate_info_t GetAnimateInfo(int which) const;

    std::vector<animate_info_t> GetAllAnimateInfo() const;

    int GetNumberSheets() const;
    auto GetCurrentSheet() const { return mCurrentSheetNumber; }
    auto GetNumberBeats() const { return mSheets.at(mCurrentSheetNumber).GetNumBeats(); }
    auto GetCurrentBeat() const { return mCurrentBeatNumber; }
    int GetTotalNumberBeatsUpTo(int sheet) const;
    int GetTotalNumberBeats() const { return GetTotalNumberBeatsUpTo(mSheets.size()); }
    int GetTotalCurrentBeat() const;
    auto GetCurrentSheetName() const { return mSheets.at(mCurrentSheetNumber).GetName(); }
    std::vector<AnimationErrors> GetAnimationErrors() const;
    std::map<std::tuple<int, int, int>, Coord::CollisionType> GetCollisions() const { return mCollisions; }
    bool CurrentBeatHasCollision() const;

    // collection of position of each point, for debugging purposes
    std::pair<std::string, std::vector<std::string>> GetCurrentInfo() const;

    std::vector<DrawCommand> GenPathToDraw(unsigned whichSheet, unsigned point, const Coord& offset) const;
    Coord EndPosition(unsigned whichSheet, unsigned point, const Coord& offset) const;

    std::vector<AnimationSheet>::const_iterator sheetsBegin() const;
    std::vector<AnimationSheet>::const_iterator sheetsEnd() const;

private:
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
    unsigned mCurrentBeatNumber{};
    std::vector<int> mAnimSheetIndices;
    std::vector<AnimationErrors> mAnimationErrors;
};
}
