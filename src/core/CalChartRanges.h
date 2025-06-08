#pragma once
/*
 * CalChartRanges.h
 * Utilities for ranges
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

#include <algorithm>
#include <cstddef>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace CalChart::Ranges {

template <typename T, std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, T>
inline auto ToVector(Range&& range) -> std::vector<T>
{
    auto result = std::vector<T>{};
    std::ranges::copy(range, std::back_inserter(result));
    return result;
}

}

namespace CalChart::Ranges::details {

// Basic zip_view implementation
template <typename... Ranges>
    requires(std::ranges::viewable_range<Ranges> && ...)
class BasicZipView {
public:
    explicit BasicZipView(Ranges&&... ranges)
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
        auto operator++() -> iterator&
        {
            std::apply([](auto&... iters) { (++std::get<0>(iters), ...); }, iters_);
            // if any of the iterators matches the end iterators, we are at the end, so bump them all to the end.
            if (std::apply([](auto&... iters) { return ((std::get<0>(iters) == std::get<1>(iters)) || ...); }, iters_)) {
                std::apply([](auto&... iters) { ((std::get<0>(iters) = std::get<1>(iters)), ...); }, iters_);
            }
            return *this;
        }

        auto operator++(int) -> iterator
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
        auto operator==(const iterator& other) const
        {
            return iters_ == other.iters_;
        }

        auto operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        std::tuple<typename std::pair<std::ranges::iterator_t<Ranges>, std::ranges::iterator_t<Ranges>>...> iters_;
    };

    auto begin() -> iterator
    {
        return std::apply([](auto&&... r) {
            return iterator(std::pair{ std::ranges::begin(r), std::ranges::end(r) }...);
        },
            ranges_);
    }

    auto end() -> iterator
    {
        return std::apply([](auto&&... r) {
            return iterator(std::pair{ std::ranges::end(r), std::ranges::end(r) }...);
        },
            ranges_);
    }

private:
    std::tuple<Ranges...> ranges_;
};

// Basic enumerate_view implementation
template <typename Range>
    requires(std::ranges::viewable_range<Range>)
class BasicEnumerateView {
public:
    explicit BasicEnumerateView(Range&& range)
        : range_{ std::forward<Range>(range) }
    {
    }

    // Iterator type
    class iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::tuple<difference_type, typename std::ranges::range_value_t<Range>>;
        explicit iterator(std::ranges::iterator_t<Range> iter)
            : iter_{ iter }
        {
        }
        explicit iterator()
            : iter_{}
        {
        }

        // Increment operators
        auto operator++() -> iterator&
        {
            ++value;
            ++iter_;
            return *this;
        }

        auto operator++(int) -> iterator
        {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // Dereference operator
        auto operator*() const
        {
            return std::tuple{ value, *iter_ };
        }

        // Equality comparison operator
        auto operator==(const iterator& other) const
        {
            return iter_ == other.iter_;
        }

        auto operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        difference_type value{};
        std::ranges::iterator_t<Range> iter_;
    };

    auto begin() -> iterator
    {
        return iterator(std::ranges::begin(range_));
    }

    auto end() -> iterator
    {
        return iterator(std::ranges::end(range_));
    }

private:
    Range range_;
};

template <std::size_t I, class T>
struct ReflectT {
    using type = T;
};

// Helper template to create a tuple of N objects of type T
template <typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
struct TupleOfN;

// Specialization for creating a tuple of N objects of type T
template <typename T, std::size_t N, std::size_t... Indices>
struct TupleOfN<T, N, std::index_sequence<Indices...>> {
    using type = std::tuple<typename ReflectT<Indices, T>::type...>;
};

// Convenience alias
template <typename T, std::size_t N>
using TupleOf = typename TupleOfN<T, N>::type;

// Helper function to shift tuple elements
template <typename Tuple, std::size_t... Indices>
auto rotateLeftImpl(const Tuple& t, std::index_sequence<Indices...>)
{
    // Create a new tuple by selecting elements from the next index onward, with a default value for the last position
    return std::tuple<std::tuple_element_t<Indices + 1, Tuple>..., std::tuple_element_t<0, Tuple>>{
        std::get<Indices + 1>(t)..., std::get<0>(t)
    };
}

// Wrapper function to shift a tuple left by one position
template <typename... Ts>
auto rotateLeft(const std::tuple<Ts...>& t)
{
    return rotateLeftImpl(t, std::make_index_sequence<sizeof...(Ts) - 1>{});
}

// Helper function to transform each element in the tuple
template <typename Func, typename Tuple, std::size_t... Indices>
auto transformTupleImpl(const Tuple& t, Func func, std::index_sequence<Indices...>)
{
    return std::make_tuple(func(std::get<Indices>(t))...);
}

// Wrapper function to apply transformation function to a tuple
template <typename Func, typename... Ts>
auto transformTuple(const std::tuple<Ts...>& t, Func func)
{
    return transformTupleImpl(t, func, std::index_sequence_for<Ts...>{});
}

// Helper function to create a tuple of iterators with increments
template <typename Iterator, std::size_t... Indices>
auto makeIteratorTupleImpl(Iterator iter, std::index_sequence<Indices...>)
{
    return std::make_tuple((std::next(iter, Indices))...);
}

// Main function to generate a tuple of iterators incremented by each index
template <std::size_t N, typename Iterator>
auto makeIteratorTuple(Iterator iter)
{
    return makeIteratorTupleImpl(iter, std::make_index_sequence<N>{});
}

// Basic enumerate_view implementation
template <std::size_t N, typename Range>
    requires(std::ranges::viewable_range<Range>)
class BasicAdjacentView {
public:
    explicit BasicAdjacentView(Range&& range)
        : range_{ std::forward<Range>(range) }
    {
    }

    // Iterator type
    class iterator {
    public:
        using difference_type = std::ptrdiff_t;
        using value_type = TupleOf<typename std::ranges::range_value_t<Range>, N>;
        explicit iterator(std::optional<TupleOf<std::ranges::iterator_t<Range>, N>> iters, std::ranges::iterator_t<Range> last)
            : iter_{ iters }
            , last_{ last }
        {
        }
        explicit iterator(bool end)
            : endSentinel{ end }
        {
        }
        iterator() = default;

        // Increment operators
        auto operator++() -> iterator&
        {
            if (iter_) {
                std::get<0>(*iter_) = std::get<N - 1>(*iter_);
                ++(std::get<0>(*iter_));
                *iter_ = rotateLeft(*iter_);
            }
            return *this;
        }

        auto operator++(int) -> iterator
        {
            iterator tmp(*this);
            ++(*this);
            return tmp;
        }

        // Dereference operator
        auto operator*() const -> value_type
        {
            return transformTuple(*iter_, [](auto&& iter) { return *iter; });
        }

        // Equality comparison operator
        auto operator==(const iterator& other) const
        {
            if (endSentinel && other.endSentinel) {
                return true;
            }
            if (other.endSentinel) {
                return std::get<N - 1>(*iter_) == last_;
            }
            if (endSentinel) {
                return std::get<N - 1>(*other.iter_) == other.last_;
            }
            return iter_ == other.iter_;
        }

        auto operator!=(const iterator& other) const
        {
            return !(*this == other);
        }

    private:
        std::optional<TupleOf<std::ranges::iterator_t<Range>, N>> iter_{};
        std::ranges::iterator_t<Range> last_{};
        bool endSentinel{};
    };

    auto begin() -> iterator
    {
        auto size = std::distance(std::ranges::begin(range_), std::ranges::end(range_));
        if (static_cast<size_t>(size) < N) {
            return iterator{ true };
        }
        return iterator(makeIteratorTuple<N>(std::ranges::begin(range_)), std::ranges::end(range_));
    }

    auto end() -> iterator
    {
        return iterator{ true };
    }

private:
    Range range_;
};

}

namespace CalChart::Ranges {

template <typename... Ranges>
auto zip_view(Ranges&&... ranges)
{
    return details::BasicZipView<Ranges...>(std::forward<Ranges>(ranges)...);
}

template <typename Range>
auto enumerate_view(Range&& range)
{
    return details::BasicEnumerateView<Range>(std::forward<Range>(range));
}

template <std::size_t N, typename Range>
auto adjacent_view(Range&& range)
{
    return details::BasicAdjacentView<N, Range>(std::forward<Range>(range));
}

}
