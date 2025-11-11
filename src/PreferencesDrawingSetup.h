#pragma once
/*
 * PreferencesDrawingSetup.h
 * Dialox box for Drawing Setup part of preferences
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

#include "CalChartConstants.h"
#include "CalChartDrawPrimatives.h"
#include "PreferencesUtils.h"
#include <wx/bmpcbox.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class DrawingSetup : public PreferencePage {
    using super = PreferencePage;
    DECLARE_CLASS(DrawingSetup)
    DECLARE_EVENT_TABLE()

public:
    static auto CreatePreference(CalChart::Configuration& config, wxWindow* parent) -> DrawingSetup*
    {
        auto* pref = new DrawingSetup(config, parent);
        pref->Initialize();
        return pref;
    }

private:
    // private, use the CreatePreference method
    DrawingSetup(CalChart::Configuration& config, wxWindow* parent);

public:
    ~DrawingSetup() override = default;

    // use these to get and set default values
    auto TransferDataToWindow() -> bool override;
    auto TransferDataFromWindow() -> bool override;
    auto ClearValuesToDefault() -> bool override;

private:
    void InitFromConfig() override;
    void CreateControls() override;

    void OnCmdSelectColors();
    void SelectPenWidth(int width);
    void OnCmdResetColors();
    void OnCmdResetAll(wxCommandEvent&);
    void OnCmdChooseNewColor(wxCommandEvent&);
    void OnCmdChooseNewPalette(wxCommandEvent&);
    void OnCmdTextChanged(wxCommandEvent&);
    void OnCmdChangePaletteColor();
    void OnCmdChangePaletteName();

    void SetColor(int selection, int width, const wxColour& color);
    void SetPaletteColor(int selection, const wxColour& color);
    void SetPaletteName(int selection, const std::string& name);
    wxUI::BitmapComboBox::Proxy mNameBox{};
    wxUI::BitmapComboBox::Proxy mPaletteNameBox{};
    wxUI::SpinCtrl::Proxy mSpin{};

    wxUI::TextCtrl::Proxy mDotRatio{};
    wxUI::TextCtrl::Proxy mNumRatio{};
    wxUI::TextCtrl::Proxy mPLineRatio{};
    wxUI::TextCtrl::Proxy mSLineRatio{};
    wxUI::TextCtrl::Proxy mSpriteScale{};
    wxUI::TextCtrl::Proxy mSpriteHeight{};
    wxUI::TextCtrl::Proxy mCurveControl{};

    int mActiveColorPalette{};
    std::vector<std::string> mColorPaletteNames;
    std::vector<CalChart::Color> mColorPaletteColors;
    std::array<std::array<wxPen, toUType(CalChart::Colors::NUM)>, CalChart::kNumberPalettes> mCalChartPens{};
    std::array<std::array<wxBrush, toUType(CalChart::Colors::NUM)>, CalChart::kNumberPalettes> mCalChartBrushes{};

    std::array<double, 7> mDrawingValues{};
};
