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

#include "show_ui.h"
#include "CalChartToolBar.h"
#include "basic_ui.h"
#include "cc_sheet.h"
#include <algorithm>
#include <ctype.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

static const size_t kMaxPoints = 1000;

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
    , mView(std::make_unique<PointPickerView>())
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

static void CalculateLabels(const CalChartDoc& show,
    std::set<unsigned>& letters, bool& use_letters,
    long& maxnum)
{
    use_letters = false;
    maxnum = 1;
    for (auto i = 0; i < show.GetNumPoints(); ++i) {
        wxString tmp = show.GetPointLabel(i);
        unsigned letterIndex = 0;
        if (!isdigit(tmp[0])) {
            use_letters = true;
            letterIndex += tmp[0] - 'A';
            tmp.Remove(0, 1);
            if (!isdigit(tmp[0])) {
                tmp.Remove(0, 1);
                letterIndex += 26;
            }
        }
        long num = 0;
        if (tmp.ToLong(&num)) {
            maxnum = std::max(maxnum, num + 1);
        }
        if (use_letters) {
            letters.insert(letterIndex);
        }
    }
    if (use_letters == false) {
        maxnum = 10;
    }
}

enum {
    ShowInfoReq_ID_POINTS_SPIN = 1000,
    ShowInfoReq_ID_COLUMNS_SPIN,
    ShowInfoReq_ID_LABEL_TYPE,
    ShowInfoReq_ID_POINTS_PER_LETTER,
    ShowInfoReq_ID_LABEL_LETTERS,
    ShowInfoReq_ID_RESET,
};

BEGIN_EVENT_TABLE(ShowInfoReq, wxDialog)
EVT_BUTTON(wxID_RESET, ShowInfoReq::OnReset)
EVT_CHOICE(ShowInfoReq_ID_LABEL_TYPE, ShowInfoReq::OnCmd_label_type)
END_EVENT_TABLE()

IMPLEMENT_CLASS(ShowInfoReq, wxDialog)

ShowInfoReq::ShowInfoReq(CalChartDoc& shw, wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
    : mNumberColumns(8)
    , mShow(shw)
{
    Create(parent, id, caption, pos, size, style);
}

ShowInfoReq::~ShowInfoReq() { }

bool ShowInfoReq::Create(wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
{
    if (!wxDialog::Create(parent, id, caption, pos, size, style))
        return false;

    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);

    Center();

    return true;
}

void EnableLetter(wxWindow& window, bool letters)
{
    // on first time, we need to set up values
    window.FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER)->Enable(letters);
    window.FindWindow(ShowInfoReq_ID_LABEL_LETTERS)->Enable(letters);
}

// Layout is to give a consistent show layout to both wizard and dialog
// layout requires a parent
//  wxSlider *lettersize;
void LayoutShowInfo(wxWindow* parent, bool putLastRowButtons)
{
    parent->SetSizer(VStack([parent, putLastRowButtons](auto sizer) {
        // now we add a left and a right side, putting boxes around things
        // special attention about box sizers
        // "the order in which you create
        // new controls is important. Create your wxStaticBox control before any
        // siblings that are to appear inside the wxStaticBox in order to preserve
        // the correct Z-Order of controls."
        HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
            VStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                // add the left side
                HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                    CreateText(parent, sizer, "&Points:");
                    wxSpinCtrl* numPoints = new wxSpinCtrl(
                        parent, ShowInfoReq_ID_POINTS_SPIN, wxEmptyString, wxDefaultPosition,
                        wxSize(60, -1), wxSP_ARROW_KEYS, 1, kMaxPoints, 10);
                    sizer->Add(numPoints, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                });

                HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                    CreateText(parent, sizer, "&Columns:");
                    wxSpinCtrl* numberColumns = new wxSpinCtrl(
                        parent, ShowInfoReq_ID_COLUMNS_SPIN, wxEmptyString, wxDefaultPosition,
                        wxSize(60, -1), wxSP_ARROW_KEYS, 0, kMaxPoints, 10);
                    sizer->Add(numberColumns, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                });

                auto label_type = CreateChoiceWithHandler(parent, sizer, BasicSizerFlags(), ShowInfoReq_ID_LABEL_TYPE, { wxT("Numbers"), wxT("Letters") }, [parent](auto& event) {
                    EnableLetter(*parent, event.GetInt() == 1);
                    parent->Refresh();
                });

                HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                    CreateText(parent, sizer, "P&oints per letter");
                    wxSpinCtrl* lettersize = new wxSpinCtrl(
                        parent, ShowInfoReq_ID_POINTS_PER_LETTER, wxEmptyString,
                        wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99, 10);
                    sizer->Add(lettersize, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                });
            });

            VStack(sizer, [parent](auto sizer) {
                // now add right side
                NamedHBoxStack(parent, sizer, "&Letters", [parent](auto sizer) {
                    wxListBox* labels = new wxListBox(parent, ShowInfoReq_ID_LABEL_LETTERS, wxDefaultPosition,
                        wxDefaultSize, 0, NULL, wxLB_EXTENDED);
                    for (int i = 0; i < 26; ++i) {
                        wxString buf(static_cast<char>('A' + i));
                        buf += wxString(static_cast<char>('A' + i));
                        labels->InsertItems(1, &buf, i);
                    }
                    for (int i = 0; i < 26; ++i) {
                        wxString buf(static_cast<char>('A' + i));
                        labels->InsertItems(1, &buf, i);
                    }
                    labels->EnsureVisible(0);
                    sizer->Add(labels, 0, wxGROW | wxALL, 0);
                });
            });
        });

        if (putLastRowButtons) {
            // put a line between the controls and the buttons
            CreateHLine(parent, sizer);

            // the buttons on the bottom
            HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                CreateButton(parent, sizer, wxID_RESET, wxT("&Reset"));
                CreateButton(parent, sizer, wxID_OK);
                CreateButton(parent, sizer, wxID_CANCEL);
            });
        }
    }));
}

