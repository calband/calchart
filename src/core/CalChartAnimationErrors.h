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

#include "CalChartAnimationTypes.h"
#include "CalChartContinuityToken.h"
#include "CalChartTypes.h"
#include <array>
#include <map>
#include <ostream>

/**
 * Animation Errors
 * An object that holds the errors when compiling a show.
 */

namespace CalChart {

enum class AnimateError {
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

static inline std::ostream& operator<<(std::ostream& os, AnimateError e)
{
    return os << static_cast<int>(e);
}

struct ErrorMarker {
    SelectionList pntgroup; // which points have this error
    SYMBOL_TYPE contsymbol = SYMBOL_PLAIN; // which continuity
    int line = -1, col = -1; // where
    bool operator==(ErrorMarker const& rhs) const
    {
        return pntgroup == rhs.pntgroup && contsymbol == rhs.contsymbol && line == rhs.line && col == rhs.col;
    }
};

class AnimationErrors {
public:
    void RegisterError(AnimateError err, Cont::Token const* token, unsigned curr_pt, SYMBOL_TYPE contsymbol)
    {
        mErrorMarkers[err].contsymbol = contsymbol;
        if (token != NULL) {
            mErrorMarkers[err].line = token->line;
            mErrorMarkers[err].col = token->col;
        }
        mErrorMarkers[err].pntgroup.insert(curr_pt);
    }
    void RegisterError(AnimateError err, int line, int col, unsigned curr_pt, SYMBOL_TYPE contsymbol)
    {
        mErrorMarkers[err].contsymbol = contsymbol;
        mErrorMarkers[err].line = line;
        mErrorMarkers[err].col = col;
        mErrorMarkers[err].pntgroup.insert(curr_pt);
    }
    auto AnyErrors() const { return !mErrorMarkers.empty(); }
    auto GetErrors() const { return mErrorMarkers; }

    bool operator==(AnimationErrors const& rhs) const
    {
        return mErrorMarkers == rhs.mErrorMarkers;
    }

private:
    std::map<AnimateError, ErrorMarker> mErrorMarkers;
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
}
