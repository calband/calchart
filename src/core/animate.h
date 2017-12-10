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

#pragma once

#include "animate_types.h"
#include "cc_coord.h"

#include <memory>
#include <functional>
#include <map>
#include <set>
#include <vector>
#include <list>

AnimateDir AnimGetDirFromAngle(float ang);

class AnimateCommand;
class AnimateSheet;
struct CC_DrawCommand;
namespace CalChart {
class Show;
}
class AnimationErrors;
class ContProcedure;

typedef std::function<void(const std::string& notice)> NotifyStatus;
typedef std::function<bool(
    const std::map<AnimateError, ErrorMarker>& error_markers, size_t sheetnum,
    const std::string& message)> NotifyErrorList;

using AnimateCommands = std::vector<std::shared_ptr<AnimateCommand> >;

// AnimateSheet is a snapshot of CC_sheet
class AnimateSheet {
public:
    AnimateSheet(const std::vector<AnimatePoint>& thePoints,
        const std::vector<AnimateCommands>& theCommands,
        const std::string& s, unsigned beats)
        : mPoints(thePoints)
        , commands(theCommands)
        , name(s)
        , numbeats(beats)
    {
    }
    auto GetName() const { return name; }
    auto GetNumBeats() const { return numbeats; }
    auto GetPoints() const { return mPoints; }
    auto GetCommands(int which) const { return commands.at(which); }
    auto GetCommandsBegin(int which) const
    {
        return commands.at(which).begin();
    }
    auto GetCommandsEnd(int which) const
    {
        return commands.at(which).end();
    }

private:
    std::vector<AnimatePoint> mPoints; // should probably be const
    std::vector<AnimateCommands> commands;
    std::string name;
    unsigned numbeats;
};

class Animation {
public:
    Animation(const CalChart::Show& show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList);
    ~Animation();

    static std::list<std::unique_ptr<ContProcedure> > ParseContinuity(std::string const& continuity, AnimationErrors& errors, SYMBOL_TYPE current_symbol);

    // Returns true if changes made
    void GotoSheet(unsigned i);
    void GotoAnimationSheet(unsigned i);
    bool PrevSheet();
    bool NextSheet();

    void GotoBeat(unsigned i);
    bool PrevBeat();
    bool NextBeat();

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
        CC_coord mPosition;
        animate_info_t(int col, AnimateDir dir, float rdir, CC_coord pos)
            : mCollision(col)
            , mDirection(dir)
            , mRealDirection(rdir)
            , mPosition(pos)
        {
        }
    };
    animate_info_t GetAnimateInfo(int which) const;

    int GetNumberSheets() const;
    auto GetCurrentSheet() const { return curr_sheetnum; }
    auto GetNumberBeats() const { return sheets.at(curr_sheetnum).GetNumBeats(); }
    auto GetCurrentBeat() const { return curr_beat; }
    auto GetCurrentSheetName() const { return sheets.at(curr_sheetnum).GetName(); }

    // collection of position of each point, for debugging purposes
    std::pair<std::string, std::vector<std::string> > GetCurrentInfo() const;

    std::vector<CC_DrawCommand> GenPathToDraw(unsigned point, const CC_coord& offset) const;
    AnimatePoint EndPosition(unsigned point, const CC_coord& offset) const;

    auto sheetsBegin() const { return sheets.begin(); }
    auto sheetsEnd() const { return sheets.end(); }

private:
    std::vector<AnimatePoint> pts;
    std::vector<std::vector<std::shared_ptr<AnimateCommand> >::const_iterator> curr_cmds; // pointer to the current command
    std::map<unsigned, CollisionType> mCollisions;
    unsigned curr_sheetnum;
    unsigned curr_beat;
    std::vector<AnimateSheet> sheets;
    CollisionAction_t mCollisionAction;
    std::vector<int> mAnimSheetIndices;

    void BeginCmd(unsigned i);
    void EndCmd(unsigned i);

    void RefreshSheet();

    void CheckCollisions();

    std::vector<std::shared_ptr<AnimateCommand> > GetCommands(unsigned whichPoint) const;
};
