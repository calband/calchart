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

float BoundDirection(float f);
float NormalizeAngle(float ang);

float BoundDirectionSigned(float f);

bool IsDiagonalDirection(float f);

void CreateVector(CC_coord& c, float dir, float mag);

void CreateUnitVector(float& a, float& b, float dir);

#endif
