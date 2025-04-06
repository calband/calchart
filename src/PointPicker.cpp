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
#include "CalChartRanges.h"
#include "CalChartToolBar.h"
#include <ranges>
#include <wxUI/wxUI.hpp>

PointPicker::PointPicker(wxWindow* parent, CalChartDoc const& show)
    : PointPicker(
        parent,
        show,
        CalChart::SelectionList(std::views::iota(0, show.GetNumPoints()).begin(), std::views::iota(0, show.GetNumPoints()).end()),
        show.GetSelectionList())
{
}

// Given a set of marchers, and a set of selected marchers, create a dialog that allows the user to select
// the labels of the marchers to use.
PointPicker::PointPicker(wxWindow* parent, CalChartDoc const& show, CalChart::SelectionList const& marchersToUse, CalChart::SelectionList const& selected)
    : super(parent, wxID_ANY, "Select Marchers", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
{
    auto labels = show.GetPointsLabel(marchersToUse);
    auto instruments = show.GetPointsInstrument(marchersToUse);
    auto currentInstruments = std::set(instruments.begin(), instruments.end());

    wxUI::ListBox::Proxy mList{};
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2).Center().Proportion(0),
        wxUI::HSizer{
            wxUI::Button{ "&All" }
                .bind([this, mList] {
                    mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings());
                    EndModal(wxID_OK);
                }),
            wxUI::Button{ "&None" }
                .bind([this] {
                    mMarcherLabels = {};
                    EndModal(wxID_OK);
                }),
            wxUI::Button{ wxID_OK }.setDefault(),
        },
        wxUI::HSizer{
            wxUI::Custom{ [this, &show, mList](wxWindow* w, wxSizer* s, wxSizerFlags flags) {
                for (auto&& [which, bitmap] : CalChart::Ranges::enumerate_view(GetSymbolsBitmap())) {
                    if (!show.MakeSelectBySymbol(static_cast<CalChart::SYMBOL_TYPE>(which)).empty()) {
                        wxUI::BitmapButton{ bitmap }
                            .bind([this, which, &show, mList] {
                                // given a symbol, we go through the labels, and see if it matches
                                mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings() | std::views::filter([which, &show](auto&& label) {
                                    return show.GetPointSymbol(label) == static_cast<CalChart::SYMBOL_TYPE>(which);
                                }));
                                EndModal(wxID_OK);
                            })
                            .createAndAdd(w, s, flags);
                    }
                }
            } } },
        wxUI::HSizer{
            wxUI::Text{ "Select Instrument" },
            wxUI::Choice{ currentInstruments }
                .withSelection(wxNOT_FOUND)
                .bind([this, &show, mList](wxCommandEvent& e) {
                    // given the instrument, we go through the labels, and see if it matches
                    mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings() | std::views::filter([&show, e](auto&& label) {
                        return show.GetPointInstrument(label) == e.GetString();
                    }));
                    EndModal(wxID_OK);
                }),
        },
        wxUI::ListBox{ labels }
            .withStyle(wxLB_EXTENDED)
            .withSize(wxSize(50, 250))
            .withProxy(mList)
            .withSelections(std::vector<int>{ selected.begin(), selected.end() })
            .bind([this, mList] {
                mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList.selections().get() | std::views::transform([mList](auto&& i) {
                    return mList->GetString(i);
                }));
            })
            .bindDClick([this, mList] {
                mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings());
            }),
    }
        .fitTo(this);

    Center();
}
