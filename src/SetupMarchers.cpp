/*
 * SetupMarchers.cpp
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
    auto labels = std::vector<wxString>{};

    for (auto i = 0; i < 26; ++i) {
        wxString buf(static_cast<char>('A' + i));
        buf += wxString(static_cast<char>('A' + i));
        labels.push_back(buf);
    }
    for (auto i = 0; i < 26; ++i) {
        wxString buf(static_cast<char>('A' + i));
        labels.push_back(buf);
    }
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::HSizer{
            wxUI::VSizer{
                wxUI::HSizer{
                    wxUI::Text{ "&Points:" },
                    wxUI::SpinCtrl{ SetupMarchers_ID_POINTS_SPIN, std::pair{ 1, kMaxPoints }, 10 }
                        .withSize({ 60, -1 })
                        .withStyle(wxSP_ARROW_KEYS),
                },
                wxUI::HSizer{
                    wxUI::Text{ "&Columns:" },
                    wxUI::SpinCtrl{ SetupMarchers_ID_COLUMNS_SPIN, std::pair{ 1, kMaxPoints }, 10 }
                        .withStyle(wxSP_ARROW_KEYS),
                },
                wxUI::Choice{ SetupMarchers_ID_LABEL_TYPE, std::vector<wxString>{ wxT("Numbers"), wxT("Letters") } }
                    .bind([parent](auto& e) {
                        EnableLetter(*parent, e.GetInt() == 1);
                        parent->Refresh();
                    }),
                wxUI::HSizer{
                    wxUI::Text{ "P&oints per letter:" },
                    wxUI::SpinCtrl{ SetupMarchers_ID_POINTS_PER_LETTER, std::pair{ 1, 99 }, 10 }
                        .withStyle(wxSP_ARROW_KEYS),
                },

            },
            wxUI::VSizer{
                wxUI::HSizer{
                    "Letters",
                    wxUI::ListBox{ SetupMarchers_ID_LABEL_LETTERS, labels }
                        .withStyle(wxLB_EXTENDED)
                        .withEnsureVisible(0)
                        .withFlags(ExpandSizerFlags()),
                },
            },
        },
        // How do we do this with an wxUI::If?
        wxUI::Custom{
            [putLastRowButtons](wxWindow* window, wxSizer* sizer, wxSizerFlags flags) {
                if (!putLastRowButtons) {
                    return;
                }
                wxUI::HSizer{
                    wxUI::Button{ wxID_RESET, "&Reset" },
                    wxUI::Button{ wxID_OK },
                    wxUI::Button{ wxID_CANCEL },
                }
                    .createAndAdd(window, sizer, flags);
            } },
    }
        .fitTo(parent);
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
