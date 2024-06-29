#pragma once
/*
 * CalChartCoroutine.h
 * Utilities for coroutines
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

#include <coroutine>
#include <exception>
#include <ranges>
#include <stdexcept>

namespace CalChart::Coroutine {

// this is lifted from:
// https://en.cppreference.com/w/cpp/language/coroutines
// This can be replaced when std::generator is available.
// We have also added support for begin, end to allow for range_for
template <typename T>
struct Generator {
    // The class name 'Generator' is our choice and it is not required for coroutine
    // magic. Compiler recognizes coroutine by the presence of 'co_yield' keyword.
    // You can use name 'MyGenerator' (or any other name) instead as long as you include
    // nested struct promise_type with 'MyGenerator get_return_object()' method.

    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type // required
    {
        T value_;
        std::exception_ptr exception_;

        auto get_return_object() -> Generator
        {
            return Generator(handle_type::from_promise(*this));
        }
        auto initial_suspend() -> std::suspend_always { return {}; }
        auto final_suspend() noexcept -> std::suspend_always { return {}; }
        void unhandled_exception() { exception_ = std::current_exception(); } // saving exception

        template <std::convertible_to<T> From> // C++20 concept
        auto yield_value(From&& from) -> std::suspend_always
        {
            value_ = std::forward<From>(from); // caching the result in promise
            return {};
        }
        void return_void() { }
    };

    explicit Generator(handle_type h)
        : h_(h)
    {
    }

    explicit operator bool()
    {
        return hasData();
    }

    auto hasData() -> bool
    {
        fill(); // The only way to reliably find out whether or not we finished coroutine,
                // whether or not there is going to be a next value generated (co_yield)
                // in coroutine via C++ getter (operator () below) is to execute/resume
                // coroutine until the next co_yield point (or let it fall off end).
                // Then we store/cache result in promise to allow getter (operator() below
                // to grab it without executing coroutine).
        return !h_.done();
    }

    auto operator()() -> T
    {
        return data();
    }

    auto data() -> T
    {
        fill();
        full_ = false; // we are going to move out previously cached
                       // result to make promise empty again
        return std::move(h_.promise().value_);
    }

    struct Iterator {
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        auto operator*() const -> T;
        auto operator++() -> Iterator&;
        auto operator++(int) -> Iterator;
        auto operator==(Iterator const&) const -> bool;
        Iterator(Generator* generator = nullptr)
            : generator{ generator }
        {
        }

    private:
        bool atEnd() const;
        Generator* generator{};
    };

    auto begin() -> Iterator;
    auto end() -> Iterator;

private:
    handle_type h_;

    bool full_ = false;

    void fill()
    {
        if (!full_) {
            h_();
            if (h_.promise().exception_) {
                std::rethrow_exception(h_.promise().exception_);
            }
            // propagate coroutine exception in called context

            full_ = true;
        }
    }
};

template <typename T>
auto Generator<T>::Iterator::operator*() const -> T
{
    if (generator == nullptr) {
        throw std::runtime_error("invalid iterator");
    }
    return generator->data();
}

template <typename T>
auto Generator<T>::Iterator::operator++() -> Iterator&
{
    // iterating is a no-op as the read operation effectively iterates forward.
    return *this;
}

template <typename T>
auto Generator<T>::Iterator::operator==(typename Generator<T>::Iterator const& other) const -> bool
{
    return this->atEnd() && other.atEnd();
}

template <typename T>
auto Generator<T>::Iterator::atEnd() const -> bool
{
    if (generator) {
        return !generator->hasData();
    }
    return true;
}

template <typename T>
auto Generator<T>::begin() -> Iterator
{
    return { this };
}

template <typename T>
auto Generator<T>::end() -> Iterator
{
    return { nullptr };
}

static_assert(std::ranges::input_range<Generator<int>>);

}
