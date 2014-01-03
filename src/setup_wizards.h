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

#ifndef _SETUP_WIZARDS_H_
#define _SETUP_WIZARDS_H_

#include <wx/wizard.h>

#include <vector>

class FancyTextWin;
class wxChoice;

/**
 * The page of the setup wizard that asks the user to provide a description
 * for the show.
 * This page is linked into the full setup wizard and is used by FieldView,
 * through the OnWizardSetup(...) method.
 */
class SetDescriptionWizard : public wxWizardPageSimple
{
public:

	/**
	 * Makes the page.
	 * @param parent The setup wizard which this page is a part of.
	 */
	SetDescriptionWizard(wxWizard *parent);

	/**
	 * Returns the text that the user entered into the text box
	 * for the show description.
	 * @return The description that the user has chosen for the
	 * show.
	 */
	wxString GetValue();
	
private:
	/**
	 * TODO IS THIS UNUSED?
	 */
	wxArrayString modeStrings;

	/**
	 * A pointer to the text box that collects the user's input.
	 * You can extract the user's answer from it to get the show
	 * description.
	 */
	FancyTextWin *mText;
};

/**
 * The page of the setup wizard that asks the user to define a show mode for
 * the CalChart show. Defining the "Show Mode" is essentially the equivalent of
 * defining what field your show will take place on (e.g. in tunnel, on the
 * football field, etc.).
 * This page is linked into the setup wizard by FieldView, when it is used through
 * the OnWizardSetup(...) method.
 */
class ChooseShowModeWizard : public wxWizardPageSimple
{
public:
	/**
	 * Makes the page.
	 * @param parent The setup wizard which this page is a part of.
	 */
	ChooseShowModeWizard(wxWizard *parent);

	/**
	 * Returns the name of the show mode selected by the user.
	 * @return The name of the show mode selected by the user.
	 */
	wxString GetValue();
	
private:
	
	/**
	 * The list of strings that will be provided as options
	 * for show modes. mChoice provides an integer representing
	 * which index in this array represents the final choice of
	 * the user.
	 */
	wxArrayString modeStrings;

	/**
	 * A dropdown box that collects the user's choice. From this,
	 * you can extract an integer representing the choice of the
	 * user. This integer is the index of the item in modeStrings
	 * that the user selected.
	 */
	wxChoice *mChoice;
};


#endif
