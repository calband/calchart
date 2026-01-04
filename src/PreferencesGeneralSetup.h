#pragma once
/*
 * PreferencesGeneralSetup.h
 * Dialox box for General Setup part of preferences
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
#include <wxUI/wxUI.hpp>

/// General setup
/// Handles general configurations parameters of CalChart
///

namespace CalChart {
class Configuration;
}

class GeneralSetup : public PreferencePage {
    using super = PreferencePage;
    DECLARE_CLASS(GeneralSetup)
    DECLARE_EVENT_TABLE()

public:
    static GeneralSetup* CreatePreference(CalChart::Configuration& config, wxWindow* parent)
    {
        auto* pref = new GeneralSetup(config, parent);
        pref->Initialize();
        return pref;
    }

private:
    // private, use the CreatePreference method
    GeneralSetup(CalChart::Configuration& config, wxWindow* parent)
        : super(config, parent, "General Setup")
    {
    }

public:
    ~GeneralSetup() override = default;

    // use these to get and set default values
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    bool ClearValuesToDefault() override;

private:
    void InitFromConfig() override;
    void CreateControls() override;

    void OnCmdResetAll(wxCommandEvent&);

    wxUI::TextCtrl::Proxy mAutoSave_Interval{};
    wxUI::TextCtrl::Proxy mIgnoredUpdateVersion{};
    wxUI::CheckBox::Proxy mBeep_On_Collisions{};
    wxUI::CheckBox::Proxy mSheetSlider{};
    wxUI::CheckBox::Proxy mScroll_Natural{};
    wxUI::CheckBox::Proxy mSetSheet_Undo{};
    wxUI::CheckBox::Proxy mSelection_Undo{};
    wxUI::CheckBox::Proxy mAllowViewer{};
};
