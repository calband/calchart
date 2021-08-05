#pragma once
/*
 * CalChartUtils.h
 * General Utilities
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

#include "CalChartTypes.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace CalChart {
struct Coord;
}

constexpr auto kEpsilon = 0.00001;
constexpr auto SQRT2 = 1.4142136;

template <typename T>
constexpr auto IS_ZERO(T a) { return std::abs(a) < kEpsilon; }
template <typename T>
constexpr auto Deg2Rad(T a) { return a * M_PI / 180.0; }
template <typename T>
constexpr auto Rad2Deg(T a) { return a * 180.0 / M_PI; }

template <typename T>
constexpr auto BoundDirection(T f)
{
    while (f >= 360.0)
        f -= 360.0;
    while (f < 0.0)
        f += 360.0;
    return f;
}

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
constexpr auto BoundDirectionRad(T f)
{
    while (f >= 2 * M_PI)
        f -= 2 * M_PI;
    while (f < 0.0)
        f += 2 * M_PI;
    return f;
}

template <typename T>
constexpr auto NormalizeAngle(T ang) { return BoundDirection(ang); }
template <typename T>
constexpr auto NormalizeAngleRad(T ang) { return BoundDirectionRad(ang); }

// we assume the layout of quadrants are
template <typename T>
auto AngleToQuadrant(T ang)
{
    // rotate angle by 22.5:
    auto ang1 = ang + 22.5;
    ang1 = NormalizeAngle(ang1);

    return (8 + static_cast<int>(-ang1 / 45.0)) % 8;
}

template <typename T>
auto AngleToDirection(T ang)
{
    return static_cast<CalChart::Direction>(AngleToQuadrant(ang));
}

template <typename T>
auto IsDiagonalDirection(T f)
{
    f = BoundDirection(f);
    return (IS_ZERO(f - 45.0) || IS_ZERO(f - 135.0) || IS_ZERO(f - 225.0) || IS_ZERO(f - 315.0));
}

template <typename T>
auto CreateUnitVector(T dir)
{
    dir = -BoundDirection(dir);
    auto result = std::tuple<decltype(dir), decltype(dir)>{ cos(Deg2Rad(dir)), sin(Deg2Rad(dir)) };
    // we 'normalize' the diagonal direction to 1,1 because that be the CalChart way.
    if (IsDiagonalDirection(dir)) {
        std::get<0>(result) /= SQRT2 / 2;
        std::get<1>(result) /= SQRT2 / 2;
    }
    if (dir > 180) {
        std::get<1>(result) = -std::get<1>(result);
    }
    return result;
}
