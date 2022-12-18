/*
 * CalChartCoord.cpp
 * Definitions for the coordinate object
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

#include "CalChartCoord.h"
#include "CalChartUtils.h"
#include <random>

namespace CalChart {

// Get magnitude of vector
float Coord::Magnitude() const
{
    return std::hypot(CoordUnits2Float(x), CoordUnits2Float(y));
}

// Get distance of point to other
float Coord::Distance(Coord const& other) const
{
    return std::hypot(other.x - x, other.y - y);
}

bool Coord::CloserThan(Coord const& other, float tolerance) const
{
    return Distance(other) < tolerance;
}

// Get magnitude, but check for diagonal military
float Coord::DM_Magnitude() const
{
    if ((x == y) || (x == -y)) {
        return CoordUnits2Float(std::abs(x));
    } else {
        return Magnitude();
    }
}

// Get direction of this vector
CalChart::Radian Coord::Direction() const
{
    if (*this == Coord{ 0 })
        return CalChart::Radian{};

    auto ang = acos(CoordUnits2Float(x) / Magnitude()); // normalize
    if (y > 0)
        ang = (-ang); // check for > PI

    return CalChart::Radian{ ang };
}

// Get direction from this coord to another
CalChart::Radian Coord::Direction(Coord const& c) const
{
    return (c - *this).Direction();
}

// Returns the type of collision between this point and another
Coord::CollisionType Coord::DetectCollision(Coord const& c) const
{
    auto dx = x - c.x;
    auto dy = y - c.y;
    // Check for special cases to avoid multiplications
    if (std::abs(dx) > Int2CoordUnits(1))
        return CollisionType::none;
    if (std::abs(dy) > Int2CoordUnits(1))
        return CollisionType::none;
    auto squaredDist = ((dx * dx) + (dy * dy));
    auto distOfOne = Int2CoordUnits(1) * Int2CoordUnits(1);
    if (squaredDist < distOfOne) {
        return CollisionType::intersect;
    } else if (squaredDist == distOfOne) {
        return CollisionType::warning;
    } else {
        return CollisionType::none;
    }
}

#include <cassert>

// Test Suite stuff
struct Coord_values {
    Coord::units x, y;
};

bool Check_Coord(Coord const& underTest, Coord_values const& values)
{
    return (underTest.x == values.x) && (underTest.y == values.y);
}

void Coord_UnitTests()
{
    static constexpr auto kNumRand = 10;
    // test some defaults:
    {
        Coord undertest;
        assert(0.0 == undertest.Magnitude());
        assert(0.0 == undertest.DM_Magnitude());
        assert(CalChart::Radian{} == undertest.Direction());
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<Coord::units> distrib;
    // test equality:
    for (size_t i = 0; i < kNumRand; ++i) {
        Coord::units x1, y1, x2, y2;
        x1 = distrib(gen);
        y1 = distrib(gen);
        x2 = distrib(gen);
        y2 = distrib(gen);
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

        Coord::units x3, y3;

        auto newValue = undertest1 + undertest2;
        x3 = undertest1.x + undertest2.x;
        y3 = undertest1.y + undertest2.y;
        (void)x3;
        (void)y3;
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

        short factor = distrib(gen);
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
