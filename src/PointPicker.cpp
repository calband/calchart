/*
 * PointPicker.cpp
 * Dialog for picking points
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

#include "PointPicker.h"
#include "CalChartToolBar.h"
#include "basic_ui.h"

enum {
    PointPicker_PointPickerList = 1100,
};

BEGIN_EVENT_TABLE(PointPicker, wxDialog)
EVT_LISTBOX(PointPicker_PointPickerList, PointPicker::PointPickerSelect)
EVT_LISTBOX_DCLICK(PointPicker_PointPickerList, PointPicker::PointPickerAll)
END_EVENT_TABLE()

PointPicker::PointPicker(CalChartDoc const& shw, wxWindow* parent)
    : super(parent, wxID_ANY, "Select Points", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
    , mShow(shw)
{
    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);

    Center();
}

void PointPicker::CreateControls()
{
    SetSizer(VStack([this](auto sizer) {
        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&All", [this]() {
                mSelection = mShow.MakeSelectAll();
                EndModal(wxID_OK);
            });
            CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&None", [this]() {
                mSelection = mShow.MakeUnselectAll();
                EndModal(wxID_OK);
            });
            auto button = CreateButton(this, sizer, BasicSizerFlags(), wxID_OK);
            button->SetDefault();
        });

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            auto counter = 0;
            for (auto&& i : GetSymbolsBitmap()) {
                auto which = static_cast<CalChart::SYMBOL_TYPE>(counter++);
                if (mShow.MakeSelectBySymbol(which).size()) {
                    CreateBitmapButtonWithHandler(this, sizer, BasicSizerFlags(), i, [this, which]() {
                        mSelection = mShow.MakeSelectBySymbol(which);
                        EndModal(wxID_OK);
                    });
                }
            }
        });

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateText(this, sizer, BasicSizerFlags(), "Select Instrument");

            auto instruments = mShow.GetPointsInstrument();
            auto currentInstruments = std::set(instruments.begin(), instruments.end());
            auto choice = CreateChoiceWithHandler(this, sizer, BasicSizerFlags(), wxID_ANY, { currentInstruments.begin(), currentInstruments.end() }, [this](wxCommandEvent& e) {
                mSelection = mShow.MakeSelectByInstrument(e.GetString());
                EndModal(wxID_OK);
            });
            choice->SetSelection(wxNOT_FOUND);
        });

        mList = new wxListBox(this, PointPicker_PointPickerList, wxDefaultPosition, wxSize(50, 250), 0, NULL, wxLB_EXTENDED);
        sizer->Add(mList, wxSizerFlags(0).Border(wxALL, 5).Center());
    }));

    RereadFromShow();
}

void PointPicker::PointPickerAll(wxCommandEvent&)
{
    mSelection = mShow.MakeSelectAll();
}

void PointPicker::PointPickerSelect(wxCommandEvent&)
{
    wxArrayInt selections;
    mList->GetSelections(selections);
    mSelection = CalChart::SelectionList(selections.begin(), selections.end());
}

void PointPicker::RereadFromShow()
{
    auto tshowLabels = mShow.GetPointsLabel();
    auto showLabels = std::vector<wxString>(tshowLabels.begin(), tshowLabels.end());
    if (mCachedLabels != showLabels) {
        mCachedLabels = showLabels;
        mList->Clear();
        mList->Set(wxArrayString{ mCachedLabels.size(), &mCachedLabels[0] });
    }
    auto showSelectionList = mShow.GetSelectionList();
    if (mSelection != showSelectionList) {
        mList->DeselectAll();
        mSelection = showSelectionList;
        for (auto&& n : mSelection) {
            mList->SetSelection(n);
        }
    }
}
