/*
 * DebugContinuityBrowser.cpp
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

#include "DebugContinuityBrowser.hpp"
#include "CalChartConfiguration.h"
#include "CalChartContinuity.h"
#include "CalChartRanges.h"
#include "ContinuityBrowser.h"
#include "ViewHandlers.hpp"

void DebugContinuityBrowser(wxWindow* parent, CalChart::Configuration& config)
{
    auto* dialog = new wxDialog(parent, wxID_ANY, "Continuity Browser Test",
        wxDefaultPosition, wxSize(800, 600),
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    auto* browser = new ContinuityBrowser(dialog, wxSize(800, 600), config);

    auto plainCont = CalChart::Continuity{ "mt E REM\nnsew np" };
    auto solCont = CalChart::Continuity{ "nsew np" };

    browser->SetHandlers(GetDebugContinuityHandlers(
        plainCont, solCont,
        [&]([[maybe_unused]] CalChart::SYMBOL_TYPE sym, [[maybe_unused]] CalChart::Continuity const& new_cont) {
            wxLogDebug("Continuity changed for symbol %c", sym);
            if (sym == CalChart::SYMBOL_PLAIN) {
                plainCont = new_cont;
            } else if (sym == CalChart::SYMBOL_SOL) {
                solCont = new_cont;
            }
            browser->OnUpdate();
        },
        [&]([[maybe_unused]] CalChart::SYMBOL_TYPE symbol) {
            wxLogDebug("Continuity selected for symbol %c", symbol);
            browser->OnUpdate();
        }));
    auto* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(browser, 1, wxEXPAND | wxALL);
    dialog->SetSizer(sizer);
    dialog->ShowModal();
    dialog->Destroy();
}
