#pragma once
/*
 * CalChartAnimationCompile.h
 * Classes for compiling the animation command
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

/**
 * AnimationCompile
 *
 * Compiling the animation is the way we get from a show layout with a set of continuities to the actual
 * movements of marchers on the field.  Effectively what we are doing is mapping the ContinuityProceedures
 * on to the Show in order to get the vector of AnimationCommands.
 *
 * ContinuityValues and Points are abstract concepts.  They represent the concept of a point or value.  In order
 * to go from the concept to a specific value, they need to be given a state that represents the show.  The
 * AnimationCompile object represents the portion of the show that is being converted from an abstract concept
 * (the StartPoint for example) to a specific value (the position of a specific marcher on the field.
 *
 */

#include "CalChartAnimation.h"
#include "CalChartAnimationErrors.h"
#include "CalChartAnimationTypes.h"
#include "CalChartShow.h"

#include <array>
#include <list>
#include <memory>
#include <vector>

namespace CalChart {

namespace Cont {
    class Procedure;
    class Token;
}

class AnimationCommand;
using AnimationVariables = std::array<std::map<unsigned, float>, Cont::kNumVariables>;
using AnimationCommands = std::vector<std::shared_ptr<AnimationCommand>>;

// Compile a point into the Animation Commands
// Variables and Errors are passed as references as they maintain state over all the compiles,
// and unfortunately, it is faster to pass them this way.
AnimationCommands
Compile(
    AnimationVariables& variablesStates,
    AnimationErrors& errors,
    Show::const_Sheet_iterator_t c_sheet,
    Show::const_Sheet_iterator_t endSheet,
    unsigned pt_num,
    SYMBOL_TYPE cont_symbol,
    std::vector<std::unique_ptr<Cont::Procedure>> const& proc);

struct AnimationCompile {
    virtual bool Append(std::unique_ptr<AnimationCommand> cmd, Cont::Token const* token) = 0;
    virtual void RegisterError(AnimateError err, Cont::Token const* token) const = 0;

    virtual float GetVarValue(Cont::Variable varnum, Cont::Token const* token) const = 0;
    virtual void SetVarValue(Cont::Variable varnum, float value) = 0;

    // helper functions to get information for building a command
    virtual Coord GetPointPosition() const = 0;
    virtual Coord GetStartingPosition() const = 0;
    virtual Coord GetEndingPosition(Cont::Token const* token) const = 0;
    virtual Coord GetReferencePointPosition(unsigned refnum) const = 0;
    virtual unsigned GetCurrentPoint() const = 0;
    virtual unsigned GetBeatsRemaining() const = 0;
};
}
