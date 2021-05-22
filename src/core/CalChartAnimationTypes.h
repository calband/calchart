#pragma once
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

namespace CalChart {

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

enum class AnimateDir {
    N,
    NE,
    E,
    SE,
    S,
    SW,
    W,
    NW
};

enum class MarchingStyle {
    HighStep,
    Military,
    ShowHigh,
    GrapeVine,
    JerkyStep,
    Close
};

}
