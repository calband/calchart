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

using AnimationVariables = std::array<std::map<unsigned, float>, Cont::kNumVariables>;
using AnimationCompileResult = std::vector<Animate::Command>;

// Compile a point into the Animation Commands
// Variables and Errors are passed as references as they maintain state over all the compiles,
// and unfortunately, it is faster to pass them this way.
// endPosition and nextPosition are a little odd.
// end is the position to go that's the next valid animation -- the shee has at least 1 beat
// next position is the position of the marcher on the next sheet, regardless of number of beats.
// is optional because if there is no next sheet (this is the last sheet), then it's null.
auto Compile(
    AnimationVariables& variablesStates,
    AnimationErrors& errors,
    unsigned whichMarcher,
    SYMBOL_TYPE cont_symbol,
    Point point,
    beats_t beats,
    bool isLastAnimationSheet,
    std::optional<Coord> endPosition,
    std::optional<Coord> nextPosition,
    std::vector<std::unique_ptr<Cont::Procedure>> const& proc) -> AnimationCompileResult;

struct AnimationCompile {
    virtual ~AnimationCompile() = default;
    virtual auto Append(Animate::Command cmd, Cont::Token const* token) -> bool = 0;
    virtual void RegisterError(AnimateError err, Cont::Token const* token) const = 0;

    [[nodiscard]] virtual auto GetVarValue(Cont::Variable varnum, Cont::Token const* token) const -> float = 0;
    virtual void SetVarValue(Cont::Variable varnum, float value) = 0;

    // helper functions to get information for building a command
    [[nodiscard]] virtual auto GetPointPosition() const -> Coord = 0;
    [[nodiscard]] virtual auto GetStartingPosition() const -> Coord = 0;
    [[nodiscard]] virtual auto GetEndingPosition(Cont::Token const* token) const -> Coord = 0;
    [[nodiscard]] virtual auto GetReferencePointPosition(unsigned refnum) const -> Coord = 0;
    [[nodiscard]] virtual auto GetCurrentPoint() const -> unsigned = 0;
    [[nodiscard]] virtual auto GetBeatsRemaining() const -> unsigned = 0;
};
}
