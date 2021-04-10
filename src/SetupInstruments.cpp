/*
 * SetupInstruments.cpp
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

#include "SetupInstruments.h"
#include "CalChartToolBar.h"
#include "basic_ui.h"
#include "cc_sheet.h"
#include <algorithm>
#include <ctype.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

enum {
    SetupInstruments_SetupInstrumentsList = 1000,
    SetupInstruments_SetInstrumentChoice,
    SetupInstruments_SelectInstrument,
};

BEGIN_EVENT_TABLE(SetupInstruments, wxDialog)
EVT_LISTBOX(SetupInstruments_SetupInstrumentsList, SetupInstruments::Select)
EVT_LISTBOX_DCLICK(SetupInstruments_SetupInstrumentsList, SetupInstruments::SelectAll)
END_EVENT_TABLE()

IMPLEMENT_CLASS(SetupInstruments, wxDialog)

// how does SetupInstruments work?
// First we read the list of instruments currently set on a show
// we then supplement the list of instruments with the default list
// we have.
//
// When no points are selected we set the choice as the unselected.
// When multiple points are selected with multiple instruments, we
// add the selection kMultiple

SetupInstruments::SetupInstruments(CalChartDoc const& shw, wxWindow* parent, wxWindowID id,
    const wxString& caption, const wxPoint& pos,
    const wxSize& size, long style)
    : mShow(shw)
{
    auto selection = mShow.GetSelectionList();
    auto dotSymbols = mShow.GetCurrentSheet()->GetSymbols();
    mDotIndices.assign(selection.begin(), selection.end());
    // if nothing is selected, then everything is selected.
    if (selection.size() == 0) {
        mDotIndices.resize(shw.GetNumPoints());
        std::iota(mDotIndices.begin(), mDotIndices.end(), 0);
    }

    std::transform(mDotIndices.begin(), mDotIndices.end(), std::back_inserter(mLabels), [&shw](auto&& i) {
        return shw.GetPointLabel(i);
    });

    std::transform(mDotIndices.begin(), mDotIndices.end(), std::back_inserter(mInstruments), [&shw](auto&& i) {
        return shw.GetPointInstrument(i);
    });
    std::transform(mDotIndices.begin(), mDotIndices.end(), std::back_inserter(mSymbols), [&shw, dotSymbols](auto&& i) {
        return dotSymbols.at(i);
    });

    // we construct a new list of instrument choices.
    std::set<std::string> currentInstruments(mInstruments.begin(), mInstruments.end());
    mInstrumentChoices.assign(currentInstruments.begin(), currentInstruments.end());
    for (auto& i : kInstruments) {
        if (currentInstruments.count(i) == 0) {
            mInstrumentChoices.push_back(i);
        }
    }

    Create(parent, id, caption, pos, size, style);

    SelectAllPoints();
}

bool SetupInstruments::Create(wxWindow* parent, wxWindowID id,
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

    // now populate
    wxListBox* list = static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList));
    std::vector<wxString> tlabel;
    std::transform(mLabels.begin(), mLabels.end(), std::back_inserter(tlabel), [](auto&& i) { return i; });
    list->Set(tlabel);

    Center();

    return true;
}

void SetupInstruments::CreateControls()
{
    SetSizer(VStack([this](auto sizer) {
        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&All", [this]() {
                SelectAllPoints();
            });
            CreateButtonWithHandler(this, sizer, BasicSizerFlags(), "&None", [this]() {
                SelectNone();
            });
        });

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            auto counter = 0;
            for (auto&& i : GetSymbolsBitmap()) {
                auto which = static_cast<SYMBOL_TYPE>(counter++);
                if (std::count(mSymbols.begin(), mSymbols.end(), which)) {
                    CreateBitmapButtonWithHandler(this, sizer, BasicSizerFlags(), i, [this, which]() {
                        SelectSymbol(which);
                    });
                }
            }
        });

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateText(this, sizer, BasicSizerFlags(), "Select Instrument");
            auto currentInstruments = std::set(mInstruments.begin(), mInstruments.end());
            auto choices = std::vector<wxString>(currentInstruments.begin(), currentInstruments.end());
            auto choice = CreateChoiceWithHandler(this, sizer, BasicSizerFlags(), SetupInstruments_SelectInstrument, choices, [this](wxCommandEvent& e) {
                SelectInstrument();
            });
            choice->SetSelection(wxNOT_FOUND);
        });

        CreateHLine(this, sizer);

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            auto list = new wxListBox(this, SetupInstruments_SetupInstrumentsList, wxDefaultPosition, wxSize(50, 200), 0, NULL, wxLB_EXTENDED);
            sizer->Add(list, wxSizerFlags(0).Border(wxALL, 5).Center());
            VStack(sizer, BasicSizerFlags(), [this](auto sizer) {
                CreateText(this, sizer, BasicSizerFlags(), "Set Instrument");
                std::vector<wxString> choices;
                choices.insert(choices.end(), mInstrumentChoices.begin(), mInstrumentChoices.end());
                choices.push_back(kCustom);
                auto choice = CreateChoiceWithHandler(this, sizer, BasicSizerFlags(), SetupInstruments_SetInstrumentChoice, choices, [this](wxCommandEvent& e) {
                    OnCmdChoice();
                });
            });
        });

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            CreateButton(this, sizer, BasicSizerFlags(), wxID_CANCEL);
            CreateButton(this, sizer, BasicSizerFlags(), wxID_OK);
        });
    }));
}

std::map<int, std::string> SetupInstruments::GetInstruments() const
{
    auto result = std::vector<std::pair<int, std::string>>{};
    std::transform(mDotIndices.begin(), mDotIndices.end(), mInstruments.begin(), std::back_inserter(result), [](auto&& a, auto&& b) -> std::pair<int, std::string> { return { a, b }; });
    return { result.begin(), result.end() };
}

// the list of selected dots has changed, update the controls
// Specifically, set the instrument selector to not found,
// If nothing is selected, set the set instrument choice to not found
// if 1 instrument selected, set the instrument choice to that
// if more than 1 instrument is selected, add a "
void SetupInstruments::SelectionListChanged()
{
    static_cast<wxChoice*>(FindWindow(SetupInstruments_SelectInstrument))->SetSelection(wxNOT_FOUND);

    auto list = static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList));
    auto choice = static_cast<wxChoice*>(FindWindow(SetupInstruments_SetInstrumentChoice));

    wxArrayInt selections;
    auto n = list->GetSelections(selections);
    // if nothing is selected, remove the choice
    if (n == 0) {
        choice->SetSelection(wxNOT_FOUND);
        return;
    }

    std::vector<std::string> totalList;
    std::transform(selections.begin(), selections.end(), std::back_inserter(totalList), [this](auto& i) { return mInstruments[i]; });
    std::sort(totalList.begin(), totalList.end());
    totalList.erase(std::unique(totalList.begin(), totalList.end()), totalList.end());
    if (totalList.size() == 1) {
        std::vector<wxString> choices;
        choices.insert(choices.end(), mInstrumentChoices.begin(), mInstrumentChoices.end());
        choices.push_back(kCustom);
        choice->Set(choices);
        choice->SetSelection(choice->FindString(totalList[0]));
    } else {
        std::vector<wxString> choices;
        choices.push_back(kMultiple);
        choices.insert(choices.end(), mInstrumentChoices.begin(), mInstrumentChoices.end());
        choices.push_back(kCustom);
        choice->Set(choices);
        choice->SetSelection(0);
    }
}

void SetupInstruments::SelectAll(wxCommandEvent&)
{
    SelectAllPoints();
}

void SetupInstruments::SelectAllPoints()
{
    auto list = static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList));
    for (auto i = 0; i < list->GetCount(); ++i) {
        list->SetSelection(i);
    }
    SelectionListChanged();
}

void SetupInstruments::Select(wxCommandEvent&)
{
    SelectionListChanged();
}

void SetupInstruments::SelectNone()
{
    static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList))->DeselectAll();
    SelectionListChanged();
}

void SetupInstruments::SelectSymbol(SYMBOL_TYPE sym)
{
    auto list = static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList));
    list->DeselectAll();
    for (auto i = 0ul; i < mSymbols.size(); ++i) {
        if (mSymbols[i] == sym) {
            list->SetSelection(i);
        }
    }
    SelectionListChanged();
}

void SetupInstruments::SelectInstrument()
{
    // from the current select, mark all the instruments that match
    auto choice = static_cast<wxChoice*>(FindWindow(SetupInstruments_SelectInstrument));
    auto result = choice->GetString(choice->GetSelection());

    auto list = static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList));
    list->DeselectAll();
    wxArrayInt selections;
    for (auto i = 0ul; i < mInstruments.size(); ++i) {
        if (result == mInstruments[i]) {
            list->SetSelection(i);
        }
    }
    SelectionListChanged();
}

// now we set the instruments to be different
void SetupInstruments::OnCmdChoice()
{
    auto list = static_cast<wxListBox*>(FindWindow(SetupInstruments_SetupInstrumentsList));
    auto choice = static_cast<wxChoice*>(FindWindow(SetupInstruments_SetInstrumentChoice));
    wxArrayInt selections;
    auto n = list->GetSelections(selections);
    if (n == 0) {
        choice->SetSelection(wxNOT_FOUND);
        return;
    }
    auto result = choice->GetString(choice->GetSelection());
    if (result == kMultiple) {
        return;
    }

    if (result == kCustom) {
        auto newInstrument = wxGetTextFromUser("Enter Custom Instrument", "Setup Instrument", "", this);
        if (newInstrument != "") {
            // we have a new instrument.
            result = newInstrument;
            mInstrumentChoices.push_back(result);
        }
    }

    for (auto& i : selections) {
        mInstruments[i] = result;
    }
    // update the list of choices for the instrument selector and set to
    {
        auto choice = static_cast<wxChoice*>(FindWindow(SetupInstruments_SelectInstrument));
        auto currentInstruments = std::set(mInstruments.begin(), mInstruments.end());
        auto choices = std::vector<wxString>(currentInstruments.begin(), currentInstruments.end());
        choice->Set(choices);
        choice->SetSelection(wxNOT_FOUND);
    }
    // now remove the multiple from the list.
    {
        std::vector<wxString> choices;
        choices.insert(choices.end(), mInstrumentChoices.begin(), mInstrumentChoices.end());
        choices.push_back(kCustom);
        choice->Set(choices);
        choice->SetSelection(choice->FindString(result));
    }
}
