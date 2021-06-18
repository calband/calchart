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
    using super = PreferencePage;
    DECLARE_CLASS(ContCellSetup)
    DECLARE_EVENT_TABLE()

public:
    ContCellSetup(CalChartConfiguration& config, wxWindow* parent);
    ~ContCellSetup() override = default;

    void Init() override;
    void CreateControls() override;

    // use these to get and set default values
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;
    bool ClearValuesToDefault() override;

private:
    void OnCmdFontSize(wxSpinEvent&);
    void OnCmdRounding(wxSpinEvent&);
    void OnCmdTextPadding(wxSpinEvent&);
    void OnCmdBoxPadding(wxSpinEvent&);
    void OnCmdSelectWidth(wxSpinEvent&);

    void OnCmdSelectColors();
    void OnCmdChooseNewColor(wxCommandEvent&);
    void OnCmdResetColors();
    void SetColor(int selection, const wxColour& color);

    // we can set up the Font, colors, size.
    wxBitmapComboBox* mNameBox{};
    wxBrush mContCellBrushes[COLOR_CONTCELLS_NUM]{};
};
