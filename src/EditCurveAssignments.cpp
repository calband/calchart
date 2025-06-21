/*
 * EditCurveAssignments.cpp
 * Dialog for Edting which Marchers are assigned to a curve
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

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

#include "EditCurveAssignments.hpp"
#include "CalChartDoc.h"
#include "PointPicker.h"
#include <ranges>
#include <wx/dialog.h>
#include <wxUI/wxUI.hpp>

namespace {
auto removeFromSet(CalChart::SelectionList set, CalChart::SelectionList const& toRemove)
{
    for (auto&& i : toRemove) {
        set.erase(i);
    }
    return set;
}
}

class EditCurveAssignments : public wxDialog {
    using super = wxDialog;

public:
    EditCurveAssignments(wxWindow* parent, CalChartDoc const& show, int whichCurve);
    ~EditCurveAssignments() = default;

    [[nodiscard]] auto GetCurveAssignment() const -> std::vector<std::string> { return mCurveAssignment; }

private:
    std::vector<std::string> mCurveAssignment;
};

EditCurveAssignments::EditCurveAssignments(wxWindow* parent, CalChartDoc const& show, int whichCurve)
    : super(parent, wxID_ANY, "Edit Curve Assignments", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
{
    wxUI::ListBox::Proxy listProxy{};
    auto marchers = show.GetMarchersAssignedToCurve(whichCurve);
    auto items = wxArrayString{};
    for (auto&& i : marchers) {
        items.push_back(show.GetPointLabel(i));
    }
    auto selections = marchers.empty() ? std::vector<int>{} : std::vector<int>{ 0 };
    auto useMarcherPicker = [this, &show, listProxy](int whereToInsert) {
        auto marchersToUse = removeFromSet(show.MakeSelectAll(), show.MakeSelectByLabels(CalChart::Ranges::ToVector<std::string>(listProxy->GetStrings())));
        if (auto labels = PromptUserToPickMarchers(this, show, marchersToUse, {});
            labels.has_value()) {
            wxArrayString items;
            for (auto&& label : *labels) {
                items.Add(label);
            }
            *listProxy = listProxy->Insert(items, whereToInsert);
            mCurveAssignment = CalChart::Ranges::ToVector<std::string>(listProxy->GetStrings());
        }
    };
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2).Center().Proportion(0),
        wxUI::HSizer{
            wxUI::Button{ "Insert" }.bind([listProxy, useMarcherPicker] {
                auto whereToInsert = [&] {
                    if (auto selected = listProxy.selection().get(); selected != wxNOT_FOUND) {
                        return selected;
                    }
                    return 0;
                }();
                useMarcherPicker(whereToInsert);
            }),
            wxUI::Button{ "Append" }.bind([listProxy, useMarcherPicker] {
                auto whereToInsert = [&] {
                    if (auto selected = listProxy.selection().get(); selected != wxNOT_FOUND) {
                        return selected + 1;
                    }
                    return 0;
                }();
                useMarcherPicker(whereToInsert);
            }),
        },
        wxUI::HSizer{
            wxUI::ListBox{ items }.withStyle(wxLB_SINGLE).withSize(wxSize(50, 250)).withSelections(selections).withProxy(listProxy),
            wxUI::VSizer{
                wxUI::Button{ "Move Up" }.bind([this, listProxy] {
                    auto selected = listProxy.selection().get();
                    auto next = selected - 1;
                    if (selected == wxNOT_FOUND || selected == 0) {
                        return;
                    }
                    auto value = listProxy->GetString(selected);
                    listProxy->Delete(selected);
                    *listProxy = listProxy->Insert(value, next);
                    mCurveAssignment = CalChart::Ranges::ToVector<std::string>(listProxy->GetStrings());
                }),
                wxUI::Button{ "Move Down" }.bind([this, listProxy] {
                    auto selected = listProxy.selection().get();
                    auto next = selected + 1;
                    if (selected == wxNOT_FOUND || static_cast<size_t>(next) == listProxy->GetCount()) {
                        return;
                    }
                    auto value = listProxy->GetString(selected);
                    listProxy->Delete(selected);
                    *listProxy = listProxy->Insert(value, next);
                    mCurveAssignment = CalChart::Ranges::ToVector<std::string>(listProxy->GetStrings());
                }),
                wxUI::Button{ "Remove" }.bind([this, listProxy] {
                    auto selected = listProxy.selection().get();
                    if (selected == wxNOT_FOUND) {
                        return;
                    }
                    listProxy->Delete(selected);
                    mCurveAssignment = CalChart::Ranges::ToVector<std::string>(listProxy->GetStrings());
                    if (listProxy->IsEmpty()) {
                        return;
                    }
                    if (static_cast<size_t>(selected) == listProxy->GetCount()) {
                        selected--;
                    }

                    *listProxy = selected;
                }),
            },
        },
        wxUI::HSizer{
            wxUI::Button{ wxID_CANCEL },
            wxUI::Button{ wxID_OK },
        },
    }
        .fitTo(this);
    Center();
}

auto PromptUserForCurveAssignment(wxWindow* parent, CalChartDoc const& show) -> std::optional<std::pair<size_t, std::vector<std::string>>>
{
    auto numberCurves = show.GetCurrentNumberCurves();
    if (numberCurves == 0) {
        wxMessageBox("Sorry, no curves available to edit.", "No Curves", wxOK | wxICON_INFORMATION);
        return std::nullopt;
    }
    auto whichCurve = 0;
    if (numberCurves > 1) {
        auto curves = CalChart::Ranges::ToVector<wxString>(std::views::iota(0ul, numberCurves) | std::views::transform([](int i) -> wxString { return std::string("Curve ") + std::to_string(i); }));

        whichCurve = wxGetSingleChoiceIndex(
            "Which curve would you like to edit?",
            "Edit Curve Assignments",
            static_cast<int>(numberCurves),
            curves.data());
        if (whichCurve == wxNOT_FOUND) {
            return std::nullopt;
        }
    }
    EditCurveAssignments dialog(parent, show, whichCurve);
    if (dialog.ShowModal() == wxID_OK) {
        return std::pair<size_t, std::vector<std::string>>{ whichCurve, dialog.GetCurveAssignment() };
    }
    return std::nullopt;
}
