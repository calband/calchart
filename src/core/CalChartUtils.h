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

#include <cmath>
#include <numbers>
#include <tuple>

namespace CalChart {

/**
 * Math Utilities
 */

// Should be constexpr in c++23
template <typename T>
inline auto IS_ZERO(T a)
{
    // the fudge factor for floating point math
    constexpr auto kEpsilon = 1e-6;
    return std::abs(a) < kEpsilon;
}

template <typename T, typename U>
inline auto IS_EQUAL(T a, U b) { return IS_ZERO(b - a); }

static constexpr auto two_pi = 2.0 * std::numbers::pi;
static constexpr auto rad_per_deg = std::numbers::pi / 180.0;
static constexpr auto deg_per_rad = 180.0 / std::numbers::pi;

template <typename T>
constexpr auto Deg2Rad(T angle) { return angle * rad_per_deg; }
template <typename T>
constexpr auto Rad2Deg(T angle) { return angle * deg_per_rad; }

template <typename T>
inline auto BoundDirectionRad(T angle)
{
    angle = fmod(angle, two_pi);
    if (angle < 0.0) {
        angle += two_pi;
    }
    return angle;
}

template <typename T>
inline auto BoundDirectionDeg(T angle) { return Rad2Deg(BoundDirectionRad(Deg2Rad(angle))); }

template <typename T>
inline auto BoundDirectionSignedRad(T angle) { return BoundDirectionRad(angle + std::numbers::pi) - std::numbers::pi; }

template <typename T>
inline auto BoundDirectionSignedDeg(T angle) { return Rad2Deg(BoundDirectionSignedRad(Deg2Rad(angle))); }

template <typename T>
inline auto NormalizeAngleDeg(T angle) { return BoundDirectionDeg(angle); }

template <typename T>
inline auto NormalizeAngleRad(T angle) { return BoundDirectionRad(angle); }

// we assume the layout of quadrants are clockwise
template <typename T>
inline auto AngleToQuadrantRad(T angle)
{
    // rotate angle by a quarter:
    constexpr auto quadrants = 8;
    angle = NormalizeAngleRad(angle + std::numbers::pi / quadrants);

    return (quadrants + static_cast<int>(-angle / (two_pi / quadrants))) % quadrants;
}

template <typename T>
inline auto AngleToQuadrantDeg(T angle) { return AngleToQuadrantRad(Deg2Rad(angle)); }

template <typename T>
inline auto IsDiagonalDirectionRad(T angle)
{
    angle = BoundDirectionRad(angle);
    constexpr auto pi_1_4 = std::numbers::pi / 4;
    constexpr auto pi_3_4 = 3 * std::numbers::pi / 4;
    constexpr auto pi_5_4 = 5 * std::numbers::pi / 4;
    constexpr auto pi_7_4 = 7 * std::numbers::pi / 4;
    return (IS_ZERO(angle - pi_1_4) || IS_ZERO(angle - pi_3_4) || IS_ZERO(angle - pi_5_4) || IS_ZERO(angle - pi_7_4));
}

template <typename T>
inline auto IsDiagonalDirectionDeg(T angle) { return IsDiagonalDirectionRad(Deg2Rad(angle)); }

template <typename T>
inline auto CreateUnitVectorRad(T angle)
{
    // our coordinate system is clockwise.
    angle = -BoundDirectionRad(angle);
    return std::tuple<decltype(angle), decltype(angle)>{ std::cos(angle), std::sin(angle) };
}

template <typename T>
inline auto CreateUnitVectorDeg(T angle) { return CreateUnitVectorRad(Deg2Rad(angle)); }

// UnitVector is a very special beast in CalChart.  Diagonals are 1,1 instead of sqrt(2)/2
template <typename T>
inline auto CreateCalChartUnitVectorRad(T angle)
{
    auto result = CreateUnitVectorRad(angle);
    if (IsDiagonalDirectionRad(angle)) {
        std::get<0>(result) /= std::numbers::sqrt2 / 2;
        std::get<1>(result) /= std::numbers::sqrt2 / 2;
    }
    return result;
}

template <typename T>
inline auto CreateCalChartUnitVectorDeg(T angle) { return CreateCalChartUnitVectorRad(Deg2Rad(angle)); }

/**
 * Common utils
 */
template <typename E>
constexpr auto toUType(E enumerator)
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

// from https://stackoverflow.com/questions/261963/how-can-i-iterate-over-an-enum
template <typename C, C beginVal, C endVal>
class Iterator {
    using val_t = typename std::underlying_type<C>::type;
    val_t val;

public:
    explicit Iterator(const C& f)
        : val(static_cast<val_t>(f))
    {
    }
    Iterator()
        : val(static_cast<val_t>(beginVal))
    {
    }
    auto operator++() -> Iterator
    {
        ++val;
        return *this;
    }
    auto operator*() const { return static_cast<C>(val); }
    auto begin() const -> Iterator { return *this; } // default ctor is good
    auto end() const -> Iterator
    {
        static const Iterator endIter = ++Iterator(endVal); // cache it
        return endIter;
    }
    auto operator!=(const Iterator& i) const { return val != i.val; }
};

// static tests
namespace details {
    constexpr auto t_180 = 180.0;
    static_assert(Deg2Rad(t_180) == std::numbers::pi);
    static_assert(Rad2Deg(std::numbers::pi) == t_180);
}

}
