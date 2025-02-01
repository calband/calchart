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

#include "CalChartAngles.h"

#include <compare>
#include <cstdint>
#include <format>
#include <ostream>
#include <sstream>

namespace CalChart {

inline constexpr auto kCoordShift = 4U;
inline constexpr auto kCoordDecimal = (1U << kCoordShift);

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;
template <Arithmetic T>
struct CoordBasic {
    using units = T;

    enum class CollisionType {
        none = 0,
        warning,
        intersect,
    };

    constexpr CoordBasic() = default;

    constexpr CoordBasic(CoordBasic::units xval)
        : x(xval)
        , y(0)
    {
    }

    constexpr CoordBasic(CoordBasic::units xval, CoordBasic::units yval)
        : x(xval)
        , y(yval)
    {
    }

    template <typename U>
        requires std::is_arithmetic_v<U>
    explicit constexpr CoordBasic(std::tuple<U, U> pts)
        : x(static_cast<CoordBasic::units>(std::get<0>(pts)))
        , y(static_cast<CoordBasic::units>(std::get<1>(pts)))
    {
    }

    constexpr auto operator+=(CoordBasic c) -> CoordBasic&
    {
        x = static_cast<units>(x + c.x);
        y = static_cast<units>(y + c.y);
        return *this;
    }

    constexpr auto operator-=(CoordBasic c) -> CoordBasic&
    {
        x = static_cast<units>(x - c.x);
        y = static_cast<units>(y - c.y);
        return *this;
    }

    template <typename U>
        requires std::is_arithmetic_v<U>
    constexpr auto operator*=(U s) -> CoordBasic&
    {
        x *= s;
        y *= s;
        return *this;
    }

    template <typename U>
        requires std::is_arithmetic_v<U>
    constexpr auto operator/=(U s) -> CoordBasic&
    {
        x /= s;
        y /= s;
        return *this;
    }

    constexpr auto operator<=>(CoordBasic<T> const&) const = default;

    [[nodiscard]] auto Magnitude() const -> float;
    [[nodiscard]] auto DM_Magnitude() const -> float; // check for diagonal military also

    // Get distance of point to other
    [[nodiscard]] auto Distance(CoordBasic<T> other) const { return std::hypot(other.x - x, other.y - y); }
    // Get distance of the begin to the end
    [[nodiscard]] auto Length() const { return std::hypot(x, y); }

    // Angle of vector {0, 0} and this.
    [[nodiscard]] auto Direction() const -> CalChart::Radian;
    // Direction of vector this and other
    [[nodiscard]] auto Direction(CoordBasic<T> c) const -> CalChart::Radian;

    [[nodiscard]] constexpr auto Dot(CoordBasic<T> c) const { return x * c.x + y * c.y; }

    [[nodiscard]] auto DetectCollision(CoordBasic<T> c) const -> CollisionType;

