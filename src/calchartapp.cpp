/*
 * calchartapp.cpp
 * Central App for CalChart
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

#include "calchartapp.h"
#include "calchartdoc.h"
#include "top_frame.h"
#include "modes.h"
#include "confgr.h"
#include "basic_ui.h"
#include "platconf.h"
#include "field_view.h"

#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/fs_zip.h>

#include <wx/docview.h>

wxPrintDialogData *gPrintDialogData;

void CC_continuity_UnitTests();
void CC_point_UnitTests();
void CC_coord_UnitTests();

// This statement initializes the whole application and calls OnInit
IMPLEMENT_APP(CalChartApp)

wxHtmlHelpController&
CalChartApp::GetGlobalHelpController()
{
	return *mHelpController;
}

void CalChartApp::MacOpenFile(const wxString &fileName)
{
	OpenFileOnHost(fileName);
}

void CalChartApp::MacOpenFiles(const wxArrayString &fileNames) {
	for (int index = fileNames.GetCount() - 1; index >= 0; index--) {
		MacOpenFile(fileNames[index]);
	}
}

bool CalChartApp::OnInit()
{
	StartStopFunc_t asServer { [=](){ this->InitAppAsServer(); }, [=](){ this->ExitAppAsServer(); } };
	StartStopFunc_t asClient { [=](){ this->InitAppAsClient(); }, [=](){ this->ExitAppAsClient(); } };
	mHostInterface = HostAppInterface::Make(this, asServer, asClient);
	return mHostInterface->OnInit();
}

int CalChartApp::OnExit()
{
	mHostInterface.reset(); // calls ExitApp for client or server
	return wxApp::OnExit();
}




void CalChartApp::OpenFileOnHost(const wxString &filename) {
	mHostInterface->OpenFile(filename);
}

void CalChartApp::OpenFile(const wxString &fileName) {
	mDocManager->CreateDocument(fileName, wxDOC_SILENT);
}



void CalChartApp::InitAppAsServer() {
	//// Create a document manager
	mDocManager = new wxDocManager;

	//// Create a template relating drawing documents to their views
	(void) new wxDocTemplate(mDocManager, _T("CalChart Show"), _T("*.shw"), _T(""), _T("shw"), _T("CalChart"), _T("Field View"),
		CLASSINFO(CalChartDoc), CLASSINFO(FieldView));

	gPrintDialogData = new wxPrintDialogData();
	mHelpController = std::unique_ptr<wxHtmlHelpController>(new wxHtmlHelpController());

	//// Create the main frame window
	wxFrame *frame = new TopFrame(mDocManager, (wxFrame *)NULL, _T("CalChart"));

	{
		// Required for images in the online documentation
		wxImage::AddHandler(new wxGIFHandler);

		// Required for advanced HTML help
		wxFileSystem::AddHandler(new wxZipFSHandler);
		wxFileSystem::AddHandler(new wxArchiveFSHandler);

#if defined(__APPLE__) && (__APPLE__)
		wxString helpfile(wxT("CalChart.app/docs"));
#else
		wxString helpfile(wxT("docs"));
#endif
		helpfile.Append(PATH_SEPARATOR wxT("charthlp.hhp"));
		if (!GetGlobalHelpController().AddBook(wxFileName(helpfile)))
		{
			wxLogError(wxT("Cannot find the help system."));
		}
	}

#ifndef __WXMAC__
	frame->Show(true);
#endif //ndef __WXMAC__
	SetTopWindow(frame);

	// Get the file history
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/FileHistory"));
	mDocManager->FileHistoryLoad(*config);

	CC_continuity_UnitTests();
	CC_point_UnitTests();
	CC_coord_UnitTests();


	ProcessArguments();
}

void CalChartApp::InitAppAsClient() {
	ProcessArguments();
}

void CalChartApp::ProcessArguments() {
	if (argc > 1) {
		OpenFileOnHost(argv[1]);
	}
}





void CalChartApp::ExitAppAsServer() {
	// Flush out the other commands
	CalChartConfiguration::GetGlobalConfig().FlushWriteQueue();
	// Get the file history
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/FileHistory"));
	mDocManager->FileHistorySave(*config);
	config->Flush();

	if (gPrintDialogData) delete gPrintDialogData;

	// delete the doc manager
	delete wxDocManager::GetDocumentManager();

	mHelpController.reset();
}

std::unique_ptr<ShowMode>
CalChartApp::GetMode(const wxString& which)
{
	return ShowMode::GetMode(which);
}

void CalChartApp::ExitAppAsClient() {
}

