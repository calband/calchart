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

#include "animate_types.h"
#include "cont.h"
#include <map>
#include <array>

/**
 * Animation Errors
 * An object that holds the errors when compiling a show.
 */

namespace CalChart {

class AnimationErrors {
public:
    void RegisterError(AnimateError err, ContToken const* token, unsigned curr_pt, SYMBOL_TYPE contsymbol) {
        mErrorMarkers[err].contsymbol = contsymbol;
        if (token != NULL) {
            mErrorMarkers[err].line = token->line;
            mErrorMarkers[err].col = token->col;
        }
        mErrorMarkers[err].pntgroup.insert(curr_pt);
    }
    void RegisterError(AnimateError err, int line, int col, unsigned curr_pt, SYMBOL_TYPE contsymbol) {
        mErrorMarkers[err].contsymbol = contsymbol;
        mErrorMarkers[err].line = line;
        mErrorMarkers[err].col = col;
        mErrorMarkers[err].pntgroup.insert(curr_pt);
    }
    auto AnyErrors() const { return !mErrorMarkers.empty(); }
    auto GetErrors() const { return mErrorMarkers; }

    bool operator==(AnimationErrors const& rhs) const {
        return mErrorMarkers == rhs.mErrorMarkers;
    }

private:
    std::map<AnimateError, ErrorMarker> mErrorMarkers;
};

}
