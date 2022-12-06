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
template <typename T>
inline auto IS_ZERO(T a)
{
    // the fudge factor for floating point math
    constexpr auto kEpsilon = 1e-6;
    return std::abs(a) < kEpsilon;
}
template <typename T>
constexpr auto Deg2Rad(T a) { return a * std::numbers::pi / 180.0; }
template <typename T>
constexpr auto Rad2Deg(T a) { return a * 180.0 / std::numbers::pi; }

template <typename T>
constexpr auto BoundDirection(T f)
{
    while (f >= 360.0) {
        f -= 360.0;
    }
    while (f < 0.0) {
        f += 360.0;
    }
    return f;
}

template <typename T>
constexpr auto BoundDirectionSigned(T f)
{
    while (f >= 180.0) {
        f -= 360.0;
    }
    while (f < -180.0) {
        f += 360.0;
    }
    return f;
}

template <typename T>
constexpr auto BoundDirectionRad(T f)
{
    while (f >= 2 * std::numbers::pi) {
        f -= 2 * std::numbers::pi;
    }
    while (f < 0.0) {
        f += 2 * std::numbers::pi;
    }
    return f;
}

template <typename T>
constexpr auto NormalizeAngle(T ang) { return BoundDirection(ang); }
template <typename T>
constexpr auto NormalizeAngleRad(T ang) { return BoundDirectionRad(ang); }

// we assume the layout of quadrants are
template <typename T>
constexpr auto AngleToQuadrant(T ang)
{
    // rotate angle by 22.5:
    auto ang1 = ang + 22.5;
    ang1 = NormalizeAngle(ang1);

    return (8 + static_cast<int>(-ang1 / 45.0)) % 8;
}

template <typename T>
inline auto IsDiagonalDirection(T f)
{
    f = BoundDirection(f);
    return (IS_ZERO(f - 45.0) || IS_ZERO(f - 135.0) || IS_ZERO(f - 225.0) || IS_ZERO(f - 315.0));
}

template <typename T>
inline auto CreateUnitVector(T dir)
{
    dir = -BoundDirection(dir);
    auto result = std::tuple<decltype(dir), decltype(dir)>{ cos(Deg2Rad(dir)), sin(Deg2Rad(dir)) };
    // we 'normalize' the diagonal direction to 1,1 because that be the CalChart way.
    if (IsDiagonalDirection(dir)) {
        std::get<0>(result) /= std::numbers::sqrt2 / 2;
        std::get<1>(result) /= std::numbers::sqrt2 / 2;
    }
    if (dir > 180) {
        std::get<1>(result) = -std::get<1>(result);
    }
    return result;
}

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
static_assert(Deg2Rad(180.0) == std::numbers::pi);
static_assert(Rad2Deg(std::numbers::pi) == 180.0);
static_assert(BoundDirection(1082.0) == 2.0);
static_assert(BoundDirection(2.0) == 2.0);
static_assert(BoundDirection(-1082.0) == 358.0);
static_assert(BoundDirection(-2.0) == 358.0);
static_assert(BoundDirectionSigned(182.0) == -178.0);
static_assert(BoundDirectionSigned(-182.0) == 178.0);
static_assert(BoundDirectionRad(2.0) == 2.0);
static_assert(BoundDirectionRad(-2.0) == 2 * (std::numbers::pi - 1.0));
static_assert(BoundDirection(1082.0) == NormalizeAngle(1082.0));
static_assert(BoundDirectionRad(-2.0) == NormalizeAngleRad(-2.0));
static_assert(AngleToQuadrant(0) == 0);
static_assert(AngleToQuadrant(44) == 7);
static_assert(AngleToQuadrant(45) == 7);
static_assert(AngleToQuadrant(90) == 6);
static_assert(AngleToQuadrant(135) == 5);
static_assert(AngleToQuadrant(160) == 4);
static_assert(AngleToQuadrant(180) == 4);
static_assert(AngleToQuadrant(225) == 3);
static_assert(AngleToQuadrant(270) == 2);
static_assert(AngleToQuadrant(315) == 1);
static_assert(AngleToQuadrant(350) == 0);
static_assert(AngleToQuadrant(360) == 0);
static_assert(AngleToQuadrant(341341) == 7);

}
