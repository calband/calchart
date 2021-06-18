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

#include "PreferencesContCellSetup.h"
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
    auto copied_cont = std::vector<std::unique_ptr<CalChart::ContProcedure>>{};
    for (auto&& i : cont.GetParsedContinuity()) {
        copied_cont.emplace_back(i->clone());
    }
    return copied_cont;
}

ContCellSetup::ContCellSetup(CalChartConfiguration& config, wxWindow* parent)
    : super(config)
{
    Init();
    Create(parent, "ContCell Setup");
}

void ContCellSetup::CreateControls()
{
    SetSizer(VStack([this](auto sizer) {
        NamedVBoxStack(this, sizer, "Color settings", [this](auto& sizer) {
            HStack(sizer, LeftBasicSizerFlags(), [this](auto& sizer) {
                mNameBox = new wxBitmapComboBox(this, NEW_COLOR_CHOICE, mConfig.GetContCellColorNames().at(0), wxDefaultPosition, wxDefaultSize, COLOR_CONTCELLS_NUM, mConfig.GetContCellColorNames().data(), wxCB_READONLY | wxCB_DROPDOWN);
                sizer->Add(mNameBox, BasicSizerFlags());

                for (auto i = 0; i < COLOR_CONTCELLS_NUM; ++i) {
                    CreateAndSetItemBitmap(mNameBox, i, mConfig.Get_ContCellBrushAndPen(static_cast<ContCellColors>(i)).first);
                }
                mNameBox->SetSelection(0);
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

        NamedHBoxStack(this, sizer, "Cont Cell settings", [this](auto& sizer) {
            NamedVBoxStack(this, sizer, "Long form", [this](auto& sizer) {
                auto checkbox = new wxCheckBox(this, wxID_ANY, wxT("Long form"));
                checkbox->Bind(wxEVT_CHECKBOX, [this](wxCommandEvent& e) {
                    mConfig.Set_ContCellLongForm(e.IsChecked());
                    Refresh();
                });
                checkbox->SetValue(mConfig.Get_ContCellLongForm());
                sizer->Add(checkbox, BasicSizerFlags());
            });

            NamedVBoxStack(this, sizer, "Font Size", [this](auto& sizer) {
                auto spin = new wxSpinCtrl(this, SPIN_Font_Size, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 30, mConfig.Get_ContCellFontSize());
                sizer->Add(spin, BasicSizerFlags());
            });

            NamedVBoxStack(this, sizer, "Rounding", [this](auto& sizer) {
                auto spin = new wxSpinCtrl(this, SPIN_Rouding, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mConfig.Get_ContCellRounding());
                sizer->Add(spin, BasicSizerFlags());
            });

            NamedVBoxStack(this, sizer, "Text Padding", [this](auto& sizer) {
                auto spin = new wxSpinCtrl(this, SPIN_Text_Padding, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mConfig.Get_ContCellTextPadding());
                sizer->Add(spin, BasicSizerFlags());
            });

            NamedVBoxStack(this, sizer, "Box Padding", [this](auto& sizer) {
                auto spin = new wxSpinCtrl(this, SPIN_Box_Padding, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 10, mConfig.Get_ContCellBoxPadding());
                sizer->Add(spin, BasicSizerFlags());
            });
        });

        auto canvas = new ContinuityBrowserPanel(SYMBOL_PLAIN, mConfig, this);
        sizer->Add(canvas, 1, wxEXPAND);
        auto basic_cont = CalChart::Continuity{ "ewns np\nX = distfrom(sp r2)\nmt (24-X)w\nmarch gv dist(np) dir(np) w\nmtrm e" };
        auto clonedOut = do_cloning(basic_cont);
        clonedOut.emplace_back(std::make_unique<CalChart::ContProcMT>(std::make_unique<CalChart::ContValueUnset>(), std::make_unique<CalChart::ContValueUnset>()));
        canvas->DoSetContinuity(CalChart::Continuity{ std::move(clonedOut) });
    }));

    TransferDataToWindow();
}

void ContCellSetup::Init()
{
    // first read out the defaults:
    for (auto i = 0; i < COLOR_CONTCELLS_NUM; ++i) {
        auto brushAndPen = mConfig.Get_ContCellBrushAndPen(static_cast<ContCellColors>(i));
        mContCellBrushes[i] = brushAndPen.first;
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

    for (ContCellColors i = COLOR_CONTCELLS_PROC; i < COLOR_CONTCELLS_NUM; i = static_cast<ContCellColors>(static_cast<int>(i) + 1)) {
        SetColor(i, mConfig.GetContCellDefaultColors()[i]);
        mConfig.Clear_ContCellConfigColor(i);
    }
    Init();
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

void ContCellSetup::SetColor(int selection, const wxColour& color)
{
    auto pen = *wxThePenList->FindOrCreatePen(color, 1, wxPENSTYLE_SOLID);
    mContCellBrushes[selection] = *wxTheBrushList->FindOrCreateBrush(color, wxBRUSHSTYLE_SOLID);

    mConfig.Set_ContCellBrushAndPen(static_cast<ContCellColors>(selection), mContCellBrushes[selection], pen);

    // update the namebox list
    CreateAndSetItemBitmap(mNameBox, selection, mContCellBrushes[selection]);
    Refresh();
}

void ContCellSetup::OnCmdSelectColors()
{
    auto selection = mNameBox->GetSelection();
    auto data = wxColourData{};
    data.SetChooseFull(true);
    data.SetColour(mContCellBrushes[selection].GetColour());
    auto dialog = wxColourDialog(this, &data);
    if (dialog.ShowModal() == wxID_OK) {
        auto retdata = dialog.GetColourData();
        auto c = retdata.GetColour();
        SetColor(selection, c);
    }
    Refresh();
}

void ContCellSetup::OnCmdResetColors()
{
    auto selection = mNameBox->GetSelection();
    SetColor(selection, mConfig.GetContCellDefaultColors()[selection]);
    mConfig.Clear_ContCellConfigColor(static_cast<ContCellColors>(selection));
    Refresh();
}

void ContCellSetup::OnCmdChooseNewColor(wxCommandEvent&)
{
    Refresh();
}
