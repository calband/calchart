/*
 * CalChartPreferences.cpp
 * Dialox box for preferences
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

#include "CalChartPreferences.h"
#include "PreferencesContCellSetup.h"
#include "PreferencesDrawingSetup.h"
#include "PreferencesGeneralSetup.h"
#include "PreferencesPSPrintingSetup.h"
#include "PreferencesShowModeSetup.h"
#include "PreferencesUtils.h"

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

CalChartPreferences::CalChartPreferences(wxWindow* parent, wxWindowID id, const wxString& caption, const wxPoint& pos, const wxSize& size, long style)
    : wxDialog(parent, id, caption, pos, size, style)
    , mConfig(CalChartConfiguration::GetGlobalConfig())
{
    auto topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);

    mNotebook = new wxNotebook(this, wxID_ANY);
    topsizer->Add(mNotebook, BasicSizerFlags());

    mNotebook->AddPage(new GeneralSetup(mConfig, mNotebook, wxID_ANY), wxT("General"));
    mNotebook->AddPage(new ContCellSetup(mConfig, mNotebook, wxID_ANY), wxT("Continuity"));
    mNotebook->AddPage(new DrawingSetup(mConfig, mNotebook, wxID_ANY), wxT("Drawing"));
    mNotebook->AddPage(new PSPrintingSetUp(mConfig, mNotebook, wxID_ANY), wxT("PS Printing"));
    mNotebook->AddPage(new ShowModeSetup(mConfig, mNotebook, wxID_ANY), wxT("Show Mode Setup"));

    // the buttons on the bottom
    wxBoxSizer* okCancelBox = new wxBoxSizer(wxHORIZONTAL);
    topsizer->Add(okCancelBox, BasicSizerFlags());

    okCancelBox->Add(new wxButton(this, wxID_APPLY), BasicSizerFlags());
    okCancelBox->Add(new wxButton(this, wxID_RESET, wxT("&Reset All")), BasicSizerFlags());
    okCancelBox->Add(new wxButton(this, wxID_OK), BasicSizerFlags());
    okCancelBox->Add(new wxButton(this, wxID_CANCEL), BasicSizerFlags());

    // This fits the dalog to the minimum size dictated by the sizers
    GetSizer()->Fit(this);
    // This ensures that the dialog cannot be smaller than the minimum size
    GetSizer()->SetSizeHints(this);
    Center();
}

bool CalChartPreferences::TransferDataToWindow() { return true; }

bool CalChartPreferences::TransferDataFromWindow()
{
    // transfer everything to the config...
    size_t pages = mNotebook->GetPageCount();
    for (size_t i = 0; i < pages; ++i) {
        PreferencePage* page = static_cast<PreferencePage*>(mNotebook->GetPage(i));
        page->TransferDataFromWindow();
    }
    CalChartConfiguration::AssignConfig(mConfig);
    return true;
}

void CalChartPreferences::OnCmdResetAll(wxCommandEvent&)
{
    // transfer everything to the config...
    size_t pages = mNotebook->GetPageCount();
    for (size_t i = 0; i < pages; ++i) {
        PreferencePage* page = static_cast<PreferencePage*>(mNotebook->GetPage(i));
        page->ClearValuesToDefault();
    }
    CalChartConfiguration::AssignConfig(mConfig);
}
