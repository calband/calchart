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
#include "PreferencesUtils.h"
#include "CalChartDoc.h"
#include "CalChartSizes.h"
#include "ColorSetupCanvas.h"
#include "ContinuityBrowserPanel.h"
#include "ContinuityComposerDialog.h"
#include "cc_drawcommand.h"
#include "cc_shapes.h"
#include "cc_sheet.h"
#include "confgr.h"
#include "cont.h"
#include "draw.h"
#include "mode_dialog.h"
#include "mode_dialog_canvas.h"
#include "modes.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

using namespace CalChart;

enum {
    BUTTON_SELECT = 1000,
    BUTTON_RESTORE,
    SPIN_WIDTH,
    NEW_COLOR_CHOICE,
    DOTRATIO,
    NUMRATIO,
    PLINERATIO,
    SLINERATIO,
    SPRITESCALE,
    SPRITEHEIGHT,
    BUTTON_EDIT_PALETTE_COLOR,
    BUTTON_EDIT_PALETTE_NAME,
    NEW_COLOR_PALETTE,
};

BEGIN_EVENT_TABLE(DrawingSetup, PreferencePage)
EVT_BUTTON(BUTTON_SELECT, DrawingSetup::OnCmdSelectColors)
EVT_BUTTON(BUTTON_RESTORE, DrawingSetup::OnCmdResetColors)
EVT_SPINCTRL(SPIN_WIDTH, DrawingSetup::OnCmdSelectWidth)
EVT_COMBOBOX(NEW_COLOR_CHOICE, DrawingSetup::OnCmdChooseNewColor)
EVT_COMBOBOX(NEW_COLOR_PALETTE, DrawingSetup::OnCmdChooseNewPalette)
EVT_TEXT_ENTER(DOTRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(NUMRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SPRITESCALE, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SPRITEHEIGHT, DrawingSetup::OnCmdTextChanged)
EVT_BUTTON(BUTTON_EDIT_PALETTE_COLOR, DrawingSetup::OnCmdChangePaletteColor)
EVT_BUTTON(BUTTON_EDIT_PALETTE_NAME, DrawingSetup::OnCmdChangePaletteName)
END_EVENT_TABLE()

IMPLEMENT_CLASS(DrawingSetup, PreferencePage)

void DrawingSetup::CreateControls()
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    wxStaticBoxSizer* boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Palette Selector")), wxVERTICAL);
    topsizer->Add(boxsizer);

    mPaletteNameBox = new wxBitmapComboBox(this, NEW_COLOR_PALETTE, mColorPaletteNames.at(0), wxDefaultPosition, wxSize(200, -1), kNumberPalettes, mColorPaletteNames.data(), wxCB_READONLY | wxCB_DROPDOWN);
    boxsizer->Add(mPaletteNameBox, LeftBasicSizerFlags());

    for (auto i = 0; i < kNumberPalettes; ++i) {
        CreateAndSetItemBitmap(mPaletteNameBox, i, mColorPaletteColors.at(i));
    }
    mPaletteNameBox->SetSelection(mActiveColorPalette);

    auto horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(horizontalsizer, BasicSizerFlags());

    horizontalsizer->Add(new wxButton(this, BUTTON_EDIT_PALETTE_COLOR, wxT("&Edit Color")), BasicSizerFlags());
    horizontalsizer->Add(new wxButton(this, BUTTON_EDIT_PALETTE_NAME, wxT("&Edit Name")), BasicSizerFlags());

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Color settings")), wxVERTICAL);
    topsizer->Add(boxsizer);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    nameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, mConfig.GetColorNames().at(0), wxDefaultPosition, wxDefaultSize, COLOR_NUM, mConfig.GetColorNames().data(), wxCB_READONLY | wxCB_DROPDOWN);
    horizontalsizer->Add(nameBox, BasicSizerFlags());

    for (CalChartColors i = COLOR_FIELD; i < COLOR_NUM; i = static_cast<CalChartColors>(static_cast<int>(i) + 1)) {
        CreateAndSetItemBitmap(nameBox, i, mConfig.Get_CalChartBrushAndPen(i).first);
    }
    nameBox->SetSelection(0);

    spin = new wxSpinCtrl(this, SPIN_WIDTH, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());
    spin->SetValue(mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());
    horizontalsizer->Add(spin, BasicSizerFlags());
    boxsizer->Add(horizontalsizer, LeftBasicSizerFlags());

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);

    horizontalsizer->Add(new wxButton(this, BUTTON_SELECT, wxT("&Change Color")), BasicSizerFlags());
    horizontalsizer->Add(new wxButton(this, BUTTON_RESTORE, wxT("&Reset Color")), BasicSizerFlags());

    boxsizer->Add(horizontalsizer, BasicSizerFlags());

    boxsizer = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("ratios")), wxVERTICAL);
    topsizer->Add(boxsizer);

    horizontalsizer = new wxBoxSizer(wxHORIZONTAL);
    boxsizer->Add(horizontalsizer, LeftBasicSizerFlags());

    AddTextboxWithCaption(this, horizontalsizer, DOTRATIO, wxT("Dot Ratio:"), wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, NUMRATIO, wxT("Num Ratio:"), wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, PLINERATIO, wxT("P-Line Ratio:"), wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, SLINERATIO, wxT("S-Line Ratio:"), wxTE_PROCESS_ENTER);

    AddTextboxWithCaption(this, horizontalsizer, SPRITESCALE, wxT("Sprite Scale:"), wxTE_PROCESS_ENTER);
    AddTextboxWithCaption(this, horizontalsizer, SPRITEHEIGHT, wxT("Sprite Height:"), wxTE_PROCESS_ENTER);

    auto prefCanvas = new ColorSetupCanvas(mConfig, this);
    // set scroll rate 1 to 1, so we can have even scrolling of whole field
    topsizer->Add(prefCanvas, 1, wxEXPAND);
    //	mCanvas->SetScrollRate(1, 1);

    TransferDataToWindow();
}

void DrawingSetup::Init()
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
        CreateAndSetItemBitmap(nameBox, i, mCalChartBrushes[mActiveColorPalette][i]);
    }
    spin->SetValue(mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());

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
    Init();
    TransferDataToWindow();
    return true;
}

// when the palette changes
void DrawingSetup::SetColor(int selection, int width, const wxColour& color)
{
    mCalChartPens[mActiveColorPalette][selection] = *wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    mCalChartBrushes[mActiveColorPalette][selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    mConfig.Set_CalChartBrushAndPen(static_cast<CalChartColors>(selection), mCalChartBrushes[mActiveColorPalette][selection], mCalChartPens[mActiveColorPalette][selection]);

    // update the namebox list
    CreateAndSetItemBitmap(nameBox, selection, mCalChartBrushes[mActiveColorPalette][selection]);
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

void DrawingSetup::OnCmdSelectColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
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

void DrawingSetup::OnCmdChangePaletteColor(wxCommandEvent&)
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

void DrawingSetup::OnCmdChangePaletteName(wxCommandEvent&)
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
    int selection = nameBox->GetSelection();
    SetColor(selection, e.GetPosition(), mCalChartPens[mActiveColorPalette][selection].GetColour());
}

void DrawingSetup::OnCmdResetColors(wxCommandEvent&)
{
    int selection = nameBox->GetSelection();
    SetColor(selection, mConfig.GetDefaultPenWidth()[selection],
        mConfig.GetDefaultColors()[selection]);
    mConfig.Clear_CalChartConfigColor(static_cast<CalChartColors>(selection));
}

void DrawingSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    spin->SetValue(mCalChartPens[mActiveColorPalette][nameBox->GetSelection()].GetWidth());
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