std::vector<wxString> GenNumberLabels(int num)
{
    std::vector<wxString> results;
    for (auto i = 0; i < num; ++i) {
        // Begin with 1, not 0
        wxString buffer;
        buffer.Printf(wxT("%u"), i);
        results.push_back(buffer);
    }
    return results;
}

auto NumberLabelsSet(wxListBox const* label_letters)
{
    auto numlabels = 0;
    for (auto i = 0u; i < label_letters->GetCount(); i++) {
        if (label_letters->IsSelected(i)) {
            ++numlabels;
        }
    }
    return numlabels;
}

std::vector<wxString> GenLetterLabels(int numPerLetter, int num, wxListBox const* label_letters)
{
    std::vector<wxString> results;
    // Letters
    if (NumberLabelsSet(label_letters)) {
        auto letr = 0;
        while (num > 0) {
            if (label_letters->IsSelected(letr)) {
                int n = std::min(num, numPerLetter);
                for (int i = 0; i < n; ++i) {
                    wxString buffer;
                    buffer.Printf(wxT("%s%u"), label_letters->GetString(letr), i);
                    results.push_back(buffer);
                }
                num -= n;
            }
            ++letr;
        }
    } else {
        auto letr = 0;
        while (num > 0) {
            int n = std::min(num, numPerLetter);
            for (int i = 0; i < n; ++i) {
                wxString buffer;
                if (letr >= 26) {
                    buffer.Printf(wxT("%c%c%u"), 'A' + letr - 26, 'A' + letr - 26, i);
                } else {
                    buffer.Printf(wxT("%c%u"), 'A' + letr, i);
                }
                results.push_back(buffer);
            }
            num -= n;
            ++letr;
        }
    }
    return results;
}

// Validate is to give verify show layout to both wizard and dialog
bool ValidateInfo(wxWindow* parent)
{
    wxChoice* labelType = (wxChoice*)parent->FindWindow(ShowInfoReq_ID_LABEL_TYPE);
    if (labelType->GetSelection() == 0) {
        return true;
    }
    wxSpinCtrl* pointsCtrl = (wxSpinCtrl*)parent->FindWindow(ShowInfoReq_ID_POINTS_SPIN);
    wxSpinCtrl* pointsPerLine = (wxSpinCtrl*)parent->FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
    wxListBox* label_letters = (wxListBox*)parent->FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

    int num = pointsPerLine->GetValue();
    int numlabels = NumberLabelsSet(label_letters);
    bool canDo = (numlabels == 0) || (num * numlabels >= pointsCtrl->GetValue());
    if (!canDo)
        wxMessageBox(wxT("There are not enough labels defined."),
            wxT("Set labels"));
    return canDo;
}

void ShowInfoReq::CreateControls() { LayoutShowInfo(this, true); }

bool ShowInfoReq::TransferDataToWindow()
{
    std::set<unsigned> letters;
    bool use_letters;
    long maxnum;
    CalculateLabels(mShow, letters, use_letters, maxnum);

    wxSpinCtrl* pointsCtrl = (wxSpinCtrl*)FindWindow(ShowInfoReq_ID_POINTS_SPIN);
    wxSpinCtrl* columnsCtrl = (wxSpinCtrl*)FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
    wxChoice* labelType = (wxChoice*)FindWindow(ShowInfoReq_ID_LABEL_TYPE);
    wxSpinCtrl* pointsPerLine = (wxSpinCtrl*)FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
    wxListBox* label_letters = (wxListBox*)FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

    pointsCtrl->SetValue(mShow.GetNumPoints());
    columnsCtrl->SetValue(mNumberColumns);
    labelType->SetSelection(use_letters);
    pointsPerLine->SetValue(static_cast<int>(maxnum));
    label_letters->DeselectAll();
    for (unsigned i = 0; i < label_letters->GetCount(); ++i) {
        if (letters.count(i)) {
            label_letters->SetSelection(i);
        }
    }
    EnableLetter(*this, labelType->GetSelection() == 1);
    return true;
}

