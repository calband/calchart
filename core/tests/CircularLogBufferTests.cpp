/*
 * CircularLogBufferTests.cpp
 * Unit tests for CircularLogBuffer
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
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

using CalChart::CircularLogBuffer;
using CalChart::LogMessage;

TEST_CASE("CircularLogBuffer: Add and retrieve messages", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    buffer.AddMessage("Info", "Test message 1");
    buffer.AddMessage("Warning", "Test message 2");
    buffer.AddMessage("Error", "Test message 3");

    auto messages = buffer.GetMessages();
    REQUIRE(messages.size() == 3);
    CHECK(messages[0].level == "Info");
    CHECK(messages[0].message == "Test message 1");
    CHECK(messages[1].level == "Warning");
    CHECK(messages[1].message == "Test message 2");
    CHECK(messages[2].level == "Error");
    CHECK(messages[2].message == "Test message 3");
}

TEST_CASE("CircularLogBuffer: Get message count", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    REQUIRE(buffer.GetMessages().size() == 0);

    buffer.AddMessage("Info", "Message");
    CHECK(buffer.GetMessages().size() == 1);

    buffer.AddMessage("Info", "Message");
    CHECK(buffer.GetMessages().size() == 2);
}

TEST_CASE("CircularLogBuffer: Circular wrapping", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    // Add more messages than capacity
    for (int i = 0; i < 10; ++i) {
        buffer.AddMessage("Info", "Message " + std::to_string(i));
    }

    // Should only have the last 5 messages
    auto messages = buffer.GetMessages();
    REQUIRE(messages.size() == 5);

    // Should have messages 5-9 (oldest to newest)
    CHECK(messages[0].message == "Message 5");
    CHECK(messages[1].message == "Message 6");
    CHECK(messages[2].message == "Message 7");
    CHECK(messages[3].message == "Message 8");
    CHECK(messages[4].message == "Message 9");
}

TEST_CASE("CircularLogBuffer: Formatted output", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    buffer.AddMessage("Info", "First message");
    buffer.AddMessage("Warning", "Second message");

    auto formatted = CalChart::FormatLogMessages(buffer.GetMessages());
    CHECK(formatted.find("First message") != std::string::npos);
    CHECK(formatted.find("Second message") != std::string::npos);
    CHECK(formatted.find("[Info]") != std::string::npos);
    CHECK(formatted.find("[Warning]") != std::string::npos);
    CHECK(formatted.find("```") != std::string::npos);
}

TEST_CASE("CircularLogBuffer: Empty buffer formatting", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    auto formatted = CalChart::FormatLogMessages(buffer.GetMessages());
    CHECK(formatted.find("No log messages") != std::string::npos);
}

TEST_CASE("CircularLogBuffer: Clear", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    buffer.AddMessage("Info", "Message 1");
    buffer.AddMessage("Info", "Message 2");
    REQUIRE(buffer.GetMessages().size() == 2);

    buffer.Clear();
    CHECK(buffer.GetMessages().size() == 0);

    auto messages = buffer.GetMessages();
    CHECK(messages.size() == 0);
}

TEST_CASE("CircularLogBuffer: Log message timestamps", "[CircularLogBuffer]")
{
    CircularLogBuffer buffer(5);

    buffer.AddMessage("Info", "Message");
    auto messages = buffer.GetMessages();

    REQUIRE(messages.size() == 1);
    CHECK(!messages[0].timestamp.empty());
    // Check timestamp looks like ISO 8601: contains T and Z
    CHECK(messages[0].timestamp.find('T') != std::string::npos);
    CHECK(messages[0].timestamp.find('Z') != std::string::npos);
}
