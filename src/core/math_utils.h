/*
 * math_utils.h
 * Math utility functions
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

#ifndef __MATH_UTILS_H__
#define __MATH_UTILS_H__

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>

class CC_coord;

static const double kEpsilon = 0.00001;
template <typename T>
bool IS_ZERO(const T& a)
{
	return std::abs(a) < kEpsilon;
}

template <typename T>
T Deg2Rad(const T& a)
{
	return a * M_PI/180.0;
}

#define SQRT2 1.4142136

/**
 * Confines an angle to the range of [0,360) degrees.
 * @param f The angle to bound, in degrees.
 * @return The angle f, bound on the range [0, 360).
 */
float BoundDirection(float f);

/**
 * TODO
 */
float NormalizeAngle(float ang);

/**
 * Confines an angle to the range of [-180, 180) degrees.
 * @param f The angle to bound, in degrees.
 * @return The angle f, bound on the range of [-180, 180).
 */
float BoundDirectionSigned(float f);

/**
 * Returns true if the angle f represents a diagonal direction
 * (45, 135, 225, or 315 degrees, when bounded on [0, 360)).
 * @param f The angle to check, in degrees.
 * @return True if the angle represents a diagonal direction;
 * falso otherwise.
 */
bool IsDiagonalDirection(float f);

/**
 * Modifies the input vector so that it has a particular
 * magnitude and direction (except in the case that the
 * direction is diagonal). When diagonal vectors are formed,
 * the absolute values of both final vector components will
 * have the magnitude passed as a parameter.
 * @param c The vector to modify.
 * @param dir The angle that the finished vector will
 * make with the positive x-axis, in degrees.
 * @param mag The magnitude of the final vector, except
 * in the case where the direction is diagonal. In that
 * case, both components of the final vector will have
 * absolute values equal to this parameter.
 */ 
void CreateVector(CC_coord& c, float dir, float mag);

/**
 * Modifies the input parameters so that they represent
 * a unit vector pointing in a particular direction
 * (with a special case when the direction is a diagonal
 * direction). When the direction is a diagonal direction,
 * the components of the output vector both will have
 * absolute values equal to one.
 * @param a This will be modified by the function to represent
 * the x component of the output vector.
 * @param b This will be modified by the function to represent
 * the y component of the output vector.
 * @param dir This is the angle that the final output vector
 * will make with the positive x-axis, in degrees.
 */
void CreateUnitVector(float& a, float& b, float dir);

#endif
