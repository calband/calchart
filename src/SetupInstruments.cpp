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
#include "CalChartSheet.h"
#include "CalChartToolBar.h"
#include "basic_ui.h"
#include <algorithm>
#include <ctype.h>
#include <wx/spinctrl.h>
#include <wx/statline.h>

enum {
    SetupInstruments_SetupInstrumentsList = 1000,
};

static constexpr auto kInstruments = {
    kDefaultInstrument, // this is the same as a blank
    "Picc",
    "Clarinet",
    "Alto Sax",
    "Tenor Sax",
    "Trumpet",
    "Mello",
    "Trombone",
    "Baritone",
    "Sousaphone",
    "Snare",
    "Bass Drum",
    "Quad Drum",
    "Cymbals",
    "Glock",
    "Perc",
    "Drum Major",
};

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
    std::transform(mDotIndices.begin(), mDotIndices.end(), std::back_inserter(mSymbols), [dotSymbols](auto&& i) {
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

template <std::ranges::input_range R>
auto enumerate(R&& range)
{
    using std::begin, std::end;
    using iterator_t = decltype(begin(range));
    using index_t = std::make_signed_t<std::ranges::range_difference_t<R>>;

    struct iterator {
        using difference_type = index_t;
        using value_type = std::pair<index_t, std::ranges::range_value_t<R>>;

        iterator_t iter;
        index_t index;

        decltype(auto) operator*() const
        {
            return std::pair{ index, *iter };
        }

        iterator& operator++()
        {
            ++iter;
            ++index;
            return *this;
        }

        iterator operator++(int)
        {
            auto old = *this;
            ++iter;
            ++index;
            return old;
        }
        bool operator==(const iterator& other) const
        {
            return iter == other.iter;
        }
        bool operator!=(const iterator& other) const
        {
            return iter != other.iter;
        }
    };

    return std::ranges::subrange(iterator{ begin(range), 0 }, iterator{ end(range), 0 });
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
    std::vector<wxString> tlabel;
    std::transform(mLabels.begin(), mLabels.end(), std::back_inserter(tlabel), [](auto&& i) { return i; });
    mSetupInstrumentList->Set(tlabel);

    Center();

    return true;
}

void SetupInstruments::CreateControls()
{
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::HSizer{
            wxUI::Button{ "&All" }
                .bind([this] { SelectAllPoints(); }),
            wxUI::Button{ "&None" }
                .bind([this] { SelectNone(); }),
        },
        wxUI::HSizer{
            wxUI::ForEach{
                enumerate(GetSymbolsBitmap()) | std::views::filter([this](auto bitmap) {
                    return std::count(mSymbols.begin(), mSymbols.end(), static_cast<CalChart::SYMBOL_TYPE>(std::get<0>(bitmap)));
                }),
                [this](auto bitmap) {
                    return wxUI::BitmapButton{ std::get<1>(bitmap) }
                        .bind([this, which = static_cast<CalChart::SYMBOL_TYPE>(std::get<0>(bitmap))] { SelectSymbol(which); });
                } },
        },
        wxUI::HSizer{
            wxUI::Text{ "Select Instrument" },
            wxUI::Choice{ std::set(mInstruments.begin(), mInstruments.end()) }
                .withSelection(wxNOT_FOUND)
                .bind([this] { SelectInstrument(); })
                .withProxy(mSelectInstrument),
        },
        wxUI::HLine(),
        wxUI::HSizer{
            wxUI::ListBox{ SetupInstruments_SetupInstrumentsList }
                .setStyle(wxLB_EXTENDED)
                .withSize({ 50, 200 })
                .bind(wxEVT_LISTBOX, [this] { Select(); })
                .bind(wxEVT_LISTBOX_DCLICK, [this] { SelectAll(); })
                .withProxy(mSetupInstrumentList),
            wxUI::VSizer{
                wxUI::Text{ "Set Instrument" },
                wxUI::Choice{ mInstrumentChoices }
                    .bind([this] { OnCmdChoice(); })
                    .withProxy(mInstrumentChoice),
            },
        },
        wxUI::HSizer{
            wxUI::Button{ wxID_CANCEL },
            wxUI::Button{ wxID_OK },
        },
    }
        .fitTo(this);
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
    mSelectInstrument->SetSelection(wxNOT_FOUND);

    wxArrayInt selections;
    auto n = mSetupInstrumentList->GetSelections(selections);
    // if nothing is selected, remove the choice
    if (n == 0) {
        mInstrumentChoice->SetSelection(wxNOT_FOUND);
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
        mInstrumentChoice->Set(choices);
        mInstrumentChoice->SetSelection(mInstrumentChoice->FindString(totalList[0]));
    } else {
        std::vector<wxString> choices;
        choices.push_back(kMultiple);
        choices.insert(choices.end(), mInstrumentChoices.begin(), mInstrumentChoices.end());
        choices.push_back(kCustom);
        mInstrumentChoice->Set(choices);
        mInstrumentChoice->SetSelection(0);
    }
}

void SetupInstruments::SelectAll()
{
    SelectAllPoints();
}

void SetupInstruments::SelectAllPoints()
{
    for (auto i = 0ul; i < mSetupInstrumentList->GetCount(); ++i) {
        mSetupInstrumentList->SetSelection(i);
    }
    SelectionListChanged();
}

void SetupInstruments::Select()
{
    SelectionListChanged();
}

void SetupInstruments::SelectNone()
{
    mSetupInstrumentList->DeselectAll();
    SelectionListChanged();
}

void SetupInstruments::SelectSymbol(CalChart::SYMBOL_TYPE sym)
{
    mSetupInstrumentList->DeselectAll();
    for (auto i = 0ul; i < mSymbols.size(); ++i) {
        if (mSymbols[i] == sym) {
            mSetupInstrumentList->SetSelection(i);
        }
    }
    SelectionListChanged();
}

void SetupInstruments::SelectInstrument()
{
    // from the current select, mark all the instruments that match
    auto result = mSelectInstrument->GetString(mSelectInstrument->GetSelection());

    mSetupInstrumentList->DeselectAll();
    wxArrayInt selections;
    for (auto i = 0ul; i < mInstruments.size(); ++i) {
        if (result == mInstruments[i]) {
            mSetupInstrumentList->SetSelection(i);
        }
    }
    SelectionListChanged();
}

// now we set the instruments to be different
void SetupInstruments::OnCmdChoice()
{
    wxArrayInt selections;
    auto n = mSetupInstrumentList->GetSelections(selections);
    if (n == 0) {
        mInstrumentChoice->SetSelection(wxNOT_FOUND);
        return;
    }
    auto result = mInstrumentChoice->GetString(mInstrumentChoice->GetSelection());
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
        auto currentInstruments = std::set(mInstruments.begin(), mInstruments.end());
        auto choices = std::vector<wxString>(currentInstruments.begin(), currentInstruments.end());
        mSelectInstrument->Set(choices);
        mSelectInstrument->SetSelection(wxNOT_FOUND);
    }
    // now remove the multiple from the list.
    {
        std::vector<wxString> choices;
        choices.insert(choices.end(), mInstrumentChoices.begin(), mInstrumentChoices.end());
        choices.push_back(kCustom);
        mInstrumentChoice->Set(choices);
        mInstrumentChoice->SetSelection(mInstrumentChoice->FindString(result));
    }
}
