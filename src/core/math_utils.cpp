/*
 * math_utils.cpp
 * math utility functions
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

#include "math_utils.h"

#include "cc_coord.h"

#include <ctype.h>
#include <list>
#include <tuple>

float BoundDirection(float f)
{
	while (f >= 360.0) f -= 360.0;
	while (f < 0.0) f += 360.0;
	return f;
}

float NormalizeAngle(float ang)
{
	return BoundDirection(ang);
}


float BoundDirectionSigned(float f)
{
	while (f >= 180.0) f -= 360.0;
	while (f < -180.0) f += 360.0;
	return f;
}


bool IsDiagonalDirection(float f)
{
	f = BoundDirection(f);
	return (IS_ZERO(f - 45.0) || IS_ZERO(f - 135.0) ||
		IS_ZERO(f - 225.0) || IS_ZERO(f - 315.0));
}


CC_coord CreateVector(float dir, float mag)
{
	dir = BoundDirection(dir);
	if (IsDiagonalDirection(dir))
	{
		CC_coord r{ Float2Coord(mag), Float2Coord(mag) };
		if ((dir > 50.0) && (dir < 310.0)) r.x = -r.x;
		if (dir < 180.0) r.y = -r.y;
		return r;
	}
	else
	{
		return CC_coord{ Float2Coord(mag * cos(Deg2Rad(dir))), Float2Coord(mag * -sin(Deg2Rad(dir))) };
	}
}


std::tuple<float, float> CreateUnitVector(float dir)
{
	dir = BoundDirection(dir);
	if (IsDiagonalDirection(dir))
	{
		std::tuple<float, float> result { 1.0, 1.0 };
		if ((dir > 50.0) && (dir < 310.0)) std::get<0>(result) = -std::get<0>(result);
		if (dir < 180.0) std::get<1>(result) = -std::get<0>(result);
		return result;
	}
	else
	{
		return std::tuple<float, float> { cos(Deg2Rad(dir)), -sin(Deg2Rad(dir)) };
	}
}


