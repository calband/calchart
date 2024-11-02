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
#include <ostream>

/**
 * Animation Errors
 * An object that holds the errors when compiling a show.
 */

namespace CalChart::Animate {

enum class Error : uint8_t {
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

static inline auto ErrorToString(Error error)
{
    switch (error) {
    case Error::OUTOFTIME:
        return "Ran out of time";
    case Error::EXTRATIME:
        return "Not enough to do";
    case Error::WRONGPLACE:
        return "Didn't make it to position";
    case Error::INVALID_CM:
        return "Invalid countermarch";
    case Error::INVALID_FNTN:
        return "Invalid fountain";
    case Error::DIVISION_ZERO:
        return "Division by zero";
    case Error::UNDEFINED:
        return "Undefined value";
    case Error::SYNTAX:
        return "Syntax error";
    case Error::NONINT:
        return "Non-integer value";
    case Error::NEGINT:
        return "Negative value";
    }
    return "Generic error";
}

static inline auto operator<<(std::ostream& os, Error e) -> std::ostream&
{
    return os << static_cast<int>(e);
}

using Errors = std::map<Error, SelectionList>;
inline auto AnyErrors(Errors const& errors) { return !errors.empty(); }
inline auto RegisterError(Errors& errors, Error err, int curr_pt)
{
    errors[err].insert(curr_pt);
}

}
