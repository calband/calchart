/*
 * CalChartApp.cpp
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

#include "CalChartApp.h"
#include "CalChartDoc.h"
#include "CalChartView.h"
#include "TopFrame.h"
#include "basic_ui.h"
#include "confgr.h"
#include "modes.h"
#include "platconf.h"
#include "single_instance_ipc.h"

#include <wx/fs_zip.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>

#include <wx/docview.h>
#include <wx/stdpaths.h>

wxPrintDialogData* gPrintDialogData;

namespace CalChart {
void Coord_UnitTests();
void Point_UnitTests();
void Show_UnitTests();
void ShowMode_UnitTests();
void Sheet_UnitTests();
void Continuity_UnitTests();
}

// This statement initializes the whole application and calls OnInit
IMPLEMENT_APP(CalChartApp)

wxHtmlHelpController& CalChartApp::GetGlobalHelpController()
{
    return *mHelpController;
}

#if defined(__APPLE__) && (__APPLE__)
void CalChartApp::MacOpenFile(wxString const& fileName)
{
    OpenFileOnHost(fileName);
}

void CalChartApp::MacOpenFiles(wxArrayString const& fileNames)
{
    for (auto index = 0; index < static_cast<int>(fileNames.GetCount()); ++index) {
        MacOpenFile(fileNames[index]);
    }
}
#endif // defined(__APPLE__) && (__APPLE__)

bool CalChartApp::OnInit()
{
    SetAppName(wxT("CalChart"));
    wxInitAllImageHandlers();
    StartStopFunc_t asServer{ [=]() { this->InitAppAsServer(); }, [=]() { this->ExitAppAsServer(); } };
    StartStopFunc_t asClient{ [=]() { this->InitAppAsClient(); }, [=]() { this->ExitAppAsClient(); } };
    mHostInterface = HostAppInterface::Make(this, asServer, asClient);
    return mHostInterface->OnInit();
}

int CalChartApp::OnExit()
{
    mHostInterface.reset(); // calls ExitApp for client or server
    return wxApp::OnExit();
}

void CalChartApp::OpenFileOnHost(const wxString& filename)
{
    mHostInterface->OpenFile(filename);
}

void CalChartApp::OpenFile(const wxString& fileName)
{
    mDocManager->CreateDocument(fileName, wxDOC_SILENT);
}

void CalChartApp::InitAppAsServer()
{
    //// Create a document manager
    mDocManager = new wxDocManager;

    //// Create a template relating drawing documents to their views
    (void)new wxDocTemplate(mDocManager, _T("CalChart Show"), _T("*.shw"), _T(""), _T("shw"), _T("CalChart Doc"), _T("CalChart View"), CLASSINFO(CalChartDoc), CLASSINFO(CalChartView));

    gPrintDialogData = new wxPrintDialogData;
    mHelpController = std::unique_ptr<wxHtmlHelpController>(new wxHtmlHelpController());

    //// Create the main frame window
    auto frame = new TopFrame(mDocManager, (wxFrame*)NULL, _T("CalChart"));

    {
        // Required for advanced HTML help
        wxFileSystem::AddHandler(new wxZipFSHandler);
        wxFileSystem::AddHandler(new wxArchiveFSHandler);

#if defined(__APPLE__) && (__APPLE__)
        wxString helpfile(wxT("CalChart.app/Contents/Resources"));
#else
        wxString helpfile = wxFileName(::wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR wxT("docs"));
#endif
        helpfile.Append(PATH_SEPARATOR wxT("charthlp.hhp"));
        if (!GetGlobalHelpController().AddBook(wxFileName(helpfile))) {
            wxLogError(wxT("Cannot find the help system."));
        }
    }

#ifndef __WXMAC__
    frame->Show(true);
#endif // ndef __WXMAC__
    SetTopWindow(frame);

    // Get the file history
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(wxT("/FileHistory"));
    mDocManager->FileHistoryLoad(*config);

    CalChart::Continuity_UnitTests();
    CalChart::Point_UnitTests();
    CalChart::Coord_UnitTests();
    CalChart::ShowMode_UnitTests();
    CalChart::Show_UnitTests();
    CalChart::Sheet_UnitTests();

    ProcessArguments();
}

void CalChartApp::InitAppAsClient() { ProcessArguments(); }

void CalChartApp::ProcessArguments()
{
    if (argc > 1) {
        OpenFileOnHost(argv[1]);
    }
}

void CalChartApp::ExitAppAsServer()
{
    // Flush out the other commands
    CalChartConfiguration::GetGlobalConfig().FlushWriteQueue();
    // Get the file history
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(wxT("/FileHistory"));
    mDocManager->FileHistorySave(*config);
    config->Flush();

    if (gPrintDialogData)
        delete gPrintDialogData;

    // delete the doc manager
    delete wxDocManager::GetDocumentManager();

    mHelpController.reset();
}

CalChart::ShowMode CalChartApp::GetShowMode(wxString const& which)
{
    return GetConfigShowMode(which);
}

void CalChartApp::ExitAppAsClient() {}
