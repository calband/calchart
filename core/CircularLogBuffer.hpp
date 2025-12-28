#pragma once
/*
 * CircularLogBuffer.hpp
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

#include <mutex>
#include <string>
#include <vector>

namespace CalChart {

// Represents a single log message with metadata
struct LogMessage {
    std::string timestamp; // ISO 8601 format: YYYY-MM-DDTHH:MM:SS
    std::string level; // "Error", "Warning", "Info", "Debug", "Trace"
    std::string message; // The actual log message

    LogMessage() = default;
    LogMessage(std::string ts, std::string lvl, std::string msg)
        : timestamp(std::move(ts))
        , level(std::move(lvl))
        , message(std::move(msg))
    {
    }
};

// Thread-safe circular buffer for storing recent log messages
class CircularLogBuffer {
public:
    explicit CircularLogBuffer(size_t capacity = 100);

    CircularLogBuffer(const CircularLogBuffer&);
    CircularLogBuffer& operator=(const CircularLogBuffer&);

    // Add a message to the buffer
    // Thread-safe
    void AddMessage(std::string level, std::string message);

    // Get all messages in chronological order
    // Thread-safe
    [[nodiscard]] auto GetMessages() const -> std::vector<LogMessage>;

    void Clear();

private:
    std::vector<LogMessage> buffer_;
    size_t capacity_;
    size_t current_index_; // Next position to write to
    bool is_full_; // Whether we've wrapped around
    mutable std::mutex lock_;
};

auto FormatLogMessages(const std::vector<LogMessage>& messages) -> std::string;

} // namespace CalChart
