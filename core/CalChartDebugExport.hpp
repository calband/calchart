#pragma once
/*
 * CalChartDebugExport.hpp
 * Export debug information for troubleshooting
 */

/*
   Copyright (C) 1995-2026  Garrick Brian Meeker, Richard Michael Powell

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

#include <nlohmann/json.hpp>
#include <string>

namespace CalChart {

class Show;
class Animation;

// Platform-specific display information (to be filled by UI layer)
struct DisplayInfo {
    double dpi_scale = 1.0;
    int screen_width = 0;
    int screen_height = 0;
    std::string os_name;
    std::string os_version;

    [[nodiscard]] auto toJSON() const -> nlohmann::json;
};

// Debug export data structure
struct DebugExportData {
    // Version of the debug export format
    static constexpr const char* FORMAT_VERSION = "1.0";

    // Timestamp of export
    std::string timestamp;

    // CalChart version and build info
    std::string calchart_version;
    std::string build_type;
    std::string compiler_info;

    // Platform information
    DisplayInfo display_info;

    // Show information (basic)
    struct ShowSummary {
        size_t num_sheets = 0;
        size_t num_marchers = 0;
        std::string show_mode;
    } show_summary;

    // Animation data (detailed marcher info for each sheet, beat, and marcher)
    // Structure: sheets[sheet_idx].marchers[marcher_idx].beats[beat_idx] = {position, facing, step_style}
    nlohmann::json animation_data;

    [[nodiscard]] auto toJSON() const -> nlohmann::json;
    [[nodiscard]] auto toString() const -> std::string;

    // Compress the debug data using gzip
    [[nodiscard]] auto toCompressedBytes() const -> std::vector<unsigned char>;

    // Create debug export data from show and animation
    static auto Create(Show const& show, Animation const& animation, DisplayInfo const& displayInfo = {}) -> DebugExportData;
};

} // namespace CalChart
