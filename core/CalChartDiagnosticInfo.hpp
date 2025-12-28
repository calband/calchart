#pragma once
/*
 * CalChartDiagnosticInfo.h
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

#include "CircularLogBuffer.hpp"
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace CalChart {

// Forward declarations
class Show;
class Configuration;

// Platform-agnostic diagnostic information structure
// This contains information that can be collected without any UI framework dependencies
struct DiagnosticInfo {
    // CalChart application information
    std::string calchart_version;
    std::string build_type; // Debug, Release, RelWithDebInfo, etc.
    std::string compiler_info;
    std::string build_date;

    // Show information (if available)
    struct ShowInfo {
        bool has_show = false;
        int num_sheets = 0;
        int num_marchers = 0;
        std::string show_mode;
        std::string file_format_version;
    } show_info;

    // Recent log messages
    std::vector<LogMessage> recent_logs;

    // Generic key-value pairs for additional info
    std::map<std::string, std::string> additional_info;

    // Serialize to JSON
    [[nodiscard]] auto toJSON() const -> nlohmann::json;

    // Serialize to formatted string (markdown-friendly)
    [[nodiscard]] auto toString() const -> std::string;

    // Create diagnostic info with CalChart-specific information
    static auto Create() -> DiagnosticInfo;

    // Add show information if available
    void AddShowInfo(Show const& show);
};

} // namespace CalChart
