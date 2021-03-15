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
#include "CalChartApp.h"
#include "basic_ui.h"
#include "confgr.h"
#include "modes.h"

#include <wx/wx.h>

// page for deciding the field type
ChooseShowModeWizard::ChooseShowModeWizard(wxWizard* parent)
    : wxWizardPageSimple(parent)
{
    for (auto mode : kShowModeStrings) {
        modeStrings.Add(mode);
    }

    SetSizer(VStack([this](auto sizer) {
        CreateText(this, sizer, BasicSizerFlags(), "Choose a field to set your show:");
        mChoice = CreateChoiceWithHandler(this, sizer, BasicSizerFlags(), wxID_ANY, modeStrings);
    }));
}

wxString ChooseShowModeWizard::GetValue()
{
    return modeStrings[mChoice->GetSelection()];
}
