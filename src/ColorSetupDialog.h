#pragma once
/*
 * ColorSetupCanvas.h
 * Dialox box for setting up colors
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

#include "CalChartConfiguration.h"
#include <nlohmann/json.hpp>
#include <wx/bmpcbox.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

class ColorSetupDialog : public wxDialog {
    using super = wxDialog;
    DECLARE_EVENT_TABLE()

public:
    ColorSetupDialog(wxWindow* parent, int palette);
    virtual ~ColorSetupDialog() = default;

private:
    void Init();
    void CreateControls();

    // use these to get and set default values
    bool TransferDataToWindow() override;
    bool TransferDataFromWindow() override;

    void OnCmdSelectColors(wxCommandEvent&);
    void OnCmdSelectWidth(wxSpinEvent&);
    void OnCmdResetColors(wxCommandEvent&);
    void OnCmdResetAll(wxCommandEvent&);
    void OnCmdChooseNewColor(wxCommandEvent&);
    void OnCmdTextChanged(wxCommandEvent&);
    void OnCmdChangePaletteColor(wxCommandEvent&);
    void OnCmdExport(wxCommandEvent&);
    void OnCmdImport(wxCommandEvent&);

    void SetColor(int selection, int width, const wxColour& color);
    void SetPaletteColor(const wxColour& color);
    void SetPaletteName(const wxString& name);
    bool ClearValuesToDefault();

    nlohmann::json Export() const;
    void Import(nlohmann::json const&);

    wxBitmapComboBox* mNameBox{};
    wxSpinCtrl* mSpin{};

    int const mActiveColorPalette{};
    std::vector<wxString> mColorPaletteNames;
    std::vector<wxBrush> mColorPaletteColors;
    wxPen mCalChartPens[kNumberPalettes][COLOR_NUM];
    wxBrush mCalChartBrushes[kNumberPalettes][COLOR_NUM];

    CalChartConfiguration mConfig;
};
