/* CalChartApp.h
 * Header for CalChartApp
 *
 */

/*
   Copyright (C) 1995-2010  Richard Powell

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

#ifdef __GNUG__
#pragma interface
#endif

#include "modes.h"
#include "main_ui.h"
#include "cc_winlist.h"

#include <wx/wx.h>

class CalChartApp;

extern CalChartApp* gTheApp;

class ShowModeList;

// Define a new application
class CalChartApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual void MacOpenFile(const wxString &fileName);
	int OnExit();

	ShowModeList& GetModeList() { return mModeList; }
	MainFrameList& GetFrameList() { return mFrameList; }
	CC_WinList& GetWindowList() { return mWinList; }
private:
	ShowModeList mModeList;
	MainFrameList mFrameList;
	CC_WinList mWinList;
};

#endif // _CALCHARTAPP_H_
