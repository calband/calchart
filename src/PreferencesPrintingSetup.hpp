#pragma once
/*
 * PreferencesPrintingSetup.h
 * Dialox box for PrintContinuity Setup part of preferences
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

#include "PreferencesUtils.h"

#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class FancyTextWin;
class PrintingPreview;
namespace CalChart {
class Configuration;
}

class PrintingSetup : public PreferencePage {
    using super = PreferencePage;
    DECLARE_CLASS(PrintingSetup)
    DECLARE_EVENT_TABLE()

public:
    static PrintingSetup* CreatePreference(CalChart::Configuration& config, wxWindow* parent)
    {
        auto pref = new PrintingSetup(config, parent);
        pref->Initialize();
        return pref;
    }

private:
    // private, use the CreatePreference method
    PrintingSetup(CalChart::Configuration& config, wxWindow* parent)
        : super(config, parent, "Print Continuity")
    {
    }

public:
    ~PrintingSetup() override = default;

    // use these to get and set default values
    auto TransferDataToWindow() -> bool override;
    auto TransferDataFromWindow() -> bool override;
    auto ClearValuesToDefault() -> bool override;

private:
    wxUI::Generic<FancyTextWin>::Proxy mUserInput{};
    wxUI::Generic<PrintingPreview>::Proxy mPrintingDisplay{};
    wxUI::CheckBox::Proxy mLandscape{};
    wxUI::TextCtrl::Proxy mDotRatio{};
    wxUI::TextCtrl::Proxy mPLineRatio{};
    wxUI::TextCtrl::Proxy mSLineRatio{};
    wxUI::SpinCtrl::Proxy mLinePad{};
    wxUI::SpinCtrl::Proxy mMaxFontSize{};

    void InitFromConfig() override;
    void CreateControls() override;
    void OnKeyPress(wxCommandEvent&);
    void OnCmdTextChanged(wxCommandEvent&);
};
