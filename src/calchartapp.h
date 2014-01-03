/*
 * calchartapp.h
 * Header for CalChartApp
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

#ifndef _CALCHARTAPP_H_
#define _CALCHARTAPP_H_

#include "modes.h"

#include <wx/wx.h>
#include <wx/docview.h>

class CalChartApp;
class ShowMode;
class wxHtmlHelpController;

// the global help system:
wxHtmlHelpController& GetGlobalHelpController();

DECLARE_APP(CalChartApp)

/**
 * The Calchart Application.
 * This class (a wxWidgets application) functions as the starting point for 
 * the CalChart program. When you start up the CalChart program, the OnInit()
 * method of CalChartApp is called, and when you close the program, the
 * OnExit() method is called. This class is not drawn (so it is invisible to the
 * user), but when OnInit() is called (that is, when the user starts up the
 * CalChart program), CalChartApp creates a frame that IS visible to the user,
 * and the user can use that frame to view and edit shows.
 */
class CalChartApp : public wxApp
{
public:

	/**
	 * Initializes the application.
	 * This method is called when the program starts. It is responsible for
	 * setting up the frame to which CalChart is drawn (thereby making the
	 * program visible to the user) and for linking that frame to a wxDocManager
	 * that understands the CalChart show file, including how to open it, save it,
	 * and display it. The frame uses the wxDocManager to properly display
	 * the CalChart show to the user. 
	 * @return Returns true if the application should continue processing; false
	 * otherwise. False would be returned if an error occurs, for example.
	 * OnExit() will only be called if this method returns true.
	 */
	virtual bool OnInit();

	/**
	 * TODO - I'm not a mac user...
	 * @param fileName The name of the file to open.
	 * TODO WHY VIRTUAL?!
	 */
	virtual void MacOpenFile(const wxString &fileName);

	/**
	 * Cleans up the application immediately before exiting.
	 * This method is called when the program is closed. It performs cleanup.
	 * @return Ignored.
	 */
	int OnExit();

	//TODO
	ShowModeList& GetModeList() { return mModeList; }
private:

	//TODO
	ShowModeList mModeList;

	/**
	 * Understands how to open/save/view CalChart-associated files.
	 */
	wxDocManager* mDocManager;
};

#endif // _CALCHARTAPP_H_