    units x{};
    units y{};
};

using Coord = CoordBasic<int>;

template <Arithmetic T>
[[nodiscard]] constexpr auto operator+(CoordBasic<T> lhs, CoordBasic<T> rhs) -> CoordBasic<T>
{
    lhs += rhs;
    return lhs;
}

[[nodiscard]] constexpr auto operator-(Coord lhs, Coord rhs) -> Coord
{
    lhs -= rhs;
    return lhs;
}

template <Arithmetic T>
[[nodiscard]] constexpr auto operator*(Coord c, T v) -> Coord { return Coord(c.x * v, c.y * v); }

template <Arithmetic T>
[[nodiscard]] constexpr auto operator*(T v, Coord c) -> Coord { return Coord(c.x * v, c.y * v); }

template <Arithmetic T>
[[nodiscard]] constexpr auto operator/(Coord a, T s) -> Coord { return Coord(a.x / static_cast<Coord::units>(s), a.y / static_cast<Coord::units>(s)); }

[[nodiscard]] constexpr auto operator-(Coord c) -> Coord { return -1 * c; }

inline auto operator<<(std::ostream& os, Coord c) -> std::ostream&
{
    return os << "(" << c.x << "," << c.y << ")";
}

// RoundToCoordUnits: Use when number is already in Coord format, just needs to be rounded.
template <Arithmetic T>
constexpr auto RoundToCoordUnits(T inCoord)
{
    constexpr auto point5 = 0.5;
    return static_cast<CalChart::Coord::units>((inCoord < 0) ? (inCoord - point5) : (inCoord + point5));
}

// RoundToCoordUnits, CoordUnits2Float
//  Use when we want to convert to Coord system
template <Arithmetic T>
constexpr auto Float2CoordUnits(T a)
{
    return static_cast<CalChart::Coord::units>(RoundToCoordUnits(a * CalChart::kCoordDecimal));
}

template <Arithmetic T>
constexpr auto CoordUnits2Float(T a)
{
    return a / static_cast<double>(CalChart::kCoordDecimal);
}

// Int2CoordUnits, CoordUnits2Int
//  Use when we want to convert to Coord system
template <Arithmetic T>
constexpr auto Int2CoordUnits(T a)
{
    return static_cast<CalChart::Coord::units>(a * CalChart::kCoordDecimal);
}
template <Arithmetic T>
constexpr auto CoordUnits2Int(T a)
{
    return static_cast<int>(a / static_cast<int>(CalChart::kCoordDecimal));
}

// Create vector is going to create a vector in that direction, except diagonals are different!
template <Arithmetic T>
constexpr auto CreateCalChartVector(CalChart::Radian dir, T mag) -> CalChart::Coord
{
    // using std::apply to apply a function to each tuple argument.
    return Coord{ std::apply([mag](auto... x) { return std::make_tuple(Float2CoordUnits(x * mag)...); }, CreateCalChartUnitVector(dir)) };
}

template <Arithmetic T>
constexpr auto CreateCalChartVector(CalChart::Degree dir, T mag) -> CalChart::Coord
{
    return CreateCalChartVector(static_cast<CalChart::Radian>(dir), mag);
}

template <Arithmetic T>
constexpr auto CreateVector(CalChart::Radian dir, T mag) -> CalChart::Coord
{
    // using std::apply to apply a function to each tuple argument.
    return Coord{ std::apply([mag](auto... x) { return std::make_tuple(Float2CoordUnits(x * mag)...); }, CreateUnitVector(dir)) };
}

template <Arithmetic T>
constexpr auto CreateVector(CalChart::Degree dir, T mag) -> CalChart::Coord
{
    return CreateVector(static_cast<CalChart::Radian>(dir), mag);
}

template <Arithmetic T>
constexpr auto CreateCoordVector(CalChart::Radian dir, T mag) -> CalChart::Coord
{
    // using std::apply to apply a function to each tuple argument.
    return Coord{ std::apply([mag](auto... x) { return std::make_tuple((x * mag)...); }, CreateUnitVector(dir)) };
}

template <Arithmetic T>
constexpr auto CreateCoordVector(CalChart::Degree dir, T mag) -> CalChart::Coord
{
    return CreateCoordVector(static_cast<CalChart::Radian>(dir), mag);
}

// Implementation details
// Angle of vector {0, 0} and this.
template <Arithmetic T>
inline auto CoordBasic<T>::Direction() const -> CalChart::Radian
{
    if (*this == CoordBasic<T>{}) {
        return CalChart::Radian{};
    }

    auto ang = acos(CoordUnits2Float(x) / Magnitude()); // normalize
    if (y > 0) {
        ang = (-ang); // check for > PI
    }

    return CalChart::Radian{ ang };
}

// Direction of vector this and other
template <Arithmetic T>
inline auto CoordBasic<T>::Direction(CoordBasic<T> c) const -> CalChart::Radian
{
    return (c - *this).Direction();
}

// Get magnitude of vector
template <Arithmetic T>
inline auto CoordBasic<T>::Magnitude() const -> float
{
    return static_cast<float>(std::hypot(CoordUnits2Float(x), CoordUnits2Float(y)));
}

// Get magnitude, but check for diagonal military
template <Arithmetic T>
inline auto CoordBasic<T>::DM_Magnitude() const -> float
{
    if ((x == y) || (x == -y)) {
        return static_cast<float>(CoordUnits2Float(std::abs(x)));
    }
    return Magnitude();
}

// Returns the type of collision between this point and another
template <Arithmetic T>
inline auto CoordBasic<T>::DetectCollision(CoordBasic<T> c) const -> CoordBasic<T>::CollisionType
{
    auto dx = x - c.x;
    auto dy = y - c.y;
    // Check for special cases to avoid multiplications
    if (std::abs(dx) > Int2CoordUnits(1)) {
        return CollisionType::none;
    }
    if (std::abs(dy) > Int2CoordUnits(1)) {
        return CollisionType::none;
    }
    auto squaredDist = ((dx * dx) + (dy * dy));
    auto distOfOne = Int2CoordUnits(1) * Int2CoordUnits(1);
    if (squaredDist < distOfOne) {
        return CollisionType::intersect;
    }
    if (squaredDist == distOfOne) {
        return CollisionType::warning;
    }
    return CollisionType::none;
}
}

// Custom formatter specialization for CalChart::Coord
template <>
struct std::formatter<CalChart::Coord> {
    // No parse function needed since we have no format specifiers
    static constexpr auto parse(std::format_parse_context& ctx)
    {
        return ctx.begin();
    }

    // Formats the CalChart::Coord object
    template <typename FormatContext>
    static auto format(const CalChart::Coord& coord, FormatContext& ctx)
    {
        return std::format_to(ctx.out(), "({}, {})", coord.x, coord.y);
    }
};

namespace CalChart::details {
static_assert(Coord{} == Coord{ 0 });
static_assert(Coord{ 1 } == Coord{ 1, 0 });
static_assert(Coord{ std::tuple{ 1.0, 1.0 } } == Coord{ 1, 1 });
static_assert(Coord{ 0, 0 } <= Coord{ 0, 0 });
static_assert(Coord{ -1, 0 } < Coord{ 0, 0 });
static_assert(Coord{ -1, -1 } < Coord{ -1, 0 });
static_assert(Coord{ 1, 1 } == (Coord{ 1, 0 } + Coord{ 0, 1 }));
static_assert(Coord{ 1, 1 } != (Coord{ 1, 0 } + Coord{ 1, 0 }));
static_assert(Coord{ 2, 2 } == 2 * Coord{ 1, 1 });
static_assert(Coord{ 2, 2 } == Coord{ 1, 1 } * 2);
static_assert(Coord{ 1, 1 } == Coord{ 2, 2 } / 2);
static_assert(Coord{ 1, -1 } == -Coord{ -1, 1 });
static_assert(Coord{ -1, 1 } == -Coord{ 1, -1 });

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
static_assert(1 == RoundToCoordUnits(0.5));
static_assert(0 == RoundToCoordUnits(0.4));
static_assert(-1 == RoundToCoordUnits(-0.5));
static_assert(0 == RoundToCoordUnits(-0.4));

static_assert(8 == Float2CoordUnits(0.5));
static_assert(6 == Float2CoordUnits(0.4));
static_assert(-8 == Float2CoordUnits(-0.5));
static_assert(-6 == Float2CoordUnits(-0.4));
static_assert(16 == Float2CoordUnits(1));
static_assert(16 == Float2CoordUnits(1.0));
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}
