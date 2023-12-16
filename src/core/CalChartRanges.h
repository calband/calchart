#pragma once
/*
 * CalChartRanges.h
 * Utilities for ranges
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

#include <algorithm>
#include <ranges>
#include <stdexcept>

namespace CalChart::Ranges {

template <typename T, std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, T>
inline auto ToVector(Range&& range) -> std::vector<T>
{
    auto result = std::vector<T>{};
    std::ranges::copy(range, std::back_inserter(result));
    return result;
}

// Basic zip_view implementation
template <typename... Ranges>
    requires(std::ranges::viewable_range<Ranges> && ...)
class BasicZipView {
public:
    BasicZipView(Ranges&&... ranges)
        : ranges_{ std::forward<Ranges>(ranges)... }
    {
    }

    // Iterator type
    class iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::tuple<typename std::ranges::range_value_t<Ranges>...>;
        explicit iterator(typename std::pair<std::ranges::iterator_t<Ranges>, std::ranges::iterator_t<Ranges>>... iters)
            : iters_{ iters... }
        {
        }
        explicit iterator()
            : iters_{}
        {
        }

        // Increment operators
        iterator& operator++()
        {
            std::apply([](auto&... iters) { (++std::get<0>(iters), ...); }, iters_);
            // if any of the iterators matches the end iterators, we are at the end, so bump them all to the end.
            if (std::apply([](auto&... iters) { return ((std::get<0>(iters) == std::get<1>(iters)) || ...); }, iters_)) {
                std::apply([](auto&... iters) { ((std::get<0>(iters) = std::get<1>(iters)), ...); }, iters_);
            }
            return *this;
        }

        iterator operator++(int)
        {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // Dereference operator
        auto operator*() const
        {
            return std::apply([](auto&&... iters) {
                return std::tuple{ *std::get<0>(iters)... };
            },
                iters_);
        }

        // Equality comparison operator
        bool operator==(const iterator& other) const
        {
            return iters_ == other.iters_;
        }

        bool operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        std::tuple<typename std::pair<std::ranges::iterator_t<Ranges>, std::ranges::iterator_t<Ranges>>...> iters_;
    };

    iterator begin()
    {
        return std::apply([](auto&&... r) {
            return iterator(std::pair{ std::ranges::begin(r), std::ranges::end(r) }...);
        },
            ranges_);
    }

    iterator end()
    {
        return std::apply([](auto&&... r) {
            return iterator(std::pair{ std::ranges::end(r), std::ranges::end(r) }...);
        },
            ranges_);
    }

private:
    std::tuple<Ranges...> ranges_;
};

template <typename... Ranges>
BasicZipView<Ranges...> zip_view(Ranges&&... ranges)
{
    return BasicZipView<Ranges...>(std::forward<Ranges>(ranges)...);
}

}
