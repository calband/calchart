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

#include "PreferencesUtils.h"
#include "basic_ui.h"

IMPLEMENT_ABSTRACT_CLASS(PreferencePage, wxPanel)


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

// convience sizers to change the view behavior in all at once.
static auto sRightBasicSizerFlags = wxSizerFlags().Border(wxALL, 2).Right().Proportion(0);
static auto sExpandSizerFlags = wxSizerFlags().Border(wxALL, 2).Center().Proportion(0);

void AddTextboxWithCaption(wxWindow* parent, wxBoxSizer* verticalsizer,
    int id, const wxString& caption,
    long style)
{
    wxBoxSizer* textsizer = new wxBoxSizer(wxVERTICAL);
    textsizer->Add(new wxStaticText(parent, wxID_STATIC, caption,
                       wxDefaultPosition, wxDefaultSize, 0),
                   LeftBasicSizerFlags());
    textsizer->Add(new wxTextCtrl(parent, id, wxEmptyString, wxDefaultPosition,
                       wxDefaultSize, style),
                   BasicSizerFlags());
    verticalsizer->Add(textsizer, BasicSizerFlags());
}

void AddCheckboxWithCaption(wxWindow* parent, wxBoxSizer* verticalsizer,
    int id, const wxString& caption,
    long style)
{
    wxBoxSizer* textsizer = new wxBoxSizer(wxVERTICAL);
    textsizer->Add(new wxCheckBox(parent, id, caption, wxDefaultPosition,
                       wxDefaultSize, style),
                   BasicSizerFlags());
    verticalsizer->Add(textsizer, BasicSizerFlags());
}


