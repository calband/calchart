/*
 * setup_wizards.h
 * Classes for setting up the shows
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

#pragma once

#include <wx/wizard.h>

#include <vector>

class FancyTextWin;
class wxChoice;

// page for giving a description
class SetDescriptionWizard : public wxWizardPageSimple {
public:
    SetDescriptionWizard(wxWizard* parent);

    wxString GetValue();

private:
    wxArrayString modeStrings;
    FancyTextWin* mText;
};

// page for deciding the field type
class ChooseShowModeWizard : public wxWizardPageSimple {
public:
    ChooseShowModeWizard(wxWizard* parent);
    wxString GetValue();

private:
    wxArrayString modeStrings;
    wxChoice* mChoice;
};
