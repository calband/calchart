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
#include "main_ui.h"

#include <wx/wx.h>
#include <wx/docview.h>
#include <boost/shared_ptr.hpp>

class CalChartApp;

class ShowModeList;

DECLARE_APP(CalChartApp)

class TopFrame;
TopFrame *GetMainFrame(void);

// Define a new application
class CalChartApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual void MacOpenFile(const wxString &fileName);
	int OnExit();

	ShowModeList& GetModeList() { return mModeList; }
private:
	ShowModeList mModeList;

	boost::shared_ptr<wxDocManager> mDocManager;
};

#endif // _CALCHARTAPP_H_
