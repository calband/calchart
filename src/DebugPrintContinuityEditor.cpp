/*
 * DebugPrintContinuityEditor.cpp
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

#include "DebugPrintContinuityEditor.hpp"
#include "CalChartConfiguration.h"
#include "CalChartContinuity.h"
#include "CalChartRanges.h"
#include "CalChartText.h"
#include "PrintContinuityEditor.h"
#include "ViewHandlers.hpp"

namespace {

constexpr auto page1
    = "\n\n~\\bsBIG C\\be\n\n1\t1/1\tAll:\tStand and Play Intro N\n\t8/2\tAll:\tShow High Hup with Horn "
      "Flash\n\tA1/1\tAll:\tFMHS 64 N\n\n80 Beats Total\n";

constexpr auto page2
    = "\n\n2\tE1\t\\po:\tFMHS NS/EW to SS 3 [MTHS E]\n\t\t\\so:\tFMHS EW/NS to SS 3 [MTHS "
      "E]\n\t\t\\ss:\tMTHS 2 E\n\t\t\tFMHS/DHS to SS 3 [MTHS E]\n\t\t\\sb:\tFMHS 12 N\n\t\t\tFMDHS/HS to SS 3 "
      "[MTHS E]\n\t\t\\px:\tFMHS 2 E\n\t\t\tMTHS 14 E\n\t\t\tFMDHS/HS to SS 3 [MTHS E]\n\n48 Beats Total\n";

struct PrintContinuityHelper {
    std::vector<CalChart::PrintContinuity> printContinuity;
    size_t currentSheet;
};

auto CreatePrintContinuityEditorHandlers(PrintContinuityHelper& printContinuityHelper, PrintContinuityEditor* editor)
    -> PrintContinuityEditor::Handlers
{
    return {
        [&printContinuityHelper]() {
            return printContinuityHelper.printContinuity.at(printContinuityHelper.currentSheet);
        },
        [&printContinuityHelper]() {
            return printContinuityHelper.printContinuity.at(printContinuityHelper.currentSheet).GetOriginalLine();
        },
        [&printContinuityHelper]() { return printContinuityHelper.currentSheet; },
        [&printContinuityHelper]() {
            return printContinuityHelper.printContinuity.at(printContinuityHelper.currentSheet).GetPrintNumber();
        },
        [&printContinuityHelper, editor](int which_sheet, std::string const& number, std::string const& cont) {
            printContinuityHelper.printContinuity.at(which_sheet) = CalChart::PrintContinuity{ number, cont };
            editor->Update();
        },
        [&printContinuityHelper, editor]() {
            if (printContinuityHelper.currentSheet == 0) {
                return;
            }
            --printContinuityHelper.currentSheet;
            editor->Update();
        },
        [&printContinuityHelper, editor]() {
            if (printContinuityHelper.currentSheet == printContinuityHelper.printContinuity.size() - 1) {
                return;
            }
            ++printContinuityHelper.currentSheet;
            editor->Update();
        },
    };
}
}

void DebugPrintContinuityEditor(wxWindow* parent, CalChart::Configuration const& config)
{
    auto* dialog = new wxDialog(parent, wxID_ANY, "Animation Errors Panel Playground", wxDefaultPosition,
        wxSize(800, 600), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    auto* print = new PrintContinuityEditor(dialog, config);

    PrintContinuityHelper helper{
        .printContinuity = { CalChart::PrintContinuity{ "1", page1 }, CalChart::PrintContinuity{ "test 2", page2 } },
        .currentSheet = 0,
    };
    print->SetHandlers(CreatePrintContinuityEditorHandlers(helper, print));
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(print, 1, wxEXPAND | wxALL);
    dialog->SetSizer(sizer);
    dialog->ShowModal();
    dialog->Destroy();
}
