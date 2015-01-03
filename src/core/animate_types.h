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

#ifndef _ANIMATE_TYPES_H_
#define _ANIMATE_TYPES_H_

#include "cc_coord.h"
#include <string>

// Number of variables in continuity language (A B C D X Y Z DOF DOH)
enum class ContVar
{
	A, B, C, D, X, Y, Z, DOF, DOH, NumContVars
};

enum class AnimateDir
{
	N, NE, E, SE, S, SW, W, NW
};

enum class AnimateError
{
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
	NUM_ANIMERR
};

enum class MarchingStyle
{
	HighStep, Military, ShowHigh, GrapeVine, JerkyStep, Close
};

enum class CollisionWarning
{
	NONE, SHOW, BEEP
};

std::string animate_err_msgs(AnimateError which);

typedef CC_coord AnimatePoint;

#endif // _ANIMATE_TYPES_H_
