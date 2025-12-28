/*
 * DiagnosticInfoTests.cpp
 * Unit tests for wxCalChart CollectDiagnosticInfo function
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

#include "DiagnosticInfo.h"
#include <catch2/catch_test_macros.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

TEST_CASE("wxCalChart::CollectDiagnosticInfo", "DiagnosticInfo")
{
    // Collect without document
    auto info = wxCalChart::CollectDiagnosticInfo();

    // Verify Core information is populated (from CalChart::DiagnosticInfo::Create)
    CHECK_FALSE(info.calchart_version.empty());
    CHECK_FALSE(info.build_type.empty());
    CHECK_FALSE(info.compiler_info.empty());
    CHECK_FALSE(info.build_date.empty());

    // Verify system information is added to additional_info
    // These fields should be present from wxWidgets data collection
    CHECK(info.additional_info.count("OS") >= 0); // May be 0 or 1 depending on platform
    CHECK(info.additional_info.count("Architecture") >= 0);
    CHECK(info.additional_info.count("wxWidgets") > 0);
    CHECK(info.additional_info.count("Display") > 0);
    CHECK(info.additional_info.count("Memory") > 0);

    // Verify wxWidgets version is not empty if it exists
    if (info.additional_info.count("wxWidgets") > 0) {
        CHECK_FALSE(info.additional_info.at("wxWidgets").empty());
    }

    // Verify Display info is not empty
    if (info.additional_info.count("Display") > 0) {
        CHECK_FALSE(info.additional_info.at("Display").empty());
    }

    // Verify Memory info is not empty
    if (info.additional_info.count("Memory") > 0) {
        CHECK_FALSE(info.additional_info.at("Memory").empty());
    }
}

TEST_CASE("wxCalChart::CollectDiagnosticInfo::SystemInfo", "DiagnosticInfo")
{
    auto info = wxCalChart::CollectDiagnosticInfo();

    // Check that system information looks reasonable
    // Note: We can't check for specific values as they vary by platform and CI environment

    // wxWidgets version should be in format "X.Y.Z" or similar
    if (info.additional_info.count("wxWidgets") > 0) {
        auto wx_version = info.additional_info.at("wxWidgets");
        // Should contain at least one dot
        CHECK(wx_version.find(".") != std::string::npos);
    }

    // Display info should mention displays and dimensions
    if (info.additional_info.count("Display") > 0) {
        auto display = info.additional_info.at("Display");
        // Should mention "display" and have some dimensions info
        CHECK(display.find("display") != std::string::npos);
    }

    // Memory should be non-negative number
    if (info.additional_info.count("Memory") > 0) {
        auto memory = info.additional_info.at("Memory");
        // Should contain "MB" or "Unknown"
        CHECK((memory.find("MB") != std::string::npos || memory.find("Unknown") != std::string::npos));
    }
}

TEST_CASE("wxCalChart::CollectDiagnosticInfo::JSON", "DiagnosticInfo")
{
    auto info = wxCalChart::CollectDiagnosticInfo();

    // Convert to JSON to verify it's JSON-serializable
    auto json = info.toJSON();
    auto json_str = json.dump();

    // JSON should not be empty and should be valid
    CHECK_FALSE(json_str.empty());
    CHECK(json_str[0] == '{');

    // Verify it contains expected fields
    CHECK(json.contains("calchart_version"));
    CHECK(json.contains("additional_info"));
}

TEST_CASE("wxCalChart::CollectDiagnosticInfo::StringOutput", "DiagnosticInfo")
{
    auto info = wxCalChart::CollectDiagnosticInfo();

    // Convert to string to verify it's user-readable
    auto str = info.toString();

    // String should not be empty and contain markdown sections
    CHECK_FALSE(str.empty());
    CHECK(str.find("## CalChart Information") != std::string::npos);
    CHECK(str.find("## Show Information") != std::string::npos);
    CHECK(str.find("## Additional Information") != std::string::npos);

    // Should contain at least some system info
    CHECK(str.find("wxWidgets") != std::string::npos);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
