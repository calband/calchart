/*
 * cc_coord.cpp
 * Member functions for coordinate classes
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

#include "cc_coord.h"
#include "platconf.h"

#include <math.h>

// Get magnitude of vector
float CC_coord::Magnitude() const
{
	float x_f, y_f;

	x_f = COORD2FLOAT(x);
	y_f = COORD2FLOAT(y);
	return sqrt(x_f*x_f + y_f*y_f);
}


// Get magnitude, but check for diagonal military
float CC_coord::DM_Magnitude() const
{
	if ((x == y) || (x == -y))
	{
		return COORD2FLOAT(ABS(x));
	}
	else
	{
		return Magnitude();
	}
}


// Get direction of this vector
float CC_coord::Direction() const
{
	float ang;

	if (*this == 0) return 0.0;

	ang = acos(COORD2FLOAT(x)/Magnitude());		  // normalize
	ang *= 180.0/PI;							  // convert to degrees
	if (y > 0) ang = (-ang);					  // check for > PI

	return ang;
}


// Get direction from this coord to another
float CC_coord::Direction(const CC_coord& c) const
{
	CC_coord vect = c - *this;

	return vect.Direction();
}


// Returns true if this coordinate is within 1 step of another
bool CC_coord::Collides(const CC_coord& c) const
{
	Coord dx, dy;

	dx = x - c.x;
	dy = y - c.y;
// Check for special cases to avoid multiplications
	if (ABS(dx) > INT2COORD(1)) return false;
	if (ABS(dy) > INT2COORD(1)) return false;
	return (((dx*dx)+(dy*dy)) <= (INT2COORD(1)*INT2COORD(1)));
}


// Set a coordinate from an old format disk coord
CC_coord& CC_coord::operator = (const cc_oldcoord& old)
{
	x = ((get_lil_word(&old.x)+2) << (COORD_SHIFT-3)) - INT2COORD(88);
	y = (((get_lil_word(&old.y)+2) << COORD_SHIFT) / 6) - INT2COORD(50);
	return *this;
}

