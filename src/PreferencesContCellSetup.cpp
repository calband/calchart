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

#include "PreferencesContCellSetup.h"
#include "CalChartContinuityToken.h"
#include "CalChartDrawPrimativesHelper.h"
#include "ContinuityBrowserPanel.h"
#include "PreferencesUtils.h"

#include <wx/bmpcbox.h>
#include <wx/colordlg.h>
#include <wx/spinctrl.h>

enum {
    SPIN_WIDTH = 1000,
    NEW_COLOR_CHOICE,
    SPIN_Font_Size,
    SPIN_Rouding,
    SPIN_Text_Padding,
    SPIN_Box_Padding,
};

BEGIN_EVENT_TABLE(ContCellSetup, PreferencePage)
EVT_SPINCTRL(SPIN_WIDTH, ContCellSetup::OnCmdSelectWidth)
EVT_SPINCTRL(SPIN_Font_Size, ContCellSetup::OnCmdFontSize)
EVT_SPINCTRL(SPIN_Rouding, ContCellSetup::OnCmdRounding)
EVT_SPINCTRL(SPIN_Text_Padding, ContCellSetup::OnCmdTextPadding)
EVT_SPINCTRL(SPIN_Box_Padding, ContCellSetup::OnCmdBoxPadding)
EVT_COMBOBOX(NEW_COLOR_CHOICE, ContCellSetup::OnCmdChooseNewColor)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ContCellSetup, PreferencePage)

template <typename T>
static auto do_cloning(T const& cont)
{
    auto copied_cont = std::vector<std::unique_ptr<CalChart::Cont::Procedure>>{};
    for (auto&& i : cont.GetParsedContinuity()) {
        copied_cont.emplace_back(i->clone());
    }
    return copied_cont;
}

void ContCellSetup::CreateControls()
{
    for (auto i = 0; i < toUType(CalChart::ContinuityCellColors::NUM); ++i) {
        auto brushAndPen = mConfig.Get_ContCellBrushAndPen(static_cast<CalChart::ContinuityCellColors>(i));
        mContCellPens[i] = wxCalChart::toPen(brushAndPen);
        mContCellBrushes[i] = wxCalChart::toBrush(brushAndPen);
    }
    auto colorNames = std::vector<std::tuple<wxString, wxBitmap>>{};
    for (auto i : CalChart::ContinuityCellColorsIterator{}) {
        colorNames.push_back({ CalChart::GetContCellColorNames().at(toUType(i)), CreateItemBitmap(wxCalChart::toBrush(mConfig.Get_ContCellBrushAndPen(i))) });
    }
    wxUI::VSizer{
        LeftBasicSizerFlags(),
        wxUI::VSizer{
            "Color settings",
            wxUI::HSizer{
                mNameBox = wxUI::BitmapComboBox(NEW_COLOR_CHOICE, colorNames)
                               .withSize({ 200, -1 })
                               .withStyle(wxCB_READONLY | wxCB_DROPDOWN),
                mSpin = wxUI::SpinCtrl(SPIN_WIDTH, std::pair{ 1, 10 }, mContCellPens[0].GetWidth())
                            .withStyle(wxSP_ARROW_KEYS),
            },

            wxUI::HSizer{
                wxUI::Button("&Change Color")
                    .bind([this] { OnCmdSelectColors(); }),
                wxUI::Button("&Reset Color")
                    .bind([this] { OnCmdResetColors(); }),
            },
        },
        wxUI::HSizer{
            "Cont Cell settings",
            wxUI::VSizer{
                "Long form",
                wxUI::CheckBox{ "Long form" }
                    .withValue(mConfig.Get_ContCellLongForm())
                    .bind([this](wxCommandEvent& e) {
                        mConfig.Set_ContCellLongForm(e.IsChecked());
                        Refresh();
                    }),
            },
            wxUI::VSizer{
                "Font Size",
                wxUI::SpinCtrl{ SPIN_Font_Size, std::pair{ 1, 30 }, mConfig.Get_ContCellFontSize() }
                    .withStyle(wxSP_ARROW_KEYS) },

            wxUI::VSizer{
                "Rounding",
                wxUI::SpinCtrl{ SPIN_Rouding, std::pair{ 1, 10 }, mConfig.Get_ContCellRounding() }
                    .withStyle(wxSP_ARROW_KEYS) },
            wxUI::VSizer{
                "Text Padding",
                wxUI::SpinCtrl{ SPIN_Text_Padding, std::pair{ 1, 10 }, mConfig.Get_ContCellTextPadding() }
                    .withStyle(wxSP_ARROW_KEYS) },
            wxUI::VSizer{
                "Box Padding",
                wxUI::SpinCtrl{ SPIN_Box_Padding, std::pair{ 1, 10 }, mConfig.Get_ContCellBoxPadding() }
                    .withStyle(wxSP_ARROW_KEYS) },
        },

        wxUI::Generic<ContinuityBrowserPanel>{
            ExpandSizerFlags(),
            [this] {
                auto canvas = new ContinuityBrowserPanel(CalChart::SYMBOL_PLAIN, mConfig, this);
                auto basic_cont = CalChart::Continuity{ "ewns np\nX = distfrom(sp r2)\nmt (24-X)w\nmarch gv dist(np) dir(np) w\nmtrm e" };
                auto clonedOut = do_cloning(basic_cont);
                clonedOut.emplace_back(std::make_unique<CalChart::Cont::ProcMT>(std::make_unique<CalChart::Cont::ValueUnset>(), std::make_unique<CalChart::Cont::ValueUnset>()));
                canvas->DoSetContinuity(CalChart::Continuity{ std::move(clonedOut) });
                return canvas;
            }(),
        },
    }
        .attachTo(this);

    TransferDataToWindow();
}

void ContCellSetup::InitFromConfig()
{
    // first read out the defaults:
    for (auto i = 0; i < toUType(CalChart::ContinuityCellColors::NUM); ++i) {
        auto brushAndPen = mConfig.Get_ContCellBrushAndPen(static_cast<CalChart::ContinuityCellColors>(i));
        mContCellPens[i] = wxCalChart::toPen(brushAndPen);
        mContCellBrushes[i] = wxCalChart::toBrush(brushAndPen);
    }
}

bool ContCellSetup::TransferDataToWindow()
{
    return true;
}

bool ContCellSetup::TransferDataFromWindow()
{
    return true;
}

bool ContCellSetup::ClearValuesToDefault()
{
    mConfig.Clear_ContCellLongForm();
    mConfig.Clear_ContCellFontSize();
    mConfig.Clear_ContCellRounding();
    mConfig.Clear_ContCellTextPadding();
    mConfig.Clear_ContCellBoxPadding();

    for (auto i = CalChart::ContinuityCellColors::PROC; i != CalChart::ContinuityCellColors::NUM; i = static_cast<CalChart::ContinuityCellColors>(static_cast<int>(i) + 1)) {
        SetColor(toUType(i), CalChart::GetContCellDefaultPenWidth()[toUType(i)], wxColour{ CalChart::GetContCellDefaultColors()[toUType(i)] });
        mConfig.Clear_ContCellConfigColor(i);
    }
    InitFromConfig();
    TransferDataToWindow();
    return true;
}

void ContCellSetup::OnCmdFontSize(wxSpinEvent& e)
{
    mConfig.Set_ContCellFontSize(e.GetValue());
    Refresh();
}

void ContCellSetup::OnCmdRounding(wxSpinEvent& e)
{
    mConfig.Set_ContCellRounding(e.GetValue());
    Refresh();
}

void ContCellSetup::OnCmdTextPadding(wxSpinEvent& e)
{
    mConfig.Set_ContCellTextPadding(e.GetValue());
    Refresh();
}

void ContCellSetup::OnCmdBoxPadding(wxSpinEvent& e)
{
    mConfig.Set_ContCellBoxPadding(e.GetValue());
    Refresh();
}

void ContCellSetup::SetColor(int selection, int width, wxColour const& color)
{
    mContCellPens[selection] = *wxThePenList->FindOrCreatePen(color, width, wxPENSTYLE_SOLID);
    mContCellBrushes[selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    mConfig.Set_ContCellBrushAndPen(static_cast<CalChart::ContinuityCellColors>(selection), wxCalChart::toBrushAndPen(color, width));

    // update the namebox list
    CreateAndSetItemBitmap(mNameBox.control(), selection, mContCellBrushes[selection]);
    *mSpin = mContCellPens[mNameBox.selection()].GetWidth();
    Refresh();
}

void ContCellSetup::OnCmdSelectColors()
{
    auto selection = static_cast<int>(mNameBox.selection());
    auto data = wxColourData{};
    data.SetChooseFull(true);
    data.SetColour(mContCellBrushes[selection].GetColour());
    auto dialog = wxColourDialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        auto retdata = dialog.GetColourData();
        auto c = retdata.GetColour();
        SetColor(selection, mContCellPens[selection].GetWidth(), c);
    }
    Refresh();
}

void ContCellSetup::OnCmdSelectWidth(wxSpinEvent& e)
{
    auto selection = static_cast<int>(mNameBox.selection());
    SetColor(selection, e.GetPosition(), mContCellPens[selection].GetColour());
}

void ContCellSetup::OnCmdResetColors()
{
    auto selection = static_cast<int>(mNameBox.selection());
    SetColor(selection, CalChart::GetContCellDefaultPenWidth()[selection], wxColour{ CalChart::GetContCellDefaultColors()[selection] });
    mConfig.Clear_ContCellConfigColor(static_cast<CalChart::ContinuityCellColors>(selection));
    Refresh();
}

void ContCellSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    *mSpin = mContCellPens[mNameBox.selection()].GetWidth();
    Refresh();
}
