/*
 * SystemConfiguration.cpp
 * Functions for manipulating configuration Settings
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

#include "SystemConfiguration.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include <wx/config.h>

namespace {

class SystemConfigurationDetails : public CalChart::ConfigurationDetails {
public:
    ~SystemConfigurationDetails() override = default;

private:
    static auto GetAndConfigureConfig() -> wxConfigBase*
    {
        auto* config = wxConfigBase::Get();
        config->SetPath("/CalChart");
        return config;
    }

public:
    [[nodiscard]] auto Read(std::string_view key, CalChart::ConfigurationType const& defaultValue) const -> CalChart::ConfigurationType override
    {
        auto* config = SystemConfigurationDetails::GetAndConfigureConfig();
        auto value = std::visit(
            CalChart::overloaded{
                [config, key = wxString{ key.data() }](CalChart::Color value) -> CalChart::ConfigurationType {
                    wxString string = wxString{};
                    config->Read(key, &string);
                    if (string == wxString{}) {
                        return value;
                    }
                    return CalChart::Color::FromString(std::string{ string });
                },
                // handle wxString specially
                [config, key = wxString{ key.data() }](std::string value) -> CalChart::ConfigurationType {
                    wxString string = wxString{ value };
                    config->Read(key, &string);
                    return std::string{ string };
                },
                [config, key = wxString{ key.data() }](auto value) -> CalChart::ConfigurationType {
                    config->Read(key, &value);
                    return value;
                },
            },
            defaultValue);
        return value;
    }
    void Write(std::string_view key, CalChart::ConfigurationType const& value) override
    {
        auto* config = SystemConfigurationDetails::GetAndConfigureConfig();
        std::visit(
            CalChart::overloaded{
                [config, key = wxString{ key.data() }](CalChart::Color value) {
                    config->Write(key, wxString{ value.ToString() });
                },
                // handle wxString specially
                [config, key = wxString{ key.data() }](std::string value) {
                    config->Write(key, wxString{ value });
                },
                [config, key = wxString{ key.data() }](auto value) {
                    config->Write(key, value);
                },
            },
            value);
        config->Flush();
    }
    void Clear(std::string_view key) override
    {
        auto* config = SystemConfigurationDetails::GetAndConfigureConfig();
        config->DeleteEntry(std::string{ key });
        config->Flush();
    }
};
}

namespace wxCalChart {
auto GetGlobalConfig() -> CalChart::Configuration&
{
    static CalChart::Configuration sconfig(std::make_shared<SystemConfigurationDetails>());
    return sconfig;
}

void AssignConfig(CalChart::Configuration const& config)
{
    // now flush out the config
    GetGlobalConfig().Assign(config);
    GetGlobalConfig().FlushWriteQueue();
}
}