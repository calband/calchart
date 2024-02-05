/*
 * CalChartPreferences.cpp
 * Dialox box for preferences
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

#include "PreferencesDrawingSetup.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartShapes.h"
#include "CalChartSheet.h"
#include "CalChartShowMode.h"
#include "CalChartSizes.h"
#include "ColorSetupCanvas.h"
#include "ContinuityBrowserPanel.h"
#include "ModeSetupDialog.h"
#include "PreferencesUtils.h"
#include "ShowModeSetupCanvas.h"

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

// private, use the CreatePreference method
DrawingSetup::DrawingSetup(CalChartConfiguration& config, wxWindow* parent)
    : super(config, parent, "Drawing Setup")
{
    // first read out the defaults:
    mActiveColorPalette = mConfig.Get_ActiveColorPalette();
    mColorPaletteNames = GetColorPaletteNames(mConfig);
    mColorPaletteColors = GetColorPaletteColors(mConfig);
    for (auto palette = 0; palette < CalChart::kNumberPalettes; ++palette) {
        for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
            auto brushAndPen = mConfig.Get_CalChartBrushAndPen(palette, i);
            mCalChartPens[palette][toUType(i)] = wxCalChart::toPen(brushAndPen);
            mCalChartBrushes[palette][toUType(i)] = wxCalChart::toBrush(brushAndPen);
        }
    }
}

void DrawingSetup::CreateControls()
{
    auto colorPalettes = std::vector<std::tuple<wxString, wxBitmap>>{};
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        colorPalettes.push_back({ mColorPaletteNames.at(i), CreateItemBitmap(wxCalChart::toBrush(mColorPaletteColors.at(i))) });
    }
    auto colorNames = std::vector<std::tuple<wxString, wxBitmap>>{};
    for (auto i : CalChart::ColorsIterator{}) {
        colorNames.push_back({ CalChart::GetColorNames().at(toUType(i)), CreateItemBitmap(wxCalChart::toBrush(mConfig.Get_CalChartBrushAndPen(i))) });
    }
    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::VSizer{
            "Palette Selector",
            mPaletteNameBox = wxUI::BitmapComboBox(NEW_COLOR_PALETTE, colorPalettes)
                                  .withSelection(mActiveColorPalette)
                                  .withSize({ 200, -1 })
                                  .withStyle(wxCB_READONLY | wxCB_DROPDOWN),

            wxUI::HSizer{
                wxUI::Button("&Edit Color")
                    .bind([this] { OnCmdChangePaletteColor(); }),
                wxUI::Button("&Edit Name")
                    .bind([this] { OnCmdChangePaletteName(); }),
            },
        },
        wxUI::VSizer{
            "Color settings",
            wxUI::HSizer{
                mNameBox = wxUI::BitmapComboBox(NEW_COLOR_CHOICE, colorNames)
                               .withSelection(0)
                               .withSize({ 200, -1 })
                               .withStyle(wxCB_READONLY | wxCB_DROPDOWN),
                mSpin = wxUI::SpinCtrl(SPIN_WIDTH, std::pair{ 1, 10 }, mCalChartPens[mActiveColorPalette][0].GetWidth())
                            .withStyle(wxSP_ARROW_KEYS),
            },

            wxUI::HSizer{
                wxUI::Button("&Change Color")
                    .bind([this] { OnCmdSelectColors(); }),
                wxUI::Button("&Reset Name")
                    .bind([this] { OnCmdResetColors(); }),
            },
        },
        wxUI::HSizer{
            "ratios",
            VLabelWidget("Dot Ratio:", wxUI::TextCtrl{ DOTRATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER)),
            VLabelWidget("Num Ratio:", wxUI::TextCtrl{ NUMRATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER)),
            VLabelWidget("P-Line Ratio:", wxUI::TextCtrl{ PLINERATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER)),
            VLabelWidget("S-Line Ratio:", wxUI::TextCtrl{ SLINERATIO }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER)),
            VLabelWidget("Sprite Scale:", wxUI::TextCtrl{ SPRITESCALE }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER)),
            VLabelWidget("Sprite Height:", wxUI::TextCtrl{ SPRITEHEIGHT }.withSize({ 100, -1 }).withStyle(wxTE_PROCESS_ENTER)),
        },
        wxUI::Generic{ ExpandSizerFlags(), new ColorSetupCanvas(mConfig, this) },
    }
        .attachTo(this);

    TransferDataToWindow();
}

void DrawingSetup::InitFromConfig()
{
    // first read out the defaults:
    mActiveColorPalette = mConfig.Get_ActiveColorPalette();
    mColorPaletteNames = GetColorPaletteNames(mConfig);
    mColorPaletteColors = GetColorPaletteColors(mConfig);

    for (auto palette = 0; palette < CalChart::kNumberPalettes; ++palette) {
        for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
            auto brushAndPen = mConfig.Get_CalChartBrushAndPen(palette, i);
            mCalChartPens[palette][toUType(i)] = wxCalChart::toPen(brushAndPen);
            mCalChartBrushes[palette][toUType(i)] = wxCalChart::toBrush(brushAndPen);
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
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        CreateAndSetItemBitmap(mPaletteNameBox.control(), i, wxCalChart::toBrush(mColorPaletteColors.at(i)));
    }
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        mPaletteNameBox.control()->SetString(i, mColorPaletteNames.at(i));
    }
    mPaletteNameBox.control()->SetSelection(mActiveColorPalette);

    for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
        CreateAndSetItemBitmap(mNameBox.control(), toUType(i), mCalChartBrushes[mActiveColorPalette][toUType(i)]);
    }
    *mSpin = mCalChartPens[mActiveColorPalette][static_cast<int>(mNameBox.selection())].GetWidth();

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
    mConfig.Set_ActiveColorPalette(mActiveColorPalette);
    return true;
}

bool DrawingSetup::ClearValuesToDefault()
{
    mConfig.Clear_ActiveColorPalette();
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        mConfig.ClearColorPaletteColor(i);
        mConfig.ClearColorPaletteName(i);
    }

    for (auto palette = 0; palette < CalChart::kNumberPalettes; ++palette) {
        for (auto i = CalChart::Colors::FIELD; i != CalChart::Colors::NUM; i = static_cast<CalChart::Colors>(static_cast<int>(i) + 1)) {
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

    mConfig.Set_CalChartBrushAndPen(mActiveColorPalette, static_cast<CalChart::Colors>(selection), wxCalChart::toBrushAndPen(color, width));

    // update the namebox list
    CreateAndSetItemBitmap(mNameBox.control(), selection, mCalChartBrushes[mActiveColorPalette][selection]);
    Refresh();
}

void DrawingSetup::SetPaletteColor(int selection, wxColour const& color)
{
    mColorPaletteColors.at(selection) = wxCalChart::toColor(color);

    // this is needed so we draw things out on the page correctly.
    mConfig.SetColorPaletteColor(selection, mColorPaletteColors.at(selection));

    CreateAndSetItemBitmap(mPaletteNameBox.control(), selection, wxCalChart::toBrush(mColorPaletteColors.at(selection)));
    Refresh();
}

void DrawingSetup::SetPaletteName(int selection, wxString const& name)
{
    mColorPaletteNames.at(selection) = name;

    // this is needed so we draw things out on the page correctly.
    mConfig.SetColorPaletteName(selection, name);

    mPaletteNameBox.control()->SetString(selection, mColorPaletteNames.at(selection));
    Refresh();
}

void DrawingSetup::OnCmdSelectColors()
{
    int selection = mNameBox.selection();
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
    int selection = mPaletteNameBox.selection();
    wxColourData data;
    data.SetChooseFull(true);
    data.SetColour(wxCalChart::toColour(mColorPaletteColors.at(selection)));
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
    int selection = mPaletteNameBox.selection();
    auto v = static_cast<std::string>(*mPaletteNameBox);
    wxTextEntryDialog dialog(this, wxT("Enter name for Palette"), wxT("Enter name for Palette"), v, wxOK | wxCANCEL);
    if (dialog.ShowModal() == wxID_OK) {
        SetPaletteName(selection, dialog.GetValue());
    }
    Refresh();
}

void DrawingSetup::OnCmdSelectWidth(wxSpinEvent& e)
{
    int selection = mNameBox.selection();
    SetColor(selection, e.GetPosition(), mCalChartPens[mActiveColorPalette][selection].GetColour());
}

void DrawingSetup::OnCmdResetColors()
{
    int selection = mNameBox.selection();
    SetColor(selection, CalChart::GetDefaultPenWidth()[selection], wxColour{ CalChart::GetDefaultColors()[selection] });
    mConfig.Clear_CalChartConfigColor(mActiveColorPalette, static_cast<CalChart::Colors>(selection));
}

void DrawingSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    *mSpin = mCalChartPens[mActiveColorPalette][mNameBox.selection()].GetWidth();
}

void DrawingSetup::OnCmdChooseNewPalette(wxCommandEvent&)
{
    // we set the active palette, and now need to refresh everything
    mActiveColorPalette = mPaletteNameBox.selection();
    mConfig.Set_ActiveColorPalette(mActiveColorPalette);
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
