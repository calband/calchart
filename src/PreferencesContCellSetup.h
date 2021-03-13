#pragma once
/*
 * cc_preferences.h
 * Dialox box for preferences
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

/// Continuity Cell setup
/// Handles configurations parameters of Continuity input system
///

class wxBitmapComboBox;

class ContCellSetup : public PreferencePage {
    DECLARE_CLASS(ContCellSetup)
    DECLARE_EVENT_TABLE()

public:
    ContCellSetup(CalChartConfiguration& config, wxWindow* parent, wxWindowID id = wxID_ANY, wxString const& caption = wxT("ContCell Setup"), wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    virtual ~ContCellSetup() = default;

    virtual void Init() override;
    virtual void CreateControls() override;

    // use these to get and set default values
    virtual bool TransferDataToWindow() override;
    virtual bool TransferDataFromWindow() override;
    virtual bool ClearValuesToDefault() override;

private:
    void OnCmdLongForm(wxCommandEvent&);
    void OnCmdFontSize(wxSpinEvent&);
    void OnCmdRounding(wxSpinEvent&);
    void OnCmdTextPadding(wxSpinEvent&);
    void OnCmdBoxPadding(wxSpinEvent&);
    void OnCmdSelectWidth(wxSpinEvent&);

    void OnCmdSelectColors(wxCommandEvent&);
    void OnCmdChooseNewColor(wxCommandEvent&);
    void OnCmdResetColors(wxCommandEvent&);
    void SetColor(int selection, const wxColour& color);

    // we can set up the Font, colors, size.
    wxBitmapComboBox* nameBox;
    wxBrush mContCellBrushes[COLOR_CONTCELLS_NUM];
};
