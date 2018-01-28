/*
 * setup_wizards.cpp
 * Classes for setting up shows
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "setup_wizards.h"
#include "basic_ui.h"
#include "calchartapp.h"
#include "confgr.h"
#include "modes.h"

#include <wx/wx.h>

// page for giving a description
SetDescriptionWizard::SetDescriptionWizard(wxWizard* parent)
    : wxWizardPageSimple(parent)
{
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);
    wxStaticText* label = new wxStaticText(
        this, wxID_STATIC, wxT("Enter a show description for your show:"),
        wxDefaultPosition, wxDefaultSize, 0);
    topsizer->Add(label, 0, wxALL, 5);
    mText = new FancyTextWin(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
        wxSize(240, 100));
    topsizer->Add(mText, 0, wxALL, 5);
    topsizer->Fit(this);
}

wxString SetDescriptionWizard::GetValue() { return mText->GetValue(); }

// page for deciding the field type
ChooseShowModeWizard::ChooseShowModeWizard(wxWizard* parent)
    : wxWizardPageSimple(parent)
{
    for (auto mode : kShowModeStrings) {
        modeStrings.Add(mode);
    }
    for (auto mode : kSpringShowModeStrings) {
        modeStrings.Add(mode);
    }

    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(topsizer);
    wxStaticText* label = new wxStaticText(
        this, wxID_STATIC, wxT("Choose a field to set your show:"),
        wxDefaultPosition, wxDefaultSize, 0);
    topsizer->Add(label, 0, wxALL, 5);
    mChoice = new wxChoice(this, wxID_ANY, wxPoint(5, 5), wxDefaultSize, modeStrings);
    topsizer->Add(mChoice, 0, wxALL, 5);
    topsizer->Fit(this);
}

wxString ChooseShowModeWizard::GetValue()
{
    return modeStrings[mChoice->GetSelection()];
}
