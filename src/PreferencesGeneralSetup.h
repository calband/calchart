#pragma once
/*
 * PreferencesGeneralSetup.h
 * Dialox box for General Setup part of preferences
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

/// General setup
/// Handles general configurations parameters of CalChart
///

class GeneralSetup : public PreferencePage {
    DECLARE_CLASS(GeneralSetup)
    DECLARE_EVENT_TABLE()

public:
    GeneralSetup(CalChartConfiguration& config, wxWindow* parent, wxWindowID id = wxID_ANY, wxString const& caption = wxT("General Setup"), wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    virtual ~GeneralSetup() = default;

    virtual void Init() override;
    virtual void CreateControls() override;

    // use these to get and set default values
    virtual bool TransferDataToWindow() override;
    virtual bool TransferDataFromWindow() override;
    virtual bool ClearValuesToDefault() override;

private:
    void OnCmdResetAll(wxCommandEvent&);

    wxString mAutoSave_Interval;
    bool mBeep_On_Collisions;
    bool mScroll_Natural;
    bool mSetSheet_Undo;
    bool mSelection_Undo;
};
