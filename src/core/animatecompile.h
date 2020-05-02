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

#include "animate.h"
#include "animate_types.h"
#include "cc_show.h"

#include <array>
#include <list>
#include <memory>
#include <vector>

namespace CalChart {

class ContProcedure;
class ContToken;
class AnimateCommand;
using AnimationVariables = std::array<std::map<unsigned, float>, NUMCONTVARS>;
using AnimateCommands = std::vector<std::shared_ptr<AnimateCommand>>;

class AnimationErrors {
public:
    auto AnyErrors() const { return !mErrorMarkers.empty(); }
    auto GetErrors() const { return mErrorMarkers; }
    void RegisterError(AnimateError err, const ContToken* token, unsigned curr_pt,
        SYMBOL_TYPE contsymbol);
    void RegisterError(AnimateError err, int line, int col, unsigned curr_pt,
        SYMBOL_TYPE contsymbol);

    bool operator==(AnimationErrors const& rhs) const
    {
        return mErrorMarkers == rhs.mErrorMarkers;
    }

private:
    std::map<AnimateError, ErrorMarker> mErrorMarkers;
};

struct AnimateState {
    Coord pt;
    unsigned beats_rem;
    AnimationVariables& mVars;
    AnimationErrors& error_markers;
    AnimateCommands cmds;
};

class AnimateCompile {
public:
    // Compile a point
    static AnimateCommands
    Compile(const Show& show, AnimationVariables& variablesStates,
        AnimationErrors& errors, Show::const_Sheet_iterator_t c_sheet,
        unsigned pt_num, SYMBOL_TYPE cont_symbol,
        std::vector<std::unique_ptr<ContProcedure>> const& proc);

private:
    AnimateCompile(const Show& show, SYMBOL_TYPE cont_symbol, unsigned pt_num, Show::const_Sheet_iterator_t c_sheet, AnimateState& state);

public:
    bool Append(std::shared_ptr<AnimateCommand> cmd, const ContToken* token);
    void RegisterError(AnimateError err, const ContToken* token) const;

    float GetVarValue(int varnum, const ContToken* token) const;
    void SetVarValue(int varnum, float value);

    // helper functions to get information for building a command
    auto GetPointPosition() const { return mState.pt; }
    auto GetCurrentPoint() const { return curr_pt; }
    auto GetBeatsRemaining() const { return mState.beats_rem; }
    Coord GetStartingPosition() const;
    Coord GetEndingPosition(const ContToken* token) const;
    Coord GetReferencePointPosition(unsigned refnum) const;

private:
    const Show& mShow;
    const SYMBOL_TYPE contsymbol;
    const unsigned curr_pt;
    const Show::const_Sheet_iterator_t curr_sheet;

    AnimateState& mState;
};
}
