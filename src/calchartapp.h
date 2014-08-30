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
#include "single_instance_ipc.h"

#include <wx/wx.h>
#include <wx/docview.h>
#include <memory>

class CalChartApp;
class ShowMode;
class wxHtmlHelpController;

DECLARE_APP(CalChartApp)

// Define a new application
class CalChartApp : public wxApp
{
public:
	virtual bool OnInit();
	virtual void MacOpenFile(const wxString &fileName);
	virtual void MacOpenFiles(const wxArrayString &fileNames);
	int OnExit();

	std::unique_ptr<ShowMode> GetMode(const wxString& which);

	// the global help system:
	wxHtmlHelpController& GetGlobalHelpController();

	void OpenFile(const wxString &fileName);
	void OpenFileOnHost(const wxString &filename);
private:
	void ProcessArguments ();

	void InitAppAsServer();
	void InitAppAsClient();
	void ExitAppAsServer();
	void ExitAppAsClient();

	wxDocManager* mDocManager;
	std::unique_ptr<wxHtmlHelpController> mHelpController;

	std::unique_ptr<HostAppInterface> mHostInterface;
};

#endif // _CALCHARTAPP_H_
