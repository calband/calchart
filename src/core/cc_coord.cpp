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

#define _USE_MATH_DEFINES
#include <cmath>

#include "cc_coord.h"
#include <cstdlib>

namespace CalChart {

// Get magnitude of vector
float Coord::Magnitude() const
{
    return std::hypot(CoordUnits2Float(x), CoordUnits2Float(y));
}

// Get magnitude, but check for diagonal military
float Coord::DM_Magnitude() const
{
    if ((x == y) || (x == -y)) {
        return CoordUnits2Float(std::abs(x));
    }
    else {
        return Magnitude();
    }
}

// Get direction of this vector
float Coord::Direction() const
{
    if (*this == 0)
        return 0.0;

    auto ang = acos(CoordUnits2Float(x) / Magnitude()); // normalize
    ang *= static_cast<float>(180.0 / M_PI); // convert to degrees
    if (y > 0)
        ang = (-ang); // check for > PI

    return ang;
}

// Get direction from this coord to another
float Coord::Direction(const Coord& c) const
{
    return (c - *this).Direction();
}

// Returns the type of collision between this point and another
Coord::CollisionType Coord::DetectCollision(const Coord& c) const
{
    auto dx = x - c.x;
    auto dy = y - c.y;
    // Check for special cases to avoid multiplications
    if (std::abs(dx) > Int2CoordUnits(1))
        return COLLISION_NONE;
    if (std::abs(dy) > Int2CoordUnits(1))
        return COLLISION_NONE;
    auto squaredDist = ((dx * dx) + (dy * dy));
    auto distOfOne = Int2CoordUnits(1) * Int2CoordUnits(1);
    if (squaredDist < distOfOne) {
        return COLLISION_INTERSECT;
    }
    else if (squaredDist == distOfOne) {
        return COLLISION_WARNING;
    }
    else {
        return COLLISION_NONE;
    }
}

#include <assert.h>

// Test Suite stuff
struct Coord_values {
    Coord::units x, y;
};

bool Check_Coord(const Coord& underTest, const Coord_values& values)
{
    return (underTest.x == values.x) && (underTest.y == values.y);
}

void Coord_UnitTests()
{
    static const size_t kNumRand = 10;
    // test some defaults:
    {
        Coord undertest;
        assert(0.0 == undertest.Magnitude());
        assert(0.0 == undertest.DM_Magnitude());
        assert(0.0 == undertest.Direction());
    }

    // test equality:
    for (size_t i = 0; i < kNumRand; ++i) {
        Coord::units x1, y1, x2, y2;
        x1 = rand();
        y1 = rand();
        x2 = rand();
        y2 = rand();
        Coord undertest1(x1, y1);
        Coord undertest2(x2, y2);
        assert(x1 == undertest1.x);
        assert(y1 == undertest1.y);
        assert(x2 == undertest2.x);
        assert(y2 == undertest2.y);
        assert(Coord(x1, y1) == undertest1);
        assert(Coord(x2, y2) == undertest2);
        assert(!(Coord(x1, y1) != undertest1));
        assert(!(Coord(x2, y2) != undertest2));

        Coord newValue;

        Coord::units x3, y3;

        newValue = undertest1 + undertest2;
        x3 = undertest1.x + undertest2.x;
        y3 = undertest1.y + undertest2.y;
        assert((x3) == newValue.x);
        assert((y3) == newValue.y);
        newValue = undertest1;
        newValue += undertest2;
        assert((x3) == newValue.x);
        assert((y3) == newValue.y);
        newValue = undertest1 - undertest2;
        x3 = undertest1.x - undertest2.x;
        y3 = undertest1.y - undertest2.y;
        assert((x3) == newValue.x);
        assert((y3) == newValue.y);
        newValue = undertest1;
        newValue -= undertest2;
        assert((x3) == newValue.x);
        assert((y3) == newValue.y);

        newValue = -undertest1;
        assert((-undertest1.x) == newValue.x);
        assert((-undertest1.y) == newValue.y);

        short factor = rand();
        newValue = undertest1 * factor;
        x3 = undertest1.x * factor;
        y3 = undertest1.y * factor;
        assert((x3) == newValue.x);
        assert((y3) == newValue.y);
        newValue = undertest1;
        newValue *= factor;
        assert((x3) == newValue.x);
        assert((y3) == newValue.y);
        // avoid div by 0
        if (factor) {
            newValue = undertest1 / factor;
            x3 = undertest1.x / factor;
            y3 = undertest1.y / factor;
            assert((x3) == newValue.x);
            assert((y3) == newValue.y);
            newValue = undertest1;
            newValue /= factor;
            assert((x3) == newValue.x);
            assert((y3) == newValue.y);
        }
    }
}
}
