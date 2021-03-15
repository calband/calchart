#pragma once
/*
 * PreferencesDrawingSetup.h
 * Dialox box for Drawing Setup part of preferences
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
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>

class DrawingSetup : public PreferencePage {
    DECLARE_CLASS(DrawingSetup)
    DECLARE_EVENT_TABLE()

public:
    DrawingSetup(CalChartConfiguration& config, wxWindow* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("General Setup"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
        : PreferencePage(config)
    {
        Init();
        Create(parent, id, caption, pos, size, style);
    }
    virtual ~DrawingSetup() { }

    virtual void Init();
    virtual void CreateControls();

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();
    virtual bool ClearValuesToDefault();

private:
    void OnCmdSelectColors();
    void OnCmdSelectWidth(wxSpinEvent&);
    void OnCmdResetColors();
    void OnCmdResetAll(wxCommandEvent&);
    void OnCmdChooseNewColor(wxCommandEvent&);
    void OnCmdChooseNewPalette(wxCommandEvent&);
    void OnCmdTextChanged(wxCommandEvent&);
    void OnCmdChangePaletteColor();
    void OnCmdChangePaletteName();

    void SetColor(int selection, int width, const wxColour& color);
    void SetPaletteColor(int selection, const wxColour& color);
    void SetPaletteName(int selection, const wxString& name);
    wxBitmapComboBox* mNameBox;
    wxBitmapComboBox* mPaletteNameBox;
    wxSpinCtrl* spin;

    int mActiveColorPalette{};
    std::vector<wxString> mColorPaletteNames;
    std::vector<wxBrush> mColorPaletteColors;
    wxPen mCalChartPens[kNumberPalettes][COLOR_NUM];
    wxBrush mCalChartBrushes[kNumberPalettes][COLOR_NUM];

    double mDrawingValues[6];
};
