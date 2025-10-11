#pragma once
/*
 * cc_preferences.h
 * Dialox box for preferences
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "basic_ui.h"
#include <optional>
#include <wx/wx.h>

namespace CalChart {
class Configuration;
}
// the basic class panel we use for all the pages.
// Each page gets a references to the CalChartConfig which will be used for
// getting and setting.
// The idea is that we init the data by reading the values into the pref,
// then we transfer the data to the.
// So we have a CreatePreference function for each, and then what?  does it
// just Call Initialize?  Which sucks the data over from Config and then updates?
// Yes.
class PreferencePage : public wxPanel {
    using super = wxPanel;
    DECLARE_ABSTRACT_CLASS(PreferencePage)
public:
    PreferencePage(CalChart::Configuration& config, wxWindow* parent, const wxString& caption)
        : super(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU, caption)
        , mConfig(config)
    {
    }
    ~PreferencePage() override = default;
    void Initialize()
    {
        CreateControls();
        InitFromConfig();
        GetSizer()->Fit(this);
        GetSizer()->SetSizeHints(this);
        Center();
        TransferDataToWindow();
    }

    // use these to get and set default values
    auto TransferDataToWindow() -> bool override = 0;
    auto TransferDataFromWindow() -> bool override = 0;
    virtual auto ClearValuesToDefault() -> bool = 0;

private:
    virtual void CreateControls() = 0;

protected:
    // force a readread of the config
    virtual void InitFromConfig() = 0;
    CalChart::Configuration& mConfig;
};

inline auto toDouble(wxString const& v) -> std::optional<double>
{
    double value{};
    if (v.ToDouble(&value)) {
        return value;
    }
    return std::nullopt;
}
