/*
 * cc_coord.h
 * Definitions for the coordinate classes
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

#ifndef _CC_COORD_H_
#define _CC_COORD_H_

#include "cc_types.h"

#include <stdint.h>

#define COORD_SHIFT 4
#define COORD_DECIMAL (1<<COORD_SHIFT)
#define INT2COORD(a) ((a) * COORD_DECIMAL)
#define COORD2INT(a) ((a) / COORD_DECIMAL)
#define FLOAT2NUM(a) CLIPFLOAT((a) * (1 << COORD_SHIFT))
#define FLOAT2COORD(a) (Coord)FLOAT2NUM((a))
#define COORD2FLOAT(a) ((a) / ((float)(1 << COORD_SHIFT)))
#define CLIPFLOAT(a) (((a) < 0) ? ((a) - 0.5) : ((a) + 0.5))

class CC_coord
{
public:
	CC_coord(Coord xval = 0, Coord yval = 0) : x(xval), y(yval) {}

	float Magnitude() const;
	float DM_Magnitude() const;				  // check for diagonal military also
	float Direction() const;
	float Direction(const CC_coord& c) const;

	bool Collides(const CC_coord& c) const;

	inline CC_coord& operator += (const CC_coord& c)
	{
		x += c.x; y += c.y;
		return *this;
	}
	inline CC_coord& operator -= (const CC_coord& c)
	{
		x -= c.x; y -= c.y;
		return *this;
	}
	inline CC_coord& operator *= (short s)
	{
		x *= s; y *= s;
		return *this;
	}
	inline CC_coord& operator /= (short s)
	{
		x /= s; y /= s;
		return *this;
	}

	Coord x, y;
};
inline CC_coord operator + (const CC_coord& a, const CC_coord& b)
{
	return CC_coord(a.x + b.x, a.y + b.y);
}


inline CC_coord operator - (const CC_coord& a, const CC_coord& b)
{
	return CC_coord(a.x - b.x, a.y - b.y);
}


inline CC_coord operator * (const CC_coord& a, short s)
{
	return CC_coord(a.x * s, a.y * s);
}


inline CC_coord operator / (const CC_coord& a, short s)
{
	return CC_coord(a.x / s, a.y / s);
}


inline CC_coord operator - (const CC_coord& c)
{
	return CC_coord(-c.x, -c.y);
}


inline int operator == (const CC_coord& a, const CC_coord& b)
{
	return ((a.x == b.x) && (a.y == b.y));
}


inline int operator != (const CC_coord& a, const CC_coord& b)
{
	return ((a.x != b.x) || (a.y != b.y));
}


void CC_coord_UnitTests();

#endif
