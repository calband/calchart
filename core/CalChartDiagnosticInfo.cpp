/*
 * CalChartDiagnosticInfo.cpp
 * Platform-agnostic diagnostic information collection for bug reporting
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
#include "CalChartShow.h"
#include "CalChartShowMode.h"
#include "ccvers.h"
#include <format>

namespace CalChart {

auto DiagnosticInfo::toJSON() const -> nlohmann::json
{
    auto j = nlohmann::json{};

    // CalChart information
    j["calchart_version"] = calchart_version;
    j["build_type"] = build_type;
    j["compiler_info"] = compiler_info;
    j["build_date"] = build_date;

    // Show information
    if (show_info.has_show) {
        auto show_json = nlohmann::json{};
        show_json["num_sheets"] = show_info.num_sheets;
        show_json["num_marchers"] = show_info.num_marchers;
        show_json["show_mode"] = show_info.show_mode;
        show_json["file_format_version"] = show_info.file_format_version;
        j["show_info"] = show_json;
    } else {
        j["show_info"] = "No show loaded";
    }

    // Additional info
    if (!additional_info.empty()) {
        j["additional_info"] = additional_info;
    }

    return j;
}

auto DiagnosticInfo::toString() const -> std::string
{
    auto result = std::format(
        "## CalChart Information\n"
        "- **Version**: {}\n"
        "- **Build Type**: {}\n"
        "- **Compiler**: {}\n"
        "- **Build Date**: {}\n"
        "\n",
        calchart_version, build_type, compiler_info, build_date);

    result += "## Show Information\n";
    if (show_info.has_show) {
        result += std::format(
            "- **Sheets**: {}\n"
            "- **Marchers**: {}\n"
            "- **Mode**: {}\n"
            "- **Format Version**: {}\n",
            show_info.num_sheets, show_info.num_marchers,
            show_info.show_mode, show_info.file_format_version);
    } else {
        result += "- No show currently loaded\n";
    }
    result += "\n";

    if (!additional_info.empty()) {
        result += "## Additional Information\n";
        for (auto const& [key, value] : additional_info) {
            result += std::format("- **{}**: {}\n", key, value);
        }
        result += "\n";
    }

    return result;
}

auto DiagnosticInfo::Create() -> DiagnosticInfo
{
    auto info = DiagnosticInfo{};

    // CalChart version from ccvers.h
    info.calchart_version = CC_VERSION;

    // Build type
#ifdef NDEBUG
    info.build_type = "Release";
#else
    info.build_type = "Debug";
#endif

    // Compiler information
#if defined(__clang__)
    info.compiler_info = "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__);
#elif defined(__GNUC__)
    info.compiler_info = "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__);
#elif defined(_MSC_VER)
    info.compiler_info = "MSVC " + std::to_string(_MSC_VER);
#else
    info.compiler_info = "Unknown";
#endif

    // Build date
    info.build_date = std::string(__DATE__) + " " + std::string(__TIME__);

    return info;
}

void DiagnosticInfo::AddShowInfo(Show const& show)
{
    show_info.has_show = true;
    show_info.num_sheets = static_cast<int>(show.GetNumSheets());
    show_info.num_marchers = static_cast<int>(show.GetNumPoints());

    // ShowMode doesn't have a simple name, so we'll describe it with size
    auto const& mode = show.GetShowMode();
    auto size = mode.Size();
    show_info.show_mode = std::to_string(size.x) + "x" + std::to_string(size.y) + " steps";

    // File format version - we can add this if there's a way to get it from Show
    // For now, just indicate it's the current format
    show_info.file_format_version = "Current";
}

} // namespace CalChart
