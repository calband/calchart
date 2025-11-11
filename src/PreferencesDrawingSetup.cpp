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
#include "ModeSetupDialog.h"
#include "PreferencesUtils.h"
#include "ShowModeSetupCanvas.h"

#include <wx/colordlg.h>
#include <wx/dcbuffer.h>
#include <wx/listbook.h>
#include <wx/notebook.h>
#include <wx/statline.h>
#include <wx/stattext.h>

namespace {
enum {
    NEW_COLOR_CHOICE = 1000,
    DOTRATIO,
    NUMRATIO,
    PLINERATIO,
    SLINERATIO,
    SPRITESCALE,
    SPRITEHEIGHT,
    CURVECONTROL,
    NEW_COLOR_PALETTE,
};

}

BEGIN_EVENT_TABLE(DrawingSetup, PreferencePage)
EVT_COMBOBOX(NEW_COLOR_CHOICE, DrawingSetup::OnCmdChooseNewColor)
EVT_COMBOBOX(NEW_COLOR_PALETTE, DrawingSetup::OnCmdChooseNewPalette)
EVT_TEXT_ENTER(DOTRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(NUMRATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(PLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SLINERATIO, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SPRITESCALE, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(SPRITEHEIGHT, DrawingSetup::OnCmdTextChanged)
EVT_TEXT_ENTER(CURVECONTROL, DrawingSetup::OnCmdTextChanged)
END_EVENT_TABLE()

IMPLEMENT_CLASS(DrawingSetup, PreferencePage)

// private, use the CreatePreference method
DrawingSetup::DrawingSetup(CalChart::Configuration& config, wxWindow* parent)
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
        wxUI::HSizer{
            wxUI::VSizer{
                "Palette Selector",
                wxUI::BitmapComboBox(NEW_COLOR_PALETTE, colorPalettes)
                    .withSelection(mActiveColorPalette)
                    .withSize({ 200, -1 })
                    .withStyle(wxCB_READONLY | wxCB_DROPDOWN)
                    .withProxy(mPaletteNameBox),

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
                    wxUI::BitmapComboBox(NEW_COLOR_CHOICE, colorNames)
                        .withSelection(0)
                        .withSize({ 200, -1 })
                        .withStyle(wxCB_READONLY | wxCB_DROPDOWN)
                        .withProxy(mNameBox),
                    wxUI::SpinCtrl(std::pair{ 1, 10 }, mCalChartPens[mActiveColorPalette][0].GetWidth())
                        .withStyle(wxSP_ARROW_KEYS)
                        .withProxy(mSpin)
                        .bind([this](wxSpinEvent& e) {
                            SelectPenWidth(e.GetPosition());
                        }),
                },

                wxUI::HSizer{
                    wxUI::Button("&Change Color")
                        .bind([this] { OnCmdSelectColors(); }),
                    wxUI::Button("&Reset Name")
                        .bind([this] { OnCmdResetColors(); }),
                },
            },
        },
        wxUI::HSizer{
            "ratios",
            VLabelWidget("Dot Ratio:", wxUI::TextCtrl{ DOTRATIO }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mDotRatio)),
            VLabelWidget("Num Ratio:", wxUI::TextCtrl{ NUMRATIO }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mNumRatio)),
            VLabelWidget("P-Line Ratio:", wxUI::TextCtrl{ PLINERATIO }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mPLineRatio)),
            VLabelWidget("S-Line Ratio:", wxUI::TextCtrl{ SLINERATIO }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mSLineRatio)),
            VLabelWidget("Sprite Scale:", wxUI::TextCtrl{ SPRITESCALE }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mSpriteScale)),
            VLabelWidget("Sprite Height:", wxUI::TextCtrl{ SPRITEHEIGHT }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mSpriteHeight)),
            VLabelWidget("Curve Box:", wxUI::TextCtrl{ CURVECONTROL }.withSize({ 80, -1 }).withStyle(wxTE_PROCESS_ENTER).withProxy(mCurveControl)),
        },
        wxUI::Generic{ ExpandSizerFlags(), new ColorSetupCanvas(mConfig, this) },
    }
        .fitTo(this);

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
    mDrawingValues[6] = mConfig.Get_ControlPointRatio();
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

    *mDotRatio = std::format("{:.2f}", mDrawingValues[0]);
    *mNumRatio = std::format("{:.2f}", mDrawingValues[1]);
    *mPLineRatio = std::format("{:.2f}", mDrawingValues[2]);
    *mSLineRatio = std::format("{:.2f}", mDrawingValues[3]);
    *mSpriteScale = std::format("{:.2f}", mDrawingValues[4]);
    *mSpriteHeight = std::format("{:.2f}", mDrawingValues[5]);
    *mCurveControl = std::format("{:.2f}", mDrawingValues[6]);

    Refresh();
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

    mConfig.Clear_DotRatio();
    mConfig.Clear_NumRatio();
    mConfig.Clear_PLineRatio();
    mConfig.Clear_SLineRatio();
    mConfig.Clear_SpriteBitmapScale();
    mConfig.Clear_SpriteBitmapOffsetY();
    mConfig.Clear_ControlPointRatio();

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

void DrawingSetup::SetPaletteName(int selection, std::string const& name)
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
        SetColor(selection, mCalChartPens[mActiveColorPalette][selection].GetWidth(), dialog.GetColourData().GetColour());
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
        SetPaletteName(selection, dialog.GetValue().ToStdString());
    }
    Refresh();
}

void DrawingSetup::SelectPenWidth(int width)
{
    int selection = mNameBox.selection();
    SetColor(selection, width, mCalChartPens[mActiveColorPalette][selection].GetColour());
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
        case 6:
            mConfig.Set_ControlPointRatio(value);
            break;
        }
    }
    Refresh();
}
