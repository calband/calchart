/*
 * CircularLogBuffer.cpp
 * Circular buffer for capturing the last N log messages
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

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

#include "CircularLogBuffer.hpp"
#include <chrono>
#include <format>

namespace {
// Get current time as ISO 8601 string: YYYY-MM-DDTHH:MM:SS
std::string GetCurrentTimestamp()
{
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    auto tm = *std::gmtime(&time);
    return std::format("{:04d}-{:02d}-{:02d}T{:02d}:{:02d}:{:02d}Z",
        tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
}
} // namespace

namespace CalChart {

CircularLogBuffer::CircularLogBuffer(size_t capacity)
    : capacity_(capacity)
    , current_index_(0)
    , is_full_(false)
{
    buffer_.reserve(capacity);
}

CircularLogBuffer::CircularLogBuffer(CircularLogBuffer const& other)
{
    std::lock_guard<std::mutex> lock_other(other.lock_);
    buffer_ = other.buffer_;
    capacity_ = other.capacity_;
    current_index_ = other.current_index_;
    is_full_ = other.is_full_;
}

auto CircularLogBuffer::operator=(CircularLogBuffer const& other) -> CircularLogBuffer&
{
    if (this != &other) {
        std::unique_lock<std::mutex> lock_this(lock_, std::defer_lock);
        std::unique_lock<std::mutex> lock_other(other.lock_, std::defer_lock);
        std::lock(lock_this, lock_other);

        buffer_ = other.buffer_;
        capacity_ = other.capacity_;
        current_index_ = other.current_index_;
        is_full_ = other.is_full_;
    }
    return *this;
}

void CircularLogBuffer::AddMessage(std::string level, std::string message)
{
    auto msg = LogMessage{ GetCurrentTimestamp(), std::move(level), std::move(message) };

    std::lock_guard<std::mutex> lock(lock_);

    if (buffer_.size() < capacity_) {
        // Buffer not full yet, just append
        buffer_.push_back(std::move(msg));
    } else {
        // Buffer is full, overwrite oldest message
        buffer_[current_index_] = std::move(msg);
        is_full_ = true;
    }

    // Move index for next insertion
    current_index_ = (current_index_ + 1) % capacity_;
}

auto CircularLogBuffer::GetMessages() const -> std::vector<LogMessage>
{
    std::lock_guard<std::mutex> lock(lock_);

    std::vector<LogMessage> result;
    result.reserve(buffer_.size());

    if (!is_full_) {
        // Buffer not full yet, return in order
        result = buffer_;
    } else {
        // Buffer is full, return starting from current_index_ in chronological order
        std::copy(buffer_.begin() + current_index_, buffer_.end(), std::back_inserter(result));
        std::copy(buffer_.begin(), buffer_.begin() + current_index_, std::back_inserter(result));
    }

    return result;
}

void CircularLogBuffer::Clear()
{
    std::lock_guard<std::mutex> lock(lock_);
    buffer_.clear();
    current_index_ = 0;
    is_full_ = false;
}

auto FormatLogMessages(const std::vector<LogMessage>& messages) -> std::string
{
    if (messages.empty()) {
        return "No log messages captured.";
    }

    auto result = std::string{ "```\n" };
    for (const auto& msg : messages) {
        result += std::format("[{}] [{}] {}\n", msg.timestamp, msg.level, msg.message);
    }
    result += "```";
    return result;
}
} // namespace CalChart
