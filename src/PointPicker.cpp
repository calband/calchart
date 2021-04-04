/*
 * show_ui.cpp
 * Classes for interacting with shows
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
#include <algorithm>
#include <ctype.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

class PointPickerView : public wxView {
public:
    PointPickerView() = default;
    ~PointPickerView() = default;
    virtual void OnDraw(wxDC* dc) { }
    virtual void OnUpdate(wxView* sender, wxObject* hint = (wxObject*)NULL);
};

void PointPickerView::OnUpdate(wxView* sender, wxObject* hint)
{
    static_cast<PointPicker*>(GetFrame())->Update();
}

enum {
    PointPicker_PointPickerList = 1100,
};

BEGIN_EVENT_TABLE(PointPicker, wxDialog)
EVT_LISTBOX(PointPicker_PointPickerList, PointPicker::PointPickerSelect)
EVT_LISTBOX_DCLICK(PointPicker_PointPickerList, PointPicker::PointPickerAll)
END_EVENT_TABLE()

PointPicker::PointPicker(CalChartDoc& shw, wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
    : super(parent, id, caption, pos, size, style)
    , mShow(shw)
    , mView(new PointPickerView)
{
    // give this a view so it can pick up document changes
    mView->SetDocument(&mShow);
    mView->SetFrame(this);

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
            auto button = CreateButton(this, sizer, BasicSizerFlags(), wxID_OK, "&Close");
            button->SetDefault();
            CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&All", [this]() {
                mShow.SetSelection(mShow.MakeSelectAll());
            });
            CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&None", [this]() {
                mShow.SetSelection(mShow.MakeUnselectAll());
            });
        });

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            auto counter = 0;
            for (auto&& i : GetSymbolsBitmap()) {
                auto which = static_cast<SYMBOL_TYPE>(counter++);
                CreateBitmapButtonWithHandler(this, sizer, BasicSizerFlags(), i, [this, which]() {
                    mShow.SetSelection(mShow.GetCurrentSheet()->MakeSelectPointsBySymbol(which));
                });
            }
        });

        mList = new wxListBox(this, PointPicker_PointPickerList, wxDefaultPosition,
            wxSize(50, 500), 0, NULL, wxLB_EXTENDED);
        sizer->Add(mList, wxSizerFlags(0).Border(wxALL, 5).Center());
    }));

    Update();
}

void PointPicker::PointPickerAll(wxCommandEvent&)
{
    mShow.SetSelection(mShow.MakeSelectAll());
}

void PointPicker::PointPickerSelect(wxCommandEvent&)
{
    wxArrayInt selections;
    size_t n = mList->GetSelections(selections);

    mCachedSelection.clear();
    for (size_t i = 0; i < n; ++i)
        mCachedSelection.insert(selections[i]);
    mShow.SetSelection(mCachedSelection);
}

void PointPicker::Update()
{
    auto&& tshowLabels = mShow.GetPointLabels();
    std::vector<wxString> showLabels(tshowLabels.begin(), tshowLabels.end());
    if (mCachedLabels != showLabels) {
        mCachedLabels = showLabels;
        mList->Clear();
        mList->Set(wxArrayString{ mCachedLabels.size(), &mCachedLabels[0] });
    }
    auto showSelectionList = mShow.GetSelectionList();
    if (mCachedSelection != showSelectionList) {
        mList->DeselectAll();
        mCachedSelection = showSelectionList;
        for (auto n = mCachedSelection.begin(); n != mCachedSelection.end(); ++n) {
            mList->SetSelection(*n);
        }
    }
}

