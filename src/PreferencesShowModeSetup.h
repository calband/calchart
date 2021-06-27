#pragma once
/*
 * PreferencesShowModeSetup.h
 * Dialox box for ShowMode Setup part of preferences
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "PreferencesUtils.h"

#include "CalChartConfiguration.h"
#include "CalChartShowMode.h"
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

const int zoom_amounts[] = { 500, 200, 150, 100, 75, 50, 25, 10 };
constexpr auto defaultZoom = 3;

class ShowModeSetup : public PreferencePage {
    using super = PreferencePage;
    DECLARE_CLASS(ShowModeSetup)

public:
    static ShowModeSetup* CreatePreference(CalChartConfiguration& config, wxWindow* parent)
    {
        auto pref = new ShowModeSetup(config, parent);
        pref->Initialize();
        return pref;
    }

private:
    // private, use the CreatePreference method
    ShowModeSetup(CalChartConfiguration& config, wxWindow* parent)
        : super(config, parent, "Setup Modes")
    {
    }
public:
    ~ShowModeSetup() override = default;

    // use these to get and set default values
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    bool ClearValuesToDefault() override;

private:
    void InitFromConfig() override;
    void CreateControls() override;

    void OnCmdLineText(wxCommandEvent&);
    void OnCmdChoice();
    CalChartConfiguration::ShowModeInfo_t mShowModeValues[SHOWMODE_NUM];
    CalChart::ShowMode::YardLinesInfo_t mYardText;
    int mWhichMode{};
    int mWhichYardLine{};
};
