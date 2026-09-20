/*
 * DebugAnimationErrorsPanel.cpp
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

#include "DebugAnimationErrorsPanel.hpp"
#include "AnimationErrorsPanel.h"
#include "CalChartConfiguration.h"
#include "CalChartContinuity.h"
#include "CalChartRanges.h"
#include "ViewHandlers.hpp"

namespace {

auto const errors = std::vector{
    CalChart::Animate::Errors{
        { CalChart::Animate::Error::OUTOFTIME, { 1 } },
    },
    CalChart::Animate::Errors{
        { CalChart::Animate::Error::EXTRATIME, { 2, 3 } },
        { CalChart::Animate::Error::WRONGPLACE, { 3, 4 } },
    },
    CalChart::Animate::Errors{},
    CalChart::Animate::Errors{
        { CalChart::Animate::Error::UNDEFINED, { 2, 3 } },
        { CalChart::Animate::Error::SYNTAX, { 3, 4 } },
        { CalChart::Animate::Error::DIVISION_ZERO, { 1 } },
    },
    CalChart::Animate::Errors{
        { CalChart::Animate::Error::OUTOFTIME, { 1 } },
        { CalChart::Animate::Error::INVALID_CM, { 2, 3 } },
        { CalChart::Animate::Error::INVALID_FNTN, { 3, 4 } },
        { CalChart::Animate::Error::NONINT, { 1 } },
        { CalChart::Animate::Error::NEGINT, { 1 } },
    },
};

auto const collisions = std::map<int, CalChart::SelectionList>{
    { 1, { 4, 5, 6 } },
};

std::string SetToString(const CalChart::SelectionList& sl)
{
    std::string result = "{";
    bool first = true;
    for (auto i : sl) {
        if (!first)
            result += ", ";
        result += std::format("{}", i);
        first = false;
    }
    return result + "}";
}

auto CreateAnimationErrorsPanelHandlers() -> AnimationErrorsPanel::Handlers
{
    return {
        []() { return errors; },
        []() { return collisions; },
        [](size_t which, CalChart::SelectionList const& sl) {
            wxMessageBox(std::format("Clicked {}: with {}", which, SetToString(sl)));
        },
    };
}
}

void DebugAnimationErrorsPanel(wxWindow* parent)
{
    auto* dialog = new wxDialog(parent, wxID_ANY, "Animation Errors Panel Playground", wxDefaultPosition,
        wxSize(800, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    auto* errors = new AnimationErrorsPanel(dialog);

    errors->SetHandlers(CreateAnimationErrorsPanelHandlers());
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(errors, 1, wxEXPAND | wxALL);
    dialog->SetSizer(sizer);
    dialog->ShowModal();
    dialog->Destroy();
}
