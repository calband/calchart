#pragma once
/*
 * CalChartAnimationErrors.h
 * Errors that may occur when compiling a show
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

#include "CalChartConstants.h"
#include "CalChartContinuityToken.h"
#include "CalChartTypes.h"
#include "CalChartUtils.h"
#include <cstdint>
#include <map>
#include <optional>
#include <ostream>

/**
 * Animation Errors
 * An object that holds the errors when compiling a show.
 */

namespace CalChart {

enum class AnimateError : uint8_t {
    OUTOFTIME,
    EXTRATIME,
    WRONGPLACE,
    INVALID_CM,
    INVALID_FNTN,
    DIVISION_ZERO,
    UNDEFINED,
    SYNTAX,
    NONINT,
    NEGINT,
};

static inline auto AnimateErrorToString(AnimateError error)
{
    switch (error) {
    case AnimateError::OUTOFTIME:
        return "Ran out of time";
    case AnimateError::EXTRATIME:
        return "Not enough to do";
    case AnimateError::WRONGPLACE:
        return "Didn't make it to position";
    case AnimateError::INVALID_CM:
        return "Invalid countermarch";
    case AnimateError::INVALID_FNTN:
        return "Invalid fountain";
    case AnimateError::DIVISION_ZERO:
        return "Division by zero";
    case AnimateError::UNDEFINED:
        return "Undefined value";
    case AnimateError::SYNTAX:
        return "Syntax error";
    case AnimateError::NONINT:
        return "Non-integer value";
    case AnimateError::NEGINT:
        return "Negative value";
    }
    return "Generic error";
}

static inline auto operator<<(std::ostream& os, AnimateError e) -> std::ostream&
{
    return os << static_cast<int>(e);
}

using AnimationErrors = std::map<std::pair<AnimateError, SYMBOL_TYPE>, SelectionList>;
inline auto AnyErrors(AnimationErrors const& errors) { return !errors.empty(); }
inline auto RegisterAnimationError(AnimationErrors& errors, AnimateError err, int curr_pt, SYMBOL_TYPE contsymbol)
{
    errors[{ err, contsymbol }].insert(curr_pt);
}

}
