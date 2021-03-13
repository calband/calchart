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

float BoundDirectionSigned(float f)
{
    while (f >= 180.0)
        f -= 360.0;
    while (f < -180.0)
        f += 360.0;
    return f;
}

bool IsDiagonalDirection(float f)
{
    f = BoundDirection(f);
    return (IS_ZERO(f - 45.0) || IS_ZERO(f - 135.0) || IS_ZERO(f - 225.0) || IS_ZERO(f - 315.0));
}

CalChart::Coord CreateVector(float dir, float mag)
{
    dir = BoundDirection(dir);
    if (IsDiagonalDirection(dir)) {
        CalChart::Coord r{ Float2CoordUnits(mag), Float2CoordUnits(mag) };
        if ((dir > 50.0) && (dir < 310.0))
            r.x = -r.x;
        if (dir < 180.0)
            r.y = -r.y;
        return r;
    } else {
        return { Float2CoordUnits(mag * cos(Deg2Rad(dir))),
            Float2CoordUnits(mag * -sin(Deg2Rad(dir))) };
    }
}

std::tuple<float, float> CreateUnitVector(float dir)
{
    dir = BoundDirection(dir);
    if (IsDiagonalDirection(dir)) {
        std::tuple<float, float> result{ 1.0, 1.0 };
        if ((dir > 50.0) && (dir < 310.0))
            std::get<0>(result) = -std::get<0>(result);
        if (dir < 180.0)
            std::get<1>(result) = -std::get<0>(result);
        return result;
    } else {
        return std::tuple<float, float>{ cos(Deg2Rad(dir)), -sin(Deg2Rad(dir)) };
    }
}
