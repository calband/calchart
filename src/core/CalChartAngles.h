#pragma once
/*
 * CalChartAngles.h
 * Details related to angles and math with angles.
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartUtils.h"
#include <cmath>
#include <compare>
#include <numbers>
#include <ostream>

namespace CalChart {

struct Radian;
struct Degree;

// we lay out directions in clockwise order from North
enum class Direction {
    North,
    NorthEast,
    East,
    SouthEast,
    South,
    SouthWest,
    West,
    NorthWest,
};

// we generally implement Degree related functions in Radian.
struct Radian {
    template <typename T>
    explicit constexpr Radian(T v)
        : value(v)
    {
    }
    explicit constexpr Radian()
        : Radian(0.0)
    {
    }
    explicit constexpr Radian(Degree v);

    [[nodiscard]] explicit operator Degree() const;
    [[nodiscard]] constexpr auto getValue() const { return value; }

    constexpr auto operator<=>(Radian const&) const = default;
    // Comparision specifically using IS_EQUAL and with wrap.
    // meaning: 2*pi == 0
    [[nodiscard]] inline auto IsEqual(Radian v) const -> bool;

    [[nodiscard]] constexpr auto operator+(Radian v) const { return Radian{ value + v.value }; }
    [[nodiscard]] constexpr auto operator-(Radian v) const { return Radian{ value - v.value }; }
    [[nodiscard]] constexpr auto operator*(double v) const { return Radian{ value * v }; }
    [[nodiscard]] constexpr auto operator/(double v) const { return Radian{ value / v }; }

    constexpr auto operator+=(Radian v) -> Radian&
    {
        value += v.value;
        return *this;
    }
    constexpr auto operator-=(Radian v) -> Radian&
    {
        value -= v.value;
        return *this;
    }
    constexpr auto operator*=(double v) -> Radian&
    {
        value *= v;
        return *this;
    }
    constexpr auto operator/=(double v) -> Radian&
    {
        value /= v;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const { return Radian{ -value }; }

private:
    double value;
};

static inline constexpr auto pi = Radian{ std::numbers::pi };

struct Degree {
    template <typename T>
    explicit constexpr Degree(T v)
        : value(v)
    {
    }
    explicit constexpr Degree()
        : Degree(0.0)
    {
    }
    explicit constexpr Degree(Radian v);

    [[nodiscard]] explicit operator Radian() const;
    [[nodiscard]] constexpr auto getValue() const { return value; }

    constexpr auto operator<=>(Degree const&) const = default;
    // Comparision specifically using IS_EQUAL
    [[nodiscard]] inline auto IsEqual(Degree v) const -> bool;

    [[nodiscard]] constexpr auto operator+(Degree v) const { return Degree{ value + v.value }; }
    [[nodiscard]] constexpr auto operator-(Degree v) const { return Degree{ value - v.value }; }
    [[nodiscard]] constexpr auto operator*(double v) const { return Degree{ value * v }; }
    [[nodiscard]] constexpr auto operator/(double v) const { return Degree{ value / v }; }

    constexpr auto operator+=(Degree v) -> Degree&
    {
        value += v.value;
        return *this;
    }
    constexpr auto operator-=(Degree v) -> Degree&
    {
        value -= v.value;
        return *this;
    }
    constexpr auto operator*=(double v) -> Degree&
    {
        value *= v;
        return *this;
    }
    constexpr auto operator/=(double v) -> Degree&
    {
        value /= v;
        return *this;
    }

    [[nodiscard]] constexpr auto operator-() const { return Degree{ -value }; }

private:
    double value;
};

// Bound within the range [0, 2*pi)
inline auto BoundDirection(Radian v) -> Radian;
inline auto BoundDirection(Degree v) -> Degree;
// Bound within the range [-pi, pi)
inline auto BoundDirectionSigned(Radian v) -> Radian;
inline auto BoundDirectionSigned(Degree v) -> Degree;
inline auto NormalizeAngle(Radian v) -> Radian;
inline auto NormalizeAngle(Degree v) -> Degree;
inline auto AngleToQuadrant(Radian v) -> int;
inline auto AngleToQuadrant(Degree v) -> int;
inline auto AngleToDirection(Radian v) -> CalChart::Direction;
inline auto AngleToDirection(Degree v) -> CalChart::Direction;
inline auto IsDiagonalDirection(Radian v) -> bool;
inline auto IsDiagonalDirection(Degree v) -> bool;
inline auto CreateUnitVector(Radian v) -> std::tuple<double, double>;
inline auto CreateUnitVector(Degree v) -> std::tuple<double, double>;
// UnitVector is a very special beast in CalChart.  Diagonals are 1,1 instead of sqrt(2)/2
inline auto CreateCalChartUnitVector(Radian v) -> std::tuple<double, double>;
inline auto CreateCalChartUnitVector(Degree v) -> std::tuple<double, double>;
inline auto cos(Radian v) -> double;
inline auto cos(Degree v) -> double;
inline auto sin(Radian v) -> double;
inline auto sin(Degree v) -> double;

// implementation details

namespace details {
    static constexpr auto two_pi = 2.0 * std::numbers::pi;
    static constexpr auto rad_per_deg = std::numbers::pi / 180.0;
    static constexpr auto deg_per_rad = 180.0 / std::numbers::pi;

    template <typename T>
    constexpr auto Deg2Rad(T angle) { return angle * rad_per_deg; }
    template <typename T>
    constexpr auto Rad2Deg(T angle) { return angle * deg_per_rad; }

}

inline constexpr auto operator*(double v, Radian r) -> Radian
{
    return r * v;
}

inline constexpr auto operator*(double v, Degree r) -> Degree
{
    return r * v;
}

inline auto operator<<(std::ostream& os, Radian v) -> std::ostream& { return os << v.getValue(); }
inline auto operator<<(std::ostream& os, Degree v) -> std::ostream& { return os << v.getValue(); }

inline constexpr Radian::Radian(Degree v)
    : Radian(details::Deg2Rad(v.getValue()))
{
}

inline constexpr Degree::Degree(Radian v)
    : Degree(details::Rad2Deg(v.getValue()))
{
}

inline Radian::operator Degree() const
{
    return Degree{ details::Rad2Deg(value) };
}

inline Degree::operator Radian() const
{
    return Radian{ details::Deg2Rad(value) };
}

inline auto Radian::IsEqual(Radian v) const -> bool
{
    return IS_EQUAL(BoundDirection(*this).getValue(), BoundDirection(v).getValue());
}

inline auto Degree::IsEqual(Degree v) const -> bool
{
    return IS_EQUAL(BoundDirection(*this).getValue(), BoundDirection(v).getValue());
}

inline auto BoundDirection(Radian v) -> Radian
{
    auto angle = fmod(v.getValue(), details::two_pi);
    if (angle < 0.0) {
        angle += details::two_pi;
    }
    return Radian{ angle };
}

inline auto BoundDirection(Degree v) -> Degree { return Degree{ BoundDirection(static_cast<Radian>(v)) }; }

inline auto BoundDirectionSigned(Radian v) -> Radian { return BoundDirection(v + Radian{ std::numbers::pi }) - Radian{ std::numbers::pi }; }
inline auto BoundDirectionSigned(Degree v) -> Degree { return Degree{ BoundDirectionSigned(static_cast<Radian>(v)) }; }

inline auto NormalizeAngle(Radian v) -> Radian { return BoundDirection(v); }
inline auto NormalizeAngle(Degree v) -> Degree { return BoundDirection(v); }

// we assume the layout of quadrants are clockwise
inline auto AngleToQuadrant(Radian v) -> int
{
    // rotate angle by a quarter:
    constexpr auto quadrants = 8;
    auto angle = -BoundDirection(v + Radian{ std::numbers::pi } / quadrants) / (details::two_pi / quadrants);

    return (quadrants + static_cast<int>(angle.getValue())) % quadrants;
}

inline auto AngleToQuadrant(Degree v) -> int { return AngleToQuadrant(static_cast<Radian>(v)); }

inline auto AngleToDirection(Radian v) -> CalChart::Direction { return static_cast<CalChart::Direction>(AngleToQuadrant(v)); }

inline auto AngleToDirection(Degree v) -> CalChart::Direction { return static_cast<CalChart::Direction>(AngleToQuadrant(v)); }

inline auto IsDiagonalDirection(Radian v) -> bool
{
    auto angle = BoundDirection(v);
    constexpr auto pi_1_4 = Radian{ std::numbers::pi / 4 };
    constexpr auto pi_3_4 = Radian{ 3 * std::numbers::pi / 4 };
    constexpr auto pi_5_4 = Radian{ 5 * std::numbers::pi / 4 };
    constexpr auto pi_7_4 = Radian{ 7 * std::numbers::pi / 4 };
    return angle.IsEqual(pi_1_4) || angle.IsEqual(pi_3_4) || angle.IsEqual(pi_5_4) || angle.IsEqual(pi_7_4);
}

inline auto IsDiagonalDirection(Degree v) -> bool { return IsDiagonalDirection(static_cast<Radian>(v)); }

inline auto CreateUnitVector(Radian v) -> std::tuple<double, double>
{
    // our coordinate system is clockwise.
    auto angle = -BoundDirection(v);
    return std::tuple{ std::cos(angle.getValue()), std::sin(angle.getValue()) };
}

inline auto CreateUnitVector(Degree v) -> std::tuple<double, double> { return CreateUnitVector(static_cast<Radian>(v)); }

inline auto CreateCalChartUnitVector(Radian v) -> std::tuple<double, double>
{
    auto result = CreateUnitVector(v);
    if (IsDiagonalDirection(v)) {
        std::get<0>(result) /= std::numbers::sqrt2 / 2;
        std::get<1>(result) /= std::numbers::sqrt2 / 2;
    }
    return result;
}

inline auto CreateCalChartUnitVector(Degree v) -> std::tuple<double, double> { return CreateCalChartUnitVector(static_cast<Radian>(v)); }

inline auto cos(Radian v) -> double { return std::cos(v.getValue()); }
inline auto cos(Degree v) -> double { return cos(Radian{ v }); }
inline auto sin(Radian v) -> double { return std::sin(v.getValue()); }
inline auto sin(Degree v) -> double { return sin(Radian{ v }); }

// static tests
namespace details {
    constexpr auto t_180 = 180.0;
    constexpr auto point5 = 1.0 / 2;
    static_assert(details::Deg2Rad(t_180) == std::numbers::pi);
    static_assert(details::Rad2Deg(std::numbers::pi) == t_180);

    static_assert(Radian(1.0) == (Radian(point5) + Radian(point5)));
    static_assert(Radian(point5) == (Radian(1.0) - Radian(point5)));
    static_assert(Radian(1.0) == (Radian(point5) * 2));
    static_assert(Radian(1.0) == (2 * Radian(point5)));
    static_assert(Radian(point5) == (Radian(1.0) / 2));

    static_assert(Degree(1.0) == (Degree(point5) + Degree(point5)));
    static_assert(Degree(point5) == (Degree(1.0) - Degree(point5)));
    static_assert(Degree(1.0) == (Degree(point5) * 2));
    static_assert(Degree(1.0) == (2 * Degree(point5)));
    static_assert(Degree(point5) == (Degree(1.0) / 2));
}
}
