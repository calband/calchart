#pragma once
/*
 * CalChartCoord.h
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

#include <cstdint>


namespace CalChart {

constexpr auto kCoordShift = 4;
constexpr auto kCoordDecimal = (1 << kCoordShift);

struct Coord {
    using units = int16_t;

    enum class CollisionType {
        none = 0,
        warning,
        intersect,
    };

    constexpr Coord(Coord::units xval = 0, Coord::units yval = 0)
        : x(xval)
        , y(yval)
    {
    }

    float Magnitude() const;
    float Distance(Coord const&) const;
    float DM_Magnitude() const; // check for diagonal military also
    float Direction() const;
    float Direction(Coord const& c) const;

    CollisionType DetectCollision(Coord const& c) const;

    constexpr Coord& operator+=(Coord const& c)
    {
        x += c.x;
        y += c.y;
        return *this;
    }

    constexpr Coord& operator-=(Coord const& c)
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
};

inline Coord operator+(Coord const& a, Coord const& b)
{
    return Coord(a.x + b.x, a.y + b.y);
}

inline Coord operator-(Coord const& a, Coord const& b)
{
    return Coord(a.x - b.x, a.y - b.y);
}

inline Coord operator*(Coord const& a, int s)
{
    return Coord(a.x * s, a.y * s);
}

inline Coord operator/(Coord const& a, int s)
{
    return Coord(a.x / s, a.y / s);
}

inline Coord operator-(Coord const& c) { return Coord(-c.x, -c.y); }

inline bool operator==(Coord const& a, Coord const& b)
{
    return ((a.x == b.x) && (a.y == b.y));
}

inline bool operator<(Coord const& a, Coord const& b)
{
    return (a.y == b.y) ? (a.x < b.x) : (a.y < b.y);
}

inline bool operator!=(Coord const& a, Coord const& b)
{
    return ((a.x != b.x) || (a.y != b.y));
}

void Coord_UnitTests();
}

// RoundToCoordUnits: Use when number is already in Coord format, just needs to be
// rounded
template <typename T>
constexpr auto RoundToCoordUnits(T inCoord)
{
    return static_cast<CalChart::Coord::units>((inCoord < 0) ? (inCoord - 0.5) : (inCoord + 0.5));
}

// RoundToCoordUnits, CoordUnits2Float
//  Use when we want to convert to Coord system
template <typename T>
constexpr auto Float2CoordUnits(T a)
{
    return static_cast<CalChart::Coord::units>(RoundToCoordUnits(a * CalChart::kCoordDecimal));
}
template <typename T>
constexpr auto CoordUnits2Float(T a)
{
    return a / (float)CalChart::kCoordDecimal;
}

// Int2CoordUnits, CoordUnits2Int
//  Use when we want to convert to Coord system
template <typename T>
constexpr auto Int2CoordUnits(T a)
{
    return static_cast<CalChart::Coord::units>(a * CalChart::kCoordDecimal);
}
template <typename T>
constexpr auto CoordUnits2Int(T a)
{
    return static_cast<int>(a / CalChart::kCoordDecimal);
}
