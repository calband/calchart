/*
 * MarcherPicker.cpp
 * Dialog for picking marchers
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
#include "CalChartDoc.h"
#include "CalChartRanges.h"
#include "CalChartToolBar.h"
#include <ranges>
#include <wx/dialog.h>
#include <wxUI/wxUI.hpp>

class PointPicker : public wxDialog {
    using super = wxDialog;

public:
    PointPicker(wxWindow* parent, CalChartDoc const& show, CalChart::SelectionList const& marchersToUse, CalChart::SelectionList const& selected);
    ~PointPicker() override = default;

    auto GetMarchersSelected() const { return mMarcherLabels; }

private:
    std::vector<std::string> mMarcherLabels;
};

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
                    mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings() | std::views::transform([](auto&& string) {
                        return string.ToStdString();
                    }));
                    EndModal(wxID_OK);
                }),
            wxUI::Button{ "&None" }
                .bind([this] {
                    mMarcherLabels = {};
                    EndModal(wxID_OK);
                }),
            wxUI::Button{ wxID_OK }.setDefault(),
        },
        wxUI::HForEach(
            CalChart::Ranges::enumerate_view(GetSymbolsBitmap())
                | std::views::filter([&](auto item) {
                      return !show.MakeSelectBySymbol(static_cast<CalChart::SYMBOL_TYPE>(std::get<0>(item))).empty();
                  }),
            [this, &show, mList](auto&& item) {
                auto&& [which, bitmap] = item;
                return wxUI::BitmapButton{ bitmap }
                    .bind([this, which, &show, mList] {
                        // given a symbol, we go through the labels, and see if it matches
                        mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings() | std::views::transform([](auto&& string) {
                            return string.ToStdString();
                        }) | std::views::filter([which, &show](auto&& label) {
                            return show.GetPointSymbol(label) == static_cast<CalChart::SYMBOL_TYPE>(which);
                        }));
                        EndModal(wxID_OK);
                    });
            }),
        wxUI::HSizer{
            wxUI::Text{ "Select Instrument" },
            wxUI::Choice{ currentInstruments }
                .withSelection(wxNOT_FOUND)
                .bind([this, &show, mList](wxCommandEvent& e) {
                    // given the instrument, we go through the labels, and see if it matches
                    mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings() | std::views::transform([](auto&& string) {
                        return string.ToStdString();
                    }) | std::views::filter([&show, e](auto&& label) {
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
                    return mList->GetString(i).ToStdString();
                }));
            })
            .bindDClick([this, mList] {
                mMarcherLabels = CalChart::Ranges::ToVector<std::string>(mList->GetStrings() | std::views::transform([](auto&& string) {
                    return string.ToStdString();
                }));
            }),
    }
        .fitTo(this);

    Center();
}

auto PromptUserToPickMarchers(wxWindow* parent, CalChartDoc const& show, CalChart::SelectionList const& marchersToUse, CalChart::SelectionList const& selected) -> std::optional<std::vector<std::string>>
{
    PointPicker dialog(parent, show, marchersToUse, selected);
    if (dialog.ShowModal() == wxID_OK) {
        return dialog.GetMarchersSelected();
    }
    return std::nullopt;
}

auto PromptUserToPickMarchers(wxWindow* parent, CalChartDoc const& show) -> std::optional<std::vector<std::string>>
{
    return PromptUserToPickMarchers(
        parent,
        show,
        CalChart::SelectionList(std::views::iota(0, show.GetNumPoints()).begin(), std::views::iota(0, show.GetNumPoints()).end()),
        show.GetSelectionList());
}
