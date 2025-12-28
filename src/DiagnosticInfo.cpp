/*
 * DiagnosticInfo.cpp
 * wxWidgets-specific diagnostic information collection for bug reporting
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
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartShow.h"
#include <sstream>
#include <wx/config.h>
#include <wx/display.h>
#include <wx/platinfo.h>
#include <wx/utils.h>

namespace wxCalChart {
namespace {

    // System information collection
    struct SystemInfo {
        std::string os_name;
        std::string os_version;
        std::string architecture;
        std::string wxwidgets_version;
        std::string display_info;
        std::string memory_info;
    };

    // Collect system information using wxWidgets
    auto CollectSystemInfo() -> SystemInfo
    {
        auto info = SystemInfo{};

        // Get platform information
        auto platform = wxPlatformInfo::Get();

        // OS name and version
        info.os_name = platform.GetOperatingSystemIdName();

        auto major = platform.GetOSMajorVersion();
        auto minor = platform.GetOSMinorVersion();
        if (major != -1) {
            info.os_version = std::to_string(major);
            if (minor != -1) {
                info.os_version += "." + std::to_string(minor);
            }
        } else {
            info.os_version = "Unknown";
        }

        // Architecture
        info.architecture = platform.GetBitnessName();

        // wxWidgets version
        info.wxwidgets_version = wxString::Format("%d.%d.%d",
            wxMAJOR_VERSION, wxMINOR_VERSION, wxRELEASE_NUMBER)
                                     .ToStdString();

        // Display information
        auto display_count = wxDisplay::GetCount();
        if (display_count > 0) {
            auto primary_display = wxDisplay(static_cast<unsigned>(0));
            auto geometry = primary_display.GetGeometry();
            auto dpi = primary_display.GetPPI();

            auto ss = std::stringstream{};
            ss << display_count << " display(s), Primary: "
               << geometry.width << "x" << geometry.height
               << " @ " << dpi.x << " DPI";
            info.display_info = ss.str();
        } else {
            info.display_info = "No displays detected";
        }

        // Memory information
        auto memory = wxGetFreeMemory();
        if (memory != -1) {
            auto memory_mb = memory.ToLong() / (1024 * 1024);
            info.memory_info = std::to_string(memory_mb) + " MB free";
        } else {
            info.memory_info = "Unknown";
        }

        return info;
    }

    // Collect configuration information
    auto CollectConfigInfo(wxConfigBase* config) -> std::map<std::string, std::string>
    {
        auto config_info = std::map<std::string, std::string>{};

        // Collect a few key configuration values
        // We don't want to collect everything (privacy), just relevant debugging info

        if (config != nullptr) {
            // Add some basic config values that might be useful
            long long_value;

            if (config->Read("AutosaveInterval", &long_value)) {
                config_info["AutosaveInterval"] = std::to_string(long_value);
            }

            // Add more config values as needed, but be mindful of privacy
        }

        return config_info;
    }

    // Add document state information
    void AddDocumentInfo(CalChart::DiagnosticInfo& info, CalChartDoc const* doc)
    {
        if (doc == nullptr) {
            return;
        }

        // Add show information from document
        // CalChartDoc doesn't expose GetShow() directly, so we'll collect info directly
        info.show_info.has_show = true;
        info.show_info.num_sheets = static_cast<int>(doc->GetNumSheets());
        info.show_info.num_marchers = static_cast<int>(doc->GetNumPoints());
        info.show_info.show_mode = "Custom"; // We'd need GetShowMode() exposed to get this
        info.show_info.file_format_version = "Current";

        // Add additional document state
        info.additional_info["Document Modified"] = doc->IsModified() ? "Yes" : "No";
        info.additional_info["Document Path"] = doc->GetFilename().ToStdString();
    }

} // anonymous namespace

auto CollectDiagnosticInfo(CalChartDoc const* doc, CalChart::CircularLogBuffer const& logs) -> CalChart::DiagnosticInfo
{
    // Start with Core diagnostic info
    auto info = CalChart::DiagnosticInfo::Create();

    // Add system information
    auto sys_info = CollectSystemInfo();
    info.additional_info["OS"] = sys_info.os_name + " " + sys_info.os_version;
    info.additional_info["Architecture"] = sys_info.architecture;
    info.additional_info["wxWidgets"] = sys_info.wxwidgets_version;
    info.additional_info["Display"] = sys_info.display_info;
    info.additional_info["Memory"] = sys_info.memory_info;

    // Add document information if available
    if (doc != nullptr) {
        AddDocumentInfo(info, doc);
    }

    // Add configuration info
    auto config = wxConfigBase::Get(false);
    if (config != nullptr) {
        auto config_info = CollectConfigInfo(config);
        for (auto const& [key, value] : config_info) {
            info.additional_info["Config:" + key] = value;
        }
    }

    info.recent_logs = logs.GetMessages();

    return info;
}

} // namespace wxCalChart
