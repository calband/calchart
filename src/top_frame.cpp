/*
 * main_ui.cpp
 * Handle wxWindows interface
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

#include "top_frame.h"
#include "basic_ui.h"
#include "ccvers.h"
#include "cc_preferences_ui.h"
#include "calchartapp.h"

#include <wx/dnd.h>
#include <wx/html/helpctrl.h>

IMPLEMENT_CLASS(TopFrame, wxDocParentFrame)

BEGIN_EVENT_TABLE(TopFrame, wxDocParentFrame)
EVT_MENU(wxID_ABOUT, TopFrame::OnCmdAbout)
EVT_MENU(wxID_HELP, TopFrame::OnCmdHelp)
EVT_MENU(wxID_PREFERENCES, TopFrame::OnCmdPreferences)
END_EVENT_TABLE()


class TopFrameDropTarget : public wxFileDropTarget
{
public:
	TopFrameDropTarget(wxDocManager *manager) : mManager(manager) {}
	virtual bool OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames);
private:
	wxDocManager *mManager;
};


bool
TopFrameDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames)
{
	for (size_t i = 0; i < filenames.Count(); i++)
	{
		mManager->CreateDocument(filenames[i], wxDOC_SILENT);
	}
	return true;
}


TopFrame::TopFrame(wxDocManager *manager,
				   wxFrame *frame,
				   const wxString& title) :
wxDocParentFrame(manager, frame, wxID_ANY, title)
{
// Give it an icon
	SetBandIcon(this);

	wxMenu *file_menu = new wxMenu;
	file_menu->Append(wxID_NEW, wxT("&New Show\tCTRL-N"), wxT("Create a new show"));
	file_menu->Append(wxID_OPEN, wxT("&Open...\tCTRL-O"), wxT("Load a saved show"));
	file_menu->Append(wxID_PREFERENCES, wxT("&Preferences\tCTRL-,"));
	file_menu->Append(wxID_EXIT, wxT("&Quit\tCTRL-Q"), wxT("Quit CalChart"));

	// A nice touch: a history of files visited. Use this menu.
	manager->FileHistoryUseMenu(file_menu);

	wxMenu *help_menu = new wxMenu;
	help_menu->Append(wxID_ABOUT, wxT("&About CalChart...\tCTRL-A"), wxT("Information about the program"));
	// this comes up as a leak on Mac?
	help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"), wxT("Help on using CalChart"));

	wxMenuBar *menu_bar = new wxMenuBar;
	menu_bar->Append(file_menu, wxT("&File"));
	menu_bar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menu_bar);

	SetDropTarget(new TopFrameDropTarget(manager));
	Show(true);
}


TopFrame::~TopFrame()
{
}


void
TopFrame::OnCmdAbout(wxCommandEvent& event)
{
	TopFrame::About();
}


void
TopFrame::OnCmdHelp(wxCommandEvent& event)
{
	TopFrame::Help();
}

void
TopFrame::About()
{
	(void)wxMessageBox(wxT("CalChart v") wxT( STRINGIZE(CC_MAJOR_VERSION) ) wxT(".") wxT( STRINGIZE(CC_MINOR_VERSION) ) wxT(".") wxT( STRINGIZE(CC_SUB_MINOR_VERSION))
		wxT("\nAuthor: Gurk Meeker, Richard Michael Powell\n")
		wxT("http://calchart.sourceforge.net\n")
		wxT("Copyright (c) 1994-2011 Garrick Meeker, Richard Michael Powell\n")
		wxT("\n")
		wxT("This program is free software: you can redistribute it and/or modify\n")
		wxT("it under the terms of the GNU General Public License as published by\n")
		wxT("the Free Software Foundation, either version 3 of the License, or\n")
		wxT("(at your option) any later version.\n")
		wxT("\n")
		wxT("This program is distributed in the hope that it will be useful,\n")
		wxT("but WITHOUT ANY WARRANTY; without even the implied warranty of\n")
		wxT("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n")
		wxT("GNU General Public License for more details.\n")
		wxT("\n")
		wxT("You should have received a copy of the GNU General Public License\n")
		wxT("along with this program.  If not, see <http://www.gnu.org/licenses/>.\n")
		wxT("\n")
		wxT("Compiled on ") __TDATE__ wxT(" at ") __TTIME__,
		wxT("About CalChart"));
}


void
TopFrame::Help()
{
	wxGetApp().GetGlobalHelpController().LoadFile();
	wxGetApp().GetGlobalHelpController().DisplayContents();
}


void
TopFrame::OnCmdPreferences(wxCommandEvent& event)
{
	CalChartPreferences dialog1(this);
	if (dialog1.ShowModal() == wxID_OK)
	{
	}
}