bool ShowInfoReq::TransferDataFromWindow()
{
    auto pointsCtrl = (wxSpinCtrl const*)FindWindow(ShowInfoReq_ID_POINTS_SPIN);
    auto columnsCtrl = (wxSpinCtrl const*)FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
    auto labelType = (wxChoice const*)FindWindow(ShowInfoReq_ID_LABEL_TYPE);
    auto pointsPerLine = (wxSpinCtrl const*)FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
    auto label_letters = (wxListBox const*)FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

    auto numberPoints = pointsCtrl->GetValue();
    mNumberColumns = columnsCtrl->GetValue();

    if (labelType->GetSelection() == 0) {
        mLabels = GenNumberLabels(numberPoints);
    } else {
        // Letters
        mLabels = GenLetterLabels(pointsPerLine->GetValue(), numberPoints, label_letters);
    }
    return true;
}

bool ShowInfoReq::Validate() { return ValidateInfo(this); }

void ShowInfoReq::OnReset(wxCommandEvent&) { TransferDataToWindow(); }

void ShowInfoReq::OnCmd_label_type(wxCommandEvent& event)
{
    EnableLetter(*this, event.GetInt() == 1);
    Refresh();
}

IMPLEMENT_CLASS(ShowInfoReqWizard, wxWizardPageSimple)

BEGIN_EVENT_TABLE(ShowInfoReqWizard, wxWizardPageSimple)
EVT_CHOICE(ShowInfoReq_ID_LABEL_TYPE, ShowInfoReqWizard::OnCmd_label_type)
END_EVENT_TABLE()

ShowInfoReqWizard::ShowInfoReqWizard(wxWizard* parent)
    : wxWizardPageSimple(parent)
    , mTransferDataToWindowFirstTime(true)
    , mNumberColumns(8)
{
    LayoutShowInfo(this, false);
    GetSizer()->Fit(this);
}

bool ShowInfoReqWizard::TransferDataToWindow()
{
    // on first time, we need to set up values
    if (mTransferDataToWindowFirstTime) {
        wxSpinCtrl* pointsCtrl = (wxSpinCtrl*)FindWindow(ShowInfoReq_ID_POINTS_SPIN);
        wxSpinCtrl* columnsCtrl = (wxSpinCtrl*)FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
        wxChoice* labelType = (wxChoice*)FindWindow(ShowInfoReq_ID_LABEL_TYPE);
        wxSpinCtrl* pointsPerLine = (wxSpinCtrl*)FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
        wxListBox* label_letters = (wxListBox*)FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

        pointsCtrl->SetValue(1);
        columnsCtrl->SetValue(mNumberColumns);
        labelType->SetSelection(false);
        pointsPerLine->SetValue(10);
        label_letters->DeselectAll();
        EnableLetter(*this, labelType->GetSelection() == 1);
        mTransferDataToWindowFirstTime = false;
    }
    return true;
}

bool ShowInfoReqWizard::TransferDataFromWindow()
{
    auto pointsCtrl = (wxSpinCtrl const*)FindWindow(ShowInfoReq_ID_POINTS_SPIN);
    auto columnsCtrl = (wxSpinCtrl const*)FindWindow(ShowInfoReq_ID_COLUMNS_SPIN);
    auto labelType = (wxChoice const*)FindWindow(ShowInfoReq_ID_LABEL_TYPE);
    auto pointsPerLine = (wxSpinCtrl const*)FindWindow(ShowInfoReq_ID_POINTS_PER_LETTER);
    auto label_letters = (wxListBox const*)FindWindow(ShowInfoReq_ID_LABEL_LETTERS);

    auto numberPoints = pointsCtrl->GetValue();
    mNumberColumns = columnsCtrl->GetValue();

    if (labelType->GetSelection() == 0) {
        mLabels = GenNumberLabels(numberPoints);
    } else {
        // Letters
        mLabels = GenLetterLabels(pointsPerLine->GetValue(), numberPoints, label_letters);
    }
    return true;
}

bool ShowInfoReqWizard::Validate() { return ValidateInfo(this); }

void ShowInfoReqWizard::OnCmd_label_type(wxCommandEvent& event)
{
    EnableLetter(*this, event.GetInt() == 1);
    Refresh();
}
