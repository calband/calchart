/*
 * animate_types.h
 * collection of types and enums for animation
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "cc_coord.h"
#include <string>

// Number of variables in continuity language (A B C D X Y Z DOF DOH)
enum {
    CONTVAR_A,
    CONTVAR_B,
    CONTVAR_C,
    CONTVAR_D,
    CONTVAR_X,
    CONTVAR_Y,
    CONTVAR_Z,
    CONTVAR_DOF,
    CONTVAR_DOH,
    NUMCONTVARS
};

enum AnimateDir {
    ANIMDIR_N,
    ANIMDIR_NE,
    ANIMDIR_E,
    ANIMDIR_SE,
    ANIMDIR_S,
    ANIMDIR_SW,
    ANIMDIR_W,
    ANIMDIR_NW
};

enum AnimateError {
    ANIMERR_OUTOFTIME,
    ANIMERR_EXTRATIME,
    ANIMERR_WRONGPLACE,
    ANIMERR_INVALID_CM,
    ANIMERR_INVALID_FNTN,
    ANIMERR_DIVISION_ZERO,
    ANIMERR_UNDEFINED,
    ANIMERR_SYNTAX,
    ANIMERR_NONINT,
    ANIMERR_NEGINT,
    NUM_ANIMERR
};

static const std::string s_animate_err_msgs[NUM_ANIMERR] = {
    "Ran out of time",
    "Not enough to do",
    "Didn't make it to position",
    "Invalid countermarch",
    "Invalid fountain",
    "Division by zero",
    "Undefined value",
    "Syntax error",
    "Non-integer value",
    "Negative value",
};

enum MarchingStyle {
    STYLE_HighStep,
    STYLE_Military,
    STYLE_ShowHigh,
    STYLE_GrapeVine,
    STYLE_JerkyStep,
    STYLE_Close
};

enum CollisionWarning {
    COLLISION_RESPONSE_NONE,
    COLLISION_RESPONSE_SHOW,
    COLLISION_RESPONSE_BEEP
};

using AnimatePoint = CC_coord;

struct ErrorMarker {
    SelectionList pntgroup; // which points have this error
    SYMBOL_TYPE contsymbol = SYMBOL_PLAIN; // which continuity
    int line = -1, col = -1; // where
};
