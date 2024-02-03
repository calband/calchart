/*
 * CalChartApp.cpp
 * Central App for CalChart
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartSplash.h"
#include "CalChartView.h"
#include "HostAppInterface.h"
#include "basic_ui.h"
#include "platconf.h"

#include <wx/config.h>
#include <wx/fs_zip.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/stdpaths.h>

namespace CalChart {
void Point_UnitTests();
void Show_UnitTests();
void Sheet_UnitTests();
void Continuity_UnitTests();
}

// This statement initializes the whole application and calls OnInit
IMPLEMENT_APP(CalChartApp)

bool CalChartApp::OnInit()
{
    SetAppName("CalChart");
    wxInitAllImageHandlers();
    auto asServer = StartStopFunc_t{ [this]() { InitAppAsServer(); }, [this]() { ExitAppAsServer(); } };
    auto asClient = StartStopFunc_t{ [this]() { InitAppAsClient(); }, [this]() { ExitAppAsClient(); } };
    mHostInterface = HostAppInterface::Make(this, asServer, asClient);
    return mHostInterface->OnInit();
}

int CalChartApp::OnExit()
{
    mHostInterface.reset(); // calls ExitApp for client or server
    return wxApp::OnExit();
}

void CalChartApp::OpenFile(wxString const& fileName)
{
    mDocManager->CreateDocument(fileName, wxDOC_SILENT);
}

void CalChartApp::OpenFileOnHost(wxString const& filename)
{
    mHostInterface->OpenFile(filename);
}

#if defined(__APPLE__) && (__APPLE__)
void CalChartApp::MacOpenFile(wxString const& fileName)
{
    OpenFileOnHost(fileName);
}

void CalChartApp::MacOpenFiles(wxArrayString const& fileNames)
{
    for (auto file : fileNames) {
        MacOpenFile(file);
    }
}
#endif // defined(__APPLE__) && (__APPLE__)

wxHtmlHelpController& CalChartApp::GetGlobalHelpController()
{
    return *mHelpController;
}

wxPrintDialogData& CalChartApp::GetGlobalPrintDialog()
{
    return *mPrintDialogData;
}

void CalChartApp::InitAppAsServer()
{
    //// Create a document manager
    mDocManager = new wxDocManager;

    //// Create a template relating drawing documents to their views
    (void)new wxDocTemplate(mDocManager, "CalChart Show", "*.shw", "", "shw", "CalChart Doc", "CalChart View", CLASSINFO(CalChartDoc), CLASSINFO(CalChartView));

    mHelpController = std::make_unique<wxHtmlHelpController>();
    mPrintDialogData = std::make_unique<wxPrintDialogData>();

    //// Create the main frame window
    auto frame = new CalChartSplash(mDocManager, nullptr, "CalChart", CalChartConfiguration::GetGlobalConfig());

    // Required for advanced HTML help
    wxFileSystem::AddHandler(new wxZipFSHandler);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);

#if defined(__APPLE__) && (__APPLE__)
    auto helpfile = wxString("CalChart.app/Contents/Resources");
#else
    auto helpfile = wxFileName(::wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR "docs");
#endif
    helpfile.Append(PATH_SEPARATOR "charthlp.hhp");
    if (!GetGlobalHelpController().AddBook(wxFileName(helpfile))) {
        wxLogError("Cannot find the help system.");
    }

#ifndef __WXMAC__
    frame->Show(true);
#endif // ndef __WXMAC__
    SetTopWindow(frame);

    // Get the file history
    auto* config = wxConfigBase::Get();
    config->SetPath("/FileHistory");
    mDocManager->FileHistoryLoad(*config);

    // run the built in self tests.
    CalChart::Continuity_UnitTests();
    CalChart::Point_UnitTests();
    CalChart::Show_UnitTests();
    CalChart::Sheet_UnitTests();

    ProcessArguments();
}

void CalChartApp::InitAppAsClient()
{
    ProcessArguments();
}

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
    auto config = wxConfigBase::Get();
    config->SetPath("/FileHistory");
    mDocManager->FileHistorySave(*config);
    config->Flush();

    // delete the doc manager
    delete wxDocManager::GetDocumentManager();

    mHelpController.reset();
}

void CalChartApp::ExitAppAsClient() { }
