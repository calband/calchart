#pragma once
/*
 * CalChartUtils.h
 * General Utilities
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

#include <cmath>
#include <ranges>
#include <vector>

namespace CalChart {

/**
 * Math Utilities
 */

// Should be constexpr in c++23
template <typename T>
inline auto IS_ZERO(T a)
{
    // the fudge factor for floating point math
    constexpr auto kEpsilon = 1e-5;
    return std::abs(a) < kEpsilon;
}

template <typename T, typename U>
inline auto IS_EQUAL(T a, U b) { return IS_ZERO(b - a); }

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

public:
    Iterator() = default;

    // Iterator type
    class iterator {
        using val_t = typename std::underlying_type<C>::type;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = C;
        iterator() = default;
        explicit iterator(val_t value)
            : val_{ value }
        {
        }

        // Increment operators
        auto operator++() -> iterator&
        {
            ++val_;
            return *this;
        }

        auto operator++(int) -> iterator
        {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // Dereference operator
        auto operator*() const -> C
        {
            return static_cast<C>(val_);
        }

        // Equality comparison operator
        auto operator==(const iterator& other) const
        {
            return val_ == other.val_;
        }

        auto operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        val_t val_{};
    };

    auto begin() const -> iterator
    {
        return iterator(toUType(beginVal));
    }

    auto end() const -> iterator
    {
        return iterator(toUType(endVal) + 1);
    }
};

template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename T>
auto append(std::vector<T>& v, T const& other) -> std::vector<T>&
{
    v.insert(v.end(), other);
    return v;
}

template <typename T>
auto append(std::vector<T>& v, std::vector<T> const& other) -> std::vector<T>&
{
    v.insert(v.end(), other.begin(), other.end());
    return v;
}

template <typename T>
auto append(std::vector<T>&& v, std::vector<T> const& other) -> std::vector<T>&&
{
    v.insert(v.end(), other.begin(), other.end());
    return std::forward<std::vector<T>>(v);
}

// This actually does the work
template <typename T, std::ranges::range R>
    requires(std::convertible_to<typename std::ranges::range_value_t<R>, T>)
auto append(std::vector<T>& v, R&& range) -> std::vector<T>&
{
    v.insert(v.end(), range.begin(), range.end());
    return v;
}

}
