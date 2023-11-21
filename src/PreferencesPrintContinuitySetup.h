#pragma once
/*
 * PreferencesPrintContinuitySetup.h
 * Dialox box for PrintContinuity Setup part of preferences
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
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>
#include <wxUI/wxUI.h>

class FancyTextWin;
class PrintContinuityPreview;

class PrintContinuitySetup : public PreferencePage {
    using super = PreferencePage;
    DECLARE_CLASS(PrintContinuitySetup)
    DECLARE_EVENT_TABLE()

public:
    static PrintContinuitySetup* CreatePreference(CalChartConfiguration& config, wxWindow* parent)
    {
        auto pref = new PrintContinuitySetup(config, parent);
        pref->Initialize();
        return pref;
    }

private:
    // private, use the CreatePreference method
    PrintContinuitySetup(CalChartConfiguration& config, wxWindow* parent)
        : super(config, parent, "Print Continuity")
    {
    }

public:
    ~PrintContinuitySetup() override = default;

    // use these to get and set default values
    auto TransferDataToWindow() -> bool override;
    auto TransferDataFromWindow() -> bool override;
    auto ClearValuesToDefault() -> bool override;

private:
    wxUI::Generic<FancyTextWin>::Proxy mUserInput{};
    wxUI::Generic<PrintContinuityPreview>::Proxy mPrintContDisplay{};
    wxUI::CheckBox::Proxy mLandscape{};

    void InitFromConfig() override;
    void CreateControls() override;
    void OnKeyPress(wxCommandEvent&);
};
