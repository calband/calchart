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

/**
 * RoundToCoord: Use when number is already in Coord format, just needs to be rounded
 */
template <typename T>
static inline Coord RoundToCoord(T inCoord) { return static_cast<Coord>((inCoord < 0) ? (inCoord - 0.5) : (inCoord + 0.5)); }

// Float2Coord, Coord2Float
//  Use when we want to convert to Coord system
template <typename T>
static inline Coord Float2Coord(T a) { return RoundToCoord(a * COORD_DECIMAL); }
template <typename T>
static inline float Coord2Float(T a) { return a / (float)COORD_DECIMAL; }

// Int2Coord, Coord2Int
//  Use when we want to convert to Coord system
template <typename T>
static inline Coord Int2Coord(T a) { return a * COORD_DECIMAL; }
template <typename T>
static inline int Coord2Int(T a) { return a / COORD_DECIMAL; }

/**
 * A coordinate in CalChart.
 * Essentially, this class is just a two-dimensional vector. It can be used
 * to represent a position or path along which to move.
 */
class CC_coord
{
public:

	/** 
	 * Constructs a coordinate (or vector) with the specified coordinates.
	 * @param xval The x component of the vector/point.
	 * @param yval The y component of the vector/point.
	 */
	CC_coord(Coord xval = 0, Coord yval = 0) : x(xval), y(yval) {}

	/** 
	 * Returns the magnitude of the vector represented by this object.
	 * The magnitude is defined to be the distance from the point (0, 0)
	 * to the point (x, y), where x is the x component of the object, and
	 * y is the y component.
	 * @return The magnitude of the vector represented by this object.
	 */
	float Magnitude() const;

	/**
	 * Returns the magnitude of the vector represented by this object under
     * certain circumstances.
	 * Returns the magnitude of the object, as defined by the Magnitude() method,
	 * IF the vector represented by the object does NOT make a 45 degree angle
	 * with the x or y axis (which occurs when the x component is either equal to
	 * the y component or its negative). If the vector represented by this object
	 * DOES make a 45 degree angle with either the x or y axis, then the returned
	 * magnitude of the vector is simply the absolute value of the x component
	 * (which is equal to the absolute value of the y component of the CC_coord
	 * under the conditions in which this occurs.
	 * @return If the vector represented by this object does not make a 45 degree
	 * diagonal, this method returns the magnitude of the vector represented by
	 * the object. Otherwise, this returns the absolute value of the x and
	 * y coordinates (the absolute value of both is the same in this case).
	 */
	float DM_Magnitude() const;	

	/**
	 * Returns the angle, in degrees, that the vector represented by this
	 * object makes with the positive x-axis.
	 * @return The angle, in degrees, that the vector represented by this
	 * object makes with the positive x-axis.
	 */
	float Direction() const;

	/**
	 * Returns the angle, relative to the positive x-axis, that points at another
	 * coordinate from this coordinate, where the angle is measured in degrees.
	 * @param c The coordinate to point toward from this coordinate.
	 * @return The direction from this coordinate to the provided coordinate.
	 */
	float Direction(const CC_coord& c) const;

	/**
	 * Returns true if this coordinate is "colliding" with another.
	 * @param c A coordinate to check for collisions with this object.
	 * @return Returns true if this object is within one step of the
	 * coordinate passed as a parameter (where one step is the standard
	 * unit of the x and y coordinates of a CC_coord); returns false
	 * otherwise.
	 */
	bool Collides(const CC_coord& c) const;

	/**
	 * Performs vector addition.
	 * @param c The vector to add to this one.
	 * @return A reference to this object, which has been modified.
	 */
	inline CC_coord& operator += (const CC_coord& c)
	{
		x += c.x; y += c.y;
		return *this;
	}

	/**
	 * Performs vector subtraction.
	 * @param c The vector to subtract from this one.
	 * @return A reference to this object, which has been modified.
	 */
	inline CC_coord& operator -= (const CC_coord& c)
	{
		x -= c.x; y -= c.y;
		return *this;
	}

	/**
	 * Performs vector-scalar multiplication.
	 * @param s The scalar to multiply this vector by.
	 * @return A reference to this object, which has been modified.
	 */
	inline CC_coord& operator *= (short s)
	{
		x *= s; y *= s;
		return *this;
	}

	/**
	 * Performs vector-scalar division.
	 * @param s The scalar to divide this vector by.
	 * @return A reference to this object, which has been modified.
	 */
	inline CC_coord& operator /= (short s)
	{
		x /= s; y /= s;
		return *this;
	}

	/** 
	 * The X component of the vector.
	 */
	Coord x;
		
	/**
	 * The Y component of the vector.
	 */
	Coord y;
};

//TODO
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
