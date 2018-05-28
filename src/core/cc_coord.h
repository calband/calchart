#pragma once
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

#include <cstdint>

#include <boost/serialization/access.hpp>

namespace CalChart {

class Coord {
public:
    using units = int16_t;

    Coord(Coord::units xval = 0, Coord::units yval = 0)
        : x(xval)
        , y(yval)
    {
    }

    float Magnitude() const;
    float Distance(Coord const&) const;
    float DM_Magnitude() const; // check for diagonal military also
    float Direction() const;
    float Direction(const Coord& c) const;

    enum CollisionType {
        COLLISION_NONE = 0,
        COLLISION_WARNING,
        COLLISION_INTERSECT
    };

    CollisionType DetectCollision(const Coord& c) const;

    Coord& operator+=(const Coord& c)
    {
        x += c.x;
        y += c.y;
        return *this;
    }
    Coord& operator-=(const Coord& c)
    {
        x -= c.x;
        y -= c.y;
        return *this;
    }
    Coord& operator*=(int s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    Coord& operator/=(int s)
    {
        x /= s;
        y /= s;
        return *this;
    }
    Coord& operator*=(double s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    Coord& operator/=(double s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    units x, y;

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& x;
        ar& y;
    }
};

inline Coord operator+(const Coord& a, const Coord& b)
{
    return Coord(a.x + b.x, a.y + b.y);
}

inline Coord operator-(const Coord& a, const Coord& b)
{
    return Coord(a.x - b.x, a.y - b.y);
}

inline Coord operator*(const Coord& a, int s)
{
    return Coord(a.x * s, a.y * s);
}

inline Coord operator/(const Coord& a, int s)
{
    return Coord(a.x / s, a.y / s);
}

inline Coord operator-(const Coord& c) { return Coord(-c.x, -c.y); }

inline bool operator==(const Coord& a, const Coord& b)
{
    return ((a.x == b.x) && (a.y == b.y));
}

inline bool operator<(const Coord& a, const Coord& b)
{
    return (a.y == b.y) ? (a.x < b.x) : (a.y < b.y);
}

inline bool operator!=(const Coord& a, const Coord& b)
{
    return ((a.x != b.x) || (a.y != b.y));
}

void Coord_UnitTests();
}

constexpr auto COORD_SHIFT = 4;
constexpr auto COORD_DECIMAL = (1 << COORD_SHIFT);

// RoundToCoordUnits: Use when number is already in Coord format, just needs to be
// rounded
template <typename T>
auto RoundToCoordUnits(T inCoord)
{
    return static_cast<CalChart::Coord::units>((inCoord < 0) ? (inCoord - 0.5) : (inCoord + 0.5));
}

// RoundToCoordUnits, CoordUnits2Float
//  Use when we want to convert to Coord system
template <typename T>
auto Float2CoordUnits(T a)
{
    return static_cast<CalChart::Coord::units>(RoundToCoordUnits(a * COORD_DECIMAL));
}
template <typename T>
auto CoordUnits2Float(T a)
{
    return a / (float)COORD_DECIMAL;
}

// Int2CoordUnits, CoordUnits2Int
//  Use when we want to convert to Coord system
template <typename T>
constexpr auto Int2CoordUnits(T a)
{
    return static_cast<CalChart::Coord::units>(a * COORD_DECIMAL);
}
template <typename T>
auto CoordUnits2Int(T a)
{
    return static_cast<int>(a / COORD_DECIMAL);
}
