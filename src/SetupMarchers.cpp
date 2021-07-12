/*
 * SetupMarchers.cpp
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

#include "SetupMarchers.h"
#include "CalChartDoc.h"
#include "CalChartSheet.h"
#include "CalChartToolBar.h"
#include "basic_ui.h"
#include <algorithm>
#include <ctype.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

static constexpr auto kMaxPoints = 1000;

// returns [letters, use_letters, maxnum]
static std::tuple<std::set<unsigned>, bool, long> CalculateLabels(std::vector<std::string> labels);

enum {
    SetupMarchers_ID_POINTS_SPIN = 1000,
    SetupMarchers_ID_COLUMNS_SPIN,
    SetupMarchers_ID_LABEL_TYPE,
    SetupMarchers_ID_POINTS_PER_LETTER,
    SetupMarchers_ID_LABEL_LETTERS,
};

BEGIN_EVENT_TABLE(SetupMarchers, wxDialog)
EVT_BUTTON(wxID_RESET, SetupMarchers::OnReset)
EVT_CHOICE(SetupMarchers_ID_LABEL_TYPE, SetupMarchers::OnCmd_label_type)
END_EVENT_TABLE()

IMPLEMENT_CLASS(SetupMarchers, wxDialog)

SetupMarchers::SetupMarchers(CalChartDoc& shw, wxWindow* parent)
    : super(parent, wxID_ANY, "Setup Marchers", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
    , mNumberColumns(8)
    , mShow(shw)
{
    CreateControls();

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);

    Center();
}

SetupMarchers::~SetupMarchers() { }

static void EnableLetter(wxWindow& window, bool letters)
{
    // on first time, we need to set up values
    window.FindWindow(SetupMarchers_ID_POINTS_PER_LETTER)->Enable(letters);
    window.FindWindow(SetupMarchers_ID_LABEL_LETTERS)->Enable(letters);
}

// Layout is to give a consistent show layout to both wizard and dialog
// layout requires a parent
//  wxSlider *lettersize;
static void LayoutShowInfo(wxWindow* parent, bool putLastRowButtons)
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
                    auto numPoints = new wxSpinCtrl(parent, SetupMarchers_ID_POINTS_SPIN, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 1, kMaxPoints, 10);
                    sizer->Add(numPoints, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                });

                HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                    CreateText(parent, sizer, "&Columns:");
                    auto numberColumns = new wxSpinCtrl(parent, SetupMarchers_ID_COLUMNS_SPIN, wxEmptyString, wxDefaultPosition, wxSize(60, -1), wxSP_ARROW_KEYS, 0, kMaxPoints, 10);
                    sizer->Add(numberColumns, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                });

                CreateChoiceWithHandler(parent, sizer, BasicSizerFlags(), SetupMarchers_ID_LABEL_TYPE, { wxT("Numbers"), wxT("Letters") }, [parent](auto& event) {
                    EnableLetter(*parent, event.GetInt() == 1);
                    parent->Refresh();
                });

                HStack(sizer, BasicSizerFlags(), [parent](auto sizer) {
                    CreateText(parent, sizer, "P&oints per letter");
                    auto lettersize = new wxSpinCtrl(parent, SetupMarchers_ID_POINTS_PER_LETTER, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 99, 10);
                    sizer->Add(lettersize, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
                });
            });

            VStack(sizer, [parent](auto sizer) {
                // now add right side
                NamedHBoxStack(parent, sizer, "&Letters", [parent](auto sizer) {
                    auto labels = new wxListBox(parent, SetupMarchers_ID_LABEL_LETTERS, wxDefaultPosition, wxDefaultSize, 0, NULL, wxLB_EXTENDED);
                    for (auto i = 0; i < 26; ++i) {
                        wxString buf(static_cast<char>('A' + i));
                        buf += wxString(static_cast<char>('A' + i));
                        labels->InsertItems(1, &buf, i);
                    }
                    for (auto i = 0; i < 26; ++i) {
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

static auto GenNumberLabels(int num)
{
    std::vector<std::pair<std::string, std::string>> results;
    for (auto i = 0; i < num; ++i) {
        results.push_back({ std::to_string(i), kDefaultInstrument });
    }
    return results;
}

static auto NumberLabelsSet(wxListBox const* label_letters)
{
    auto numlabels = 0;
    for (auto i = 0u; i < label_letters->GetCount(); i++) {
        if (label_letters->IsSelected(i)) {
            ++numlabels;
        }
    }
    return numlabels;
}

static auto GenLetterLabels(int numPerLetter, int num, wxListBox const* label_letters)
{
    std::vector<std::pair<std::string, std::string>> results;
    // Letters
    if (NumberLabelsSet(label_letters)) {
        auto letr = 0;
        while (num > 0) {
            if (label_letters->IsSelected(letr)) {
                auto n = std::min(num, numPerLetter);
                for (auto i = 0; i < n; ++i) {
                    results.push_back({ label_letters->GetString(letr) + std::to_string(i), kDefaultInstrument });
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
                results.push_back({ buffer, "" });
            }
            num -= n;
            ++letr;
        }
    }
    return results;
}

// Validate is to give verify show layout to both wizard and dialog
static auto ValidateInfo(wxWindow* parent)
{
    auto labelType = static_cast<wxChoice*>(parent->FindWindow(SetupMarchers_ID_LABEL_TYPE));
    if (labelType->GetSelection() == 0) {
        return true;
    }

    auto num = static_cast<wxSpinCtrl*>(parent->FindWindow(SetupMarchers_ID_POINTS_PER_LETTER))->GetValue();
    auto numlabels = NumberLabelsSet(static_cast<wxListBox*>(parent->FindWindow(SetupMarchers_ID_LABEL_LETTERS)));
    auto pointsCtrl = static_cast<wxSpinCtrl*>(parent->FindWindow(SetupMarchers_ID_POINTS_SPIN));
    auto canDo = (numlabels == 0) || (num * numlabels >= pointsCtrl->GetValue());
    if (!canDo) {
        wxMessageBox("There are not enough labels defined.", "Set labels");
    }
    return canDo;
}

void SetupMarchers::CreateControls()
{
    LayoutShowInfo(this, true);
}

bool SetupMarchers::TransferDataToWindow()
{
    auto [letters, use_letters, maxnum] = CalculateLabels(mShow.GetPointsLabel());

    auto pointsCtrl = static_cast<wxSpinCtrl*>(FindWindow(SetupMarchers_ID_POINTS_SPIN));
    auto columnsCtrl = static_cast<wxSpinCtrl*>(FindWindow(SetupMarchers_ID_COLUMNS_SPIN));
    auto labelType = static_cast<wxChoice*>(FindWindow(SetupMarchers_ID_LABEL_TYPE));
    auto pointsPerLine = static_cast<wxSpinCtrl*>(FindWindow(SetupMarchers_ID_POINTS_PER_LETTER));
    auto label_letters = static_cast<wxListBox*>(FindWindow(SetupMarchers_ID_LABEL_LETTERS));

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

bool SetupMarchers::TransferDataFromWindow()
{
    auto pointsCtrl = static_cast<wxSpinCtrl const*>(FindWindow(SetupMarchers_ID_POINTS_SPIN));
    auto columnsCtrl = static_cast<wxSpinCtrl const*>(FindWindow(SetupMarchers_ID_COLUMNS_SPIN));
    auto labelType = static_cast<wxChoice const*>(FindWindow(SetupMarchers_ID_LABEL_TYPE));
    auto pointsPerLine = static_cast<wxSpinCtrl const*>(FindWindow(SetupMarchers_ID_POINTS_PER_LETTER));
    auto label_letters = static_cast<wxListBox const*>(FindWindow(SetupMarchers_ID_LABEL_LETTERS));

    auto numberPoints = pointsCtrl->GetValue();
    mNumberColumns = columnsCtrl->GetValue();

    if (labelType->GetSelection() == 0) {
        mLabelsAndInstruments = GenNumberLabels(numberPoints);
    } else {
        // Letters
        mLabelsAndInstruments = GenLetterLabels(pointsPerLine->GetValue(), numberPoints, label_letters);
    }
    return true;
}

bool SetupMarchers::Validate() { return ValidateInfo(this); }

void SetupMarchers::OnReset(wxCommandEvent&) { TransferDataToWindow(); }

void SetupMarchers::OnCmd_label_type(wxCommandEvent& event)
{
    EnableLetter(*this, event.GetInt() == 1);
    Refresh();
}

// common utilities
static std::tuple<std::set<unsigned>, bool, long> CalculateLabels(std::vector<std::string> labels)
{
    auto letters = std::set<unsigned>{};
    auto use_letters = false;
    auto maxnum = 1;
    for (auto tmp : labels) {
        auto letterIndex = 0;
        if (!isdigit(tmp[0])) {
            use_letters = true;
            letterIndex += tmp[0] - 'A';
            tmp.erase(0, 1);
            if (!isdigit(tmp[0])) {
                tmp.erase(0, 1);
                letterIndex += 26;
            }
        }
        maxnum = std::max<long>(maxnum, std::stol(tmp) + 1);
        if (use_letters) {
            letters.insert(letterIndex);
        }
    }
    if (use_letters == false) {
        maxnum = 10;
    }
    return { letters, use_letters, maxnum };
}

IMPLEMENT_CLASS(SetupMarchersWizard, wxWizardPageSimple)

BEGIN_EVENT_TABLE(SetupMarchersWizard, wxWizardPageSimple)
EVT_CHOICE(SetupMarchers_ID_LABEL_TYPE, SetupMarchersWizard::OnCmd_label_type)
END_EVENT_TABLE()

SetupMarchersWizard::SetupMarchersWizard(wxWizard* parent)
    : wxWizardPageSimple(parent)
    , mTransferDataToWindowFirstTime(true)
    , mNumberColumns(8)
{
    LayoutShowInfo(this, false);
    GetSizer()->Fit(this);
}

bool SetupMarchersWizard::TransferDataToWindow()
{
    // on first time, we need to set up values
    if (mTransferDataToWindowFirstTime) {
        auto pointsCtrl = static_cast<wxSpinCtrl*>(FindWindow(SetupMarchers_ID_POINTS_SPIN));
        auto columnsCtrl = static_cast<wxSpinCtrl*>(FindWindow(SetupMarchers_ID_COLUMNS_SPIN));
        auto labelType = static_cast<wxChoice*>(FindWindow(SetupMarchers_ID_LABEL_TYPE));
        auto pointsPerLine = static_cast<wxSpinCtrl*>(FindWindow(SetupMarchers_ID_POINTS_PER_LETTER));
        auto label_letters = static_cast<wxListBox*>(FindWindow(SetupMarchers_ID_LABEL_LETTERS));

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

bool SetupMarchersWizard::TransferDataFromWindow()
{
    auto pointsCtrl = static_cast<wxSpinCtrl const*>(FindWindow(SetupMarchers_ID_POINTS_SPIN));
    auto columnsCtrl = static_cast<wxSpinCtrl const*>(FindWindow(SetupMarchers_ID_COLUMNS_SPIN));
    auto labelType = static_cast<wxChoice const*>(FindWindow(SetupMarchers_ID_LABEL_TYPE));
    auto pointsPerLine = static_cast<wxSpinCtrl const*>(FindWindow(SetupMarchers_ID_POINTS_PER_LETTER));
    auto label_letters = static_cast<wxListBox const*>(FindWindow(SetupMarchers_ID_LABEL_LETTERS));

    auto numberPoints = pointsCtrl->GetValue();
    mNumberColumns = columnsCtrl->GetValue();

    if (labelType->GetSelection() == 0) {
        mLabelsAndInstruments = GenNumberLabels(numberPoints);
    } else {
        // Letters
        mLabelsAndInstruments = GenLetterLabels(pointsPerLine->GetValue(), numberPoints, label_letters);
    }
    return true;
}

bool SetupMarchersWizard::Validate() { return ValidateInfo(this); }

void SetupMarchersWizard::OnCmd_label_type(wxCommandEvent& event)
{
    EnableLetter(*this, event.GetInt() == 1);
    Refresh();
}
