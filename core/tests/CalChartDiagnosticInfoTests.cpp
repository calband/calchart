/*
 * CalChartDiagnosticInfoTests.cpp
 * Unit tests for CalChartDiagnosticInfo module
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

#include "CalChartDiagnosticInfo.hpp"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

TEST_CASE("CalChartDiagnosticInfo::Create", "DiagnosticInfo")
{
    auto info = CalChart::DiagnosticInfo::Create();

    // Check that basic fields are populated
    CHECK_FALSE(info.calchart_version.empty());
    CHECK_FALSE(info.build_type.empty());
    CHECK_FALSE(info.compiler_info.empty());
    CHECK_FALSE(info.build_date.empty());

    // Check that build_type is one of the expected values
    CHECK((info.build_type == "Debug" || info.build_type == "Release"));

    // Check that compiler_info contains expected compiler names
    CHECK((info.compiler_info.find("Clang") != std::string::npos || info.compiler_info.find("GCC") != std::string::npos || info.compiler_info.find("MSVC") != std::string::npos || info.compiler_info.find("Unknown") != std::string::npos));

    // Check that show_info starts with no show
    CHECK_FALSE(info.show_info.has_show);
    CHECK(info.show_info.num_sheets == 0);
    CHECK(info.show_info.num_marchers == 0);
}

TEST_CASE("CalChartDiagnosticInfo::toJSON", "DiagnosticInfo")
{
    auto info = CalChart::DiagnosticInfo::Create();

    auto json = info.toJSON();

    // Verify all required fields are in the JSON
    CHECK(json.contains("calchart_version"));
    CHECK(json.contains("build_type"));
    CHECK(json.contains("compiler_info"));
    CHECK(json.contains("build_date"));
    CHECK(json.contains("show_info"));

    // Verify the JSON can be converted to string
    auto json_str = json.dump();
    CHECK_FALSE(json_str.empty());
}

TEST_CASE("CalChartDiagnosticInfo::toString", "DiagnosticInfo")
{
    auto info = CalChart::DiagnosticInfo::Create();

    auto str = info.toString();

    // Verify the string is not empty
    CHECK_FALSE(str.empty());

    // Verify it contains expected markdown sections
    CHECK(str.find("## CalChart Information") != std::string::npos);
    CHECK(str.find("## Show Information") != std::string::npos);

    // Verify it contains the version (from Create)
    CHECK(str.find(info.calchart_version) != std::string::npos);
    CHECK(str.find(info.build_type) != std::string::npos);
}

TEST_CASE("CalChartDiagnosticInfo::AdditionalInfo", "DiagnosticInfo")
{
    auto info = CalChart::DiagnosticInfo::Create();

    // Add some additional info
    info.additional_info["test_key"] = "test_value";
    info.additional_info["another_key"] = "another_value";

    // Check it's stored correctly
    CHECK(info.additional_info.count("test_key") == 1);
    CHECK(info.additional_info.at("test_key") == "test_value");

    // Check toString includes additional info
    auto str = info.toString();
    CHECK(str.find("## Additional Information") != std::string::npos);
    CHECK(str.find("test_key") != std::string::npos);
    CHECK(str.find("test_value") != std::string::npos);

    // Check JSON includes additional info
    auto json = info.toJSON();
    CHECK(json.contains("additional_info"));
    auto additional = json.at("additional_info");
    CHECK(additional.contains("test_key"));
    CHECK(additional.at("test_key") == "test_value");
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
