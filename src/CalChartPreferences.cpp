/*
 * CalChartPreferences.cpp
 * Dialox box for preferences
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

#include "CalChartPreferences.h"
#include "PreferencesContCellSetup.h"
#include "PreferencesDrawingSetup.h"
#include "PreferencesGeneralSetup.h"
#include "PreferencesPSPrintingSetup.h"
#include "PreferencesPrintContinuitySetup.h"
#include "PreferencesShowModeSetup.h"
#include "PreferencesUtils.h"
#include <wxUI/wxUI.h>

// how the preferences work:
// preference dialog create a copy of the CalChart config from which to read and
// set values
// CalChart config doesn't automatically write values to main config, it must be
// flushed
// out when the user presses apply.
// first page will be general settings:
//   Auto save behavior: file location, time
// second page is Drawing preferences for edit menu
//   Color preferences
// second page is PS printing settings
// 3rd page is Show mode setup
//
// organized into pages.  Each page is responsible for reading out
// on TransferDataToWindow, caching the values locally, and
// setting them to the system on TransferDataFromWindow

////////////////

BEGIN_EVENT_TABLE(CalChartPreferences, wxDialog)
EVT_BUTTON(wxID_RESET, CalChartPreferences::OnCmdResetAll)
END_EVENT_TABLE()

IMPLEMENT_CLASS(CalChartPreferences, wxDialog)

CalChartPreferences::CalChartPreferences(wxWindow* parent, CalChartConfiguration& config)
    : super(parent, wxID_ANY, "CalChart Preferences", wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU)
    , mConfig(config)
{
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::Generic{
            [this] {
                mNotebook = new wxNotebook(this, wxID_ANY);

                mNotebook->AddPage(GeneralSetup::CreatePreference(mConfig, mNotebook), wxT("General"));
                mNotebook->AddPage(ContCellSetup::CreatePreference(mConfig, mNotebook), wxT("Continuity"));
                mNotebook->AddPage(DrawingSetup::CreatePreference(mConfig, mNotebook), wxT("Drawing"));
                mNotebook->AddPage(ShowModeSetup::CreatePreference(mConfig, mNotebook), wxT("Show Mode Setup"));
                mNotebook->AddPage(PrintContinuitySetup::CreatePreference(mConfig, mNotebook), wxT("Print Continuity Setup"));
                mNotebook->AddPage(PSPrintingSetUp::CreatePreference(mConfig, mNotebook), wxT("PS Printing"));
                return mNotebook;
            }(),
        },
        wxUI::HSizer{
            wxUI::Button{ wxID_APPLY },
            wxUI::Button{ wxID_RESET, "&Reset All" },
            wxUI::Button{ wxID_OK },
            wxUI::Button{ wxID_CANCEL },
        },
    }
        .attachTo(this);

    Center();
}

bool CalChartPreferences::TransferDataToWindow() { return true; }

bool CalChartPreferences::TransferDataFromWindow()
{
    // transfer everything to the config...
    auto pages = mNotebook->GetPageCount();
    for (auto i = 0ul; i < pages; ++i) {
        mNotebook->GetPage(i)->TransferDataFromWindow();
    }
    return true;
}

void CalChartPreferences::OnCmdResetAll(wxCommandEvent&)
{
    // transfer everything to the config...
    auto pages = mNotebook->GetPageCount();
    for (auto i = 0ul; i < pages; ++i) {
        static_cast<PreferencePage*>(mNotebook->GetPage(i))->ClearValuesToDefault();
    }
}
