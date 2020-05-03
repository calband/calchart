#pragma once
/*
 * animate.h
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
#include "cc_coord.h"

#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace CalChart {

AnimateDir AnimGetDirFromAngle(float ang);

class AnimateCommand;
class AnimateSheet;
struct DrawCommand;
class Show;
class AnimationErrors;
class ContProcedure;

typedef std::function<void(const std::string& notice)> NotifyStatus;
typedef std::function<bool(
    const std::map<AnimateError, ErrorMarker>& error_markers, size_t sheetnum,
    const std::string& message)>
    NotifyErrorList;

using AnimateCommands = std::vector<std::shared_ptr<AnimateCommand>>;

// AnimateSheet is a snapshot of CC_sheet
class AnimateSheet {
public:
    AnimateSheet(const std::vector<Coord>& thePoints,
        const std::vector<AnimateCommands>& theCommands,
        const std::string& s, unsigned beats)
        : mPoints(thePoints)
        , mCommands(theCommands)
        , name(s)
        , numbeats(beats)
    {
    }

    // make things copiable
    AnimateSheet(AnimateSheet const&);
    AnimateSheet& operator=(AnimateSheet);
    AnimateSheet(AnimateSheet&&) noexcept;
    AnimateSheet& operator=(AnimateSheet&&) noexcept;
    void swap(AnimateSheet&) noexcept;

    auto GetName() const { return name; }
    auto GetNumBeats() const { return numbeats; }
    auto GetPoints() const { return mPoints; }
    auto GetCommands(int which) const { return mCommands.at(which); }
    auto GetCommandsBegin(int which) const
    {
        return mCommands.at(which).begin();
    }
    auto GetCommandsBeginIndex(int which) const
    {
        return 0;
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

private:
    std::vector<Coord> mPoints; // should probably be const
    std::vector<AnimateCommands> mCommands;
    std::string name;
    unsigned numbeats;
};

class Animation {
public:
    Animation(const Show& show);
    ~Animation();

    // Returns true if changes made
    void GotoSheet(unsigned i);
    void GotoAnimationSheet(unsigned i);
    bool PrevSheet();
    bool NextSheet();

    void GotoBeat(unsigned i);
    bool PrevBeat();
    bool NextBeat();
    void GotoTotalBeat(int i);

    typedef void (*CollisionAction_t)();
    // set collision action returns the previous collision action
    CollisionAction_t SetCollisionAction(CollisionAction_t col)
    {
        CollisionAction_t oldaction = mCollisionAction;
        mCollisionAction = col;
        return oldaction;
    }

    // For drawing:
    struct animate_info_t {
        int mCollision;
        AnimateDir mDirection;
        float mRealDirection;
        Coord mPosition;
        animate_info_t(int col, AnimateDir dir, float rdir, Coord pos)
            : mCollision(col)
            , mDirection(dir)
            , mRealDirection(rdir)
            , mPosition(pos)
        {
        }
    };
    animate_info_t GetAnimateInfo(int which) const;

    int GetNumberSheets() const;
    auto GetCurrentSheet() const { return mCurrentSheetNumber; }
    auto GetNumberBeats() const { return mSheets.at(mCurrentSheetNumber).GetNumBeats(); }
    auto GetCurrentBeat() const { return mCurrentBeatNumber; }
    int GetTotalNumberBeatsUpTo(int sheet) const;
    int GetTotalNumberBeats() const { return GetTotalNumberBeatsUpTo(mSheets.size()); }
    int GetTotalCurrentBeat() const;
    auto GetCurrentSheetName() const { return mSheets.at(mCurrentSheetNumber).GetName(); }
    std::vector<AnimationErrors> GetAnimationErrors() const;

    // collection of position of each point, for debugging purposes
    std::pair<std::string, std::vector<std::string>> GetCurrentInfo() const;

    std::vector<DrawCommand> GenPathToDraw(unsigned whichSheet, unsigned point, const Coord& offset) const;
    Coord EndPosition(unsigned whichSheet, unsigned point, const Coord& offset) const;

    std::vector<AnimateSheet>::const_iterator sheetsBegin() const;
    std::vector<AnimateSheet>::const_iterator sheetsEnd() const;

private:
    void BeginCmd(unsigned i);
    void EndCmd(unsigned i);

    void RefreshSheet();
    void CheckCollisions();

    std::vector<std::shared_ptr<AnimateCommand>> GetCommands(unsigned whichSheet, unsigned whichPoint) const;
    AnimateCommand& GetCommand(unsigned whichSheet, unsigned whichPoint) const;

    // There are two types of data, the ones that are set when we are created, and the ones that modify over time.
    std::vector<AnimateSheet> mSheets;
    std::vector<Coord> mPoints; // current position of these points
    std::vector<size_t> mCurrentCmdIndex; // pointer to the current command in the sheet
    std::map<unsigned, Coord::CollisionType> mCollisions;
    unsigned mCurrentSheetNumber{};
    unsigned mCurrentBeatNumber{};
    CollisionAction_t mCollisionAction;
    std::vector<int> mAnimSheetIndices;
    std::vector<AnimationErrors> mAnimationErrors;

};
}
