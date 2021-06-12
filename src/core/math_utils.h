#pragma once
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

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdlib>
#include <tuple>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CalChart {
struct Coord;
}

static const double kEpsilon = 0.00001;
template <typename T>
bool IS_ZERO(const T& a)
{
    return std::abs(a) < kEpsilon;
}

template <typename T>
T Deg2Rad(const T& a) { return a * M_PI / 180.0; }

#define SQRT2 1.4142136

template <typename T>
constexpr T BoundDirection(T f)
{
    while (f >= 360.0)
        f -= 360.0;
    while (f < 0.0)
        f += 360.0;
    return f;
}

template <typename T>
constexpr T BoundDirectionRad(T f)
{
    while (f >= 2 * M_PI)
        f -= 2 * M_PI;
    while (f < 0.0)
        f += 2 * M_PI;
    return f;
}

template <typename T>
constexpr T NormalizeAngle(T ang) { return BoundDirection(ang); }

template <typename T>
constexpr T NormalizeAngleRad(T ang) { return BoundDirectionRad(ang); }

template <typename T>
T BoundDirectionSigned(T f)
{
    while (f >= 180.0)
        f -= 360.0;
    while (f < -180.0)
        f += 360.0;
    return f;
}

template <typename T>
auto IsDiagonalDirection(T f)
{
    f = BoundDirection(f);
    return (IS_ZERO(f - 45.0) || IS_ZERO(f - 135.0) || IS_ZERO(f - 225.0) || IS_ZERO(f - 315.0));
}

template <typename T>
std::tuple<T, T> CreateUnitVector(T dir)
{
    dir = BoundDirection(dir);
    if (IsDiagonalDirection(dir)) {
        std::tuple<T, T> result{ 1.0, 1.0 };
        if ((dir > 50.0) && (dir < 310.0))
            std::get<0>(result) = -std::get<0>(result);
        if (dir < 180.0)
            std::get<1>(result) = -std::get<0>(result);
        return result;
    } else {
        return std::tuple<T, T>{ cos(Deg2Rad(dir)), -sin(Deg2Rad(dir)) };
    }
}

CalChart::Coord CreateVector(float dir, float mag);
