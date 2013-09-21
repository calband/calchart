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


void CreateVector(CC_coord& c, float dir, float mag)
{
	float f;

	dir = BoundDirection(dir);
	if (IsDiagonalDirection(dir))
	{
		c.x = c.y = Float2Coord(mag);
		if ((dir > 50.0) && (dir < 310.0)) c.x = -c.x;
		if (dir < 180.0) c.y = -c.y;
	}
	else
	{
		f = mag * cos(Deg2Rad(dir));
		c.x = Float2Coord(f);
		f = mag * -sin(Deg2Rad(dir));
		c.y = Float2Coord(f);
	}
}


void CreateUnitVector(float& a, float& b, float dir)
{
	dir = BoundDirection(dir);
	if (IsDiagonalDirection(dir))
	{
		a = b = 1.0;
		if ((dir > 50.0) && (dir < 310.0)) a = -a;
		if (dir < 180.0) b = -b;
	}
	else
	{
		a = cos(Deg2Rad(dir));
		b = -sin(Deg2Rad(dir));
	}
}


