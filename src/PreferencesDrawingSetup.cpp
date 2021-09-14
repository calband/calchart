/*
 * CalChartPreferences.cpp
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

#include "PreferencesDrawingSetup.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartDrawCommand.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartSizes.h"
#include "ColorSetupCanvas.h"
#include "ContinuityBrowserPanel.h"
#include "ModeSetupDialog.h"
#include "PreferencesUtils.h"
#include "ShowModeSetupCanvas.h"
#include "draw.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

using namespace CalChart;

enum {
    SPIN_WIDTH = 1000,
    NEW_COLOR_CHOICE,
    DOTRATIO,
    NUMRATIO,
    PLINERATIO,
    SLINERATIO,
    SPRITESCALE,
    SPRITEHEIGHT,
    NEW_COLOR_PALETTE,
};

BEGIN_EVENT_TABLE(DrawingSetup, PreferencePage)
EVT_SPINCTRL(SPIN_WIDTH, DrawingSetup::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE, DrawingSetup::OnCmdChooseNewColor)
EVT_COMBOBOX(NEW_COLOR_PALETTE, DrawingSetup::OnCmdChooseNewPalette)
EVT_TEXT_ENTER(DOTRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(NUMRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SPRITESCALE, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SPRITEHEIGHT, DrawingSetup::OnCmdTextChanged)
END_EVENT_TABLE()

IMPLEMENT_CLASS(DrawingSetup, PreferencePage)

void DrawingSetup::CreateControls()
{
    SetSizer(VStack([this](auto& sizer) {
        NamedVBoxStack(this, sizer, "Palette Selector", [this](auto& sizer) {
            mPaletteNameBox = new wxBitmapComboBox(this, NEW_COLOR_PALETTE, mColorPaletteNames.at(0), wxDefaultPosition, wxSize(200, -1), kNumberPalettes, mColorPaletteNames.data(), wxCB_READONLY | wxCB_DROPDOWN);
            sizer->Add(mPaletteNameBox, LeftBasicSizerFlags());

            for (auto i = 0; i < kNumberPalettes; ++i) {
                CreateAndSetItemBitmap(mPaletteNameBox, i, mColorPaletteColors.at(i));
            }
            mPaletteNameBox->SetSelection(mActiveColorPalette);

            HStack(sizer, LeftBasicSizerFlags(), [this](auto& sizer) {
                CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&Edit Color", [this]() {
                    OnCmdChangePaletteColor();
                });
                CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&Edit Name", [this]() {
                    OnCmdChangePaletteName();
                });
            });
        });

        NamedVBoxStack(this, sizer, "Color settings", [this](auto& sizer) {
            HStack(sizer, LeftBasicSizerFlags(), [this](auto& sizer) {
                mNameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, mConfig.GetColorNames().at(0), wxDefaultPosition, wxDefaultSize, COLOR_NUM, mConfig.GetColorNames().data(), wxCB_READONLY | wxCB_DROPDOWN);
                sizer->Add(mNameBox, BasicSizerFlags());

                for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
                    CreateAndSetItemBitmap(mNameBox, i, mConfig.Get_CalChartBrushAndPen(i).first);
                }
                mNameBox->SetSelection(0);

                spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mCalChartPens[mActiveColorPalette][mNameBox->GetSelection()].GetWidth());
                spin->SetValue(mCalChartPens[mActiveColorPalette][mNameBox->GetSelection()].GetWidth());
                sizer->Add(spin, BasicSizerFlags());
            });

            HStack(sizer, LeftBasicSizerFlags(), [this](auto& sizer) {
                CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&Change Color", [this]() {
                    OnCmdSelectColors();
                });
                CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&Reset Color", [this]() {
                    OnCmdResetColors();
                });
            });
        });

        NamedHBoxStack(this, sizer, "ratios", [this](auto& sizer) {
            CreateTextboxWithCaption(this, sizer, DOTRATIO, wxT("Dot Ratio:"), wxTE_PROCESS_ENTER);
            CreateTextboxWithCaption(this, sizer, NUMRATIO, wxT("Num Ratio:"), wxTE_PROCESS_ENTER);
            CreateTextboxWithCaption(this, sizer, PLINERATIO, wxT("P-Line Ratio:"), wxTE_PROCESS_ENTER);
            CreateTextboxWithCaption(this, sizer, SLINERATIO, wxT("S-Line Ratio:"), wxTE_PROCESS_ENTER);

            CreateTextboxWithCaption(this, sizer, SPRITESCALE, wxT("Sprite Scale:"), wxTE_PROCESS_ENTER);
            CreateTextboxWithCaption(this, sizer, SPRITEHEIGHT, wxT("Sprite Height:"), wxTE_PROCESS_ENTER);
        });

        auto prefCanvas = new ColorSetupCanvas(mConfig, this);
        sizer->Add(prefCanvas, 1, wxEXPAND);
    }));

    TransferDataToWindow();
}

void DrawingSetup::InitFromConfig()
{
    // first read out the defaults:
    mActiveColorPalette = mConfig.GetActiveColorPalette();
    mColorPaletteNames = GetColorPaletteNames(mConfig);
    mColorPaletteColors = GetColorPaletteColors(mConfig);

    for (auto palette = 0; palette < kNumberPalettes; ++palette) {
        for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
            auto brushAndPen = mConfig.Get_CalChartBrushAndPen(palette, i);
            mCalChartPens[palette][i] = brushAndPen.second;
            mCalChartBrushes[palette][i] = brushAndPen.first;
        }
    }

    mDrawingValues[0] = mConfig.Get_DotRatio();
    mDrawingValues[1] = mConfig.Get_NumRatio();
    mDrawingValues[2] = mConfig.Get_PLineRatio();
    mDrawingValues[3] = mConfig.Get_SLineRatio();
    mDrawingValues[4] = mConfig.Get_SpriteBitmapScale();
    mDrawingValues[5] = mConfig.Get_SpriteBitmapOffsetY();
}

bool DrawingSetup::TransferDataToWindow()
{
    for (auto i = 0; i < kNumberPalettes; ++i) {
        CreateAndSetItemBitmap(mPaletteNameBox, i, mColorPaletteColors.at(i));
    }
    for (auto i = 0; i < kNumberPalettes; ++i) {
        mPaletteNameBox->SetString(i, mColorPaletteNames.at(i));
    }
    mPaletteNameBox->SetSelection(mActiveColorPalette);

    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM;
         i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        CreateAndSetItemBitmap(mNameBox, i, mCalChartBrushes[mActiveColorPalette][i]);
    }
    spin->SetValue(mCalChartPens[mActiveColorPalette][mNameBox->GetSelection()].GetWidth());

    wxString buf;
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(DOTRATIO);
    buf.Printf(wxT("%.2f"), mDrawingValues[0]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(NUMRATIO);
    buf.Printf(wxT("%.2f"), mDrawingValues[1]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(PLINERATIO);
    buf.Printf(wxT("%.2f"), mDrawingValues[2]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(SLINERATIO);
    buf.Printf(wxT("%.2f"), mDrawingValues[3]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(SPRITESCALE);
    buf.Printf(wxT("%.2f"), mDrawingValues[4]);
    text->SetValue(buf);
    text = (wxTextCtrl*)FindWindow(SPRITEHEIGHT);
    buf.Printf(wxT("%.2f"), mDrawingValues[5]);
    text->SetValue(buf);

    return true;
}

bool DrawingSetup::TransferDataFromWindow()
{
    // Data is already transferred when we update the controls, so nothing to do here.
    mConfig.SetActiveColorPalette(mActiveColorPalette);
    return true;
}

bool DrawingSetup::ClearValuesToDefault()
{
    mConfig.ClearActiveColorPalette();
    for (auto i = 0; i < kNumberPalettes; ++i) {
        mConfig.ClearColorPaletteColor(i);
        mConfig.ClearColorPaletteName(i);
    }

    for (auto palette = 0; palette < kNumberPalettes; ++palette) {
        for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
            mConfig.Clear_CalChartConfigColor(palette, i);
        }
    }
    InitFromConfig();
    return TransferDataToWindow();
}

// when the palette changes
void DrawingSetup::SetColor(int selection, int width, const wxColour& color)
{
    mCalChartPens[mActiveColorPalette][selection] = *wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    mCalChartBrushes[mActiveColorPalette][selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    mConfig.Set_CalChartBrushAndPen(static_cast<CalChartColors>(selection), mCalChartBrushes[mActiveColorPalette][selection], mCalChartPens[mActiveColorPalette][selection]);

    // update the namebox list
    CreateAndSetItemBitmap(mNameBox, selection, mCalChartBrushes[mActiveColorPalette][selection]);
    Refresh();
}

void DrawingSetup::SetPaletteColor(int selection, wxColour const& color)
{
    mColorPaletteColors.at(selection) = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    // this is needed so we draw things out on the page correctly.
    mConfig.SetColorPaletteColor(selection, mColorPaletteColors.at(selection));

    CreateAndSetItemBitmap(mPaletteNameBox, selection, mColorPaletteColors.at(selection));
    Refresh();
}

void DrawingSetup::SetPaletteName(int selection, wxString const& name)
{
    mColorPaletteNames.at(selection) = name;

    // this is needed so we draw things out on the page correctly.
    mConfig.SetColorPaletteName(selection, name);

    mPaletteNameBox->SetString(selection, mColorPaletteNames.at(selection));
    Refresh();
}

void DrawingSetup::OnCmdSelectColors()
{
    int selection = mNameBox->GetSelection();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(mCalChartBrushes[mActiveColorPalette][selection].GetColour());
    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        wxColourData retdata = dialog.GetColourData();
        wxColour c = retdata.GetColour();
        SetColor(selection, mCalChartPens[mActiveColorPalette][selection].GetWidth(), c);
    }
    Refresh();
}

void DrawingSetup::OnCmdChangePaletteColor()
{
    int selection = mPaletteNameBox->GetSelection();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(mColorPaletteColors.at(selection).GetColour());
    wxColourDialog dialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        wxColourData retdata = dialog.GetColourData();
        wxColour c = retdata.GetColour();
        SetPaletteColor(selection, c);
    }
    Refresh();
}

void DrawingSetup::OnCmdChangePaletteName()
{
    int selection = mPaletteNameBox->GetSelection();
    auto v = mPaletteNameBox->GetValue();
    wxTextEntryDialog dialog(this, wxT("Enter name for Palette"), wxT("Enter name for Palette"), v, wxOK | wxCANCEL);
    if (dialog.ShowModal() == wxID_OK) {
        SetPaletteName(selection, dialog.GetValue());
    }
    Refresh();
}

void DrawingSetup::OnCmdSelectWidth(wxSpinEvent& e)
{
    int selection = mNameBox->GetSelection();
    SetColor(selection, e.GetPosition(), mCalChartPens[mActiveColorPalette][selection].GetColour());
}

void DrawingSetup::OnCmdResetColors()
{
    int selection = mNameBox->GetSelection();
    SetColor(selection, mConfig.GetDefaultPenWidth()[selection],
        mConfig.GetDefaultColors()[selection]);
    mConfig.Clear_CalChartConfigColor(static_cast<CalChartColors>(selection));
}

void DrawingSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    spin->SetValue(mCalChartPens[mActiveColorPalette][mNameBox->GetSelection()].GetWidth());
}

void DrawingSetup::OnCmdChooseNewPalette(wxCommandEvent&)
{
    // we set the active palette, and now need to refresh everything
    mActiveColorPalette = mPaletteNameBox->GetSelection();
    mConfig.SetActiveColorPalette(mActiveColorPalette);
    TransferDataToWindow();
    Refresh();
}

void DrawingSetup::OnCmdTextChanged(wxCommandEvent& e)
{
    auto id = e.GetId();
    wxTextCtrl* text = (wxTextCtrl*)FindWindow(id);
    double value;
    if (text->GetValue().ToDouble(&value)) {
        switch (id - DOTRATIO) {
        case 0:
            mConfig.Set_DotRatio(value);
            break;
        case 1:
            mConfig.Set_NumRatio(value);
            break;
        case 2:
            mConfig.Set_PLineRatio(value);
            break;
        case 3:
            mConfig.Set_SLineRatio(value);
            break;
        case 4:
            mConfig.Set_SpriteBitmapScale(value);
            break;
        case 5:
            mConfig.Set_SpriteBitmapOffsetY(value);
            break;
        }
    }
    Refresh();
}
