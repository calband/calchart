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
#include "BugReportDialog.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "CalChartLogTarget.h"
#include "CalChartSplash.h"
#include "CalChartView.h"
#include "HostAppInterface.h"
#include "SystemConfiguration.h"
#include "UpdateChecker.h"
#include "basic_ui.h"
#include "platconf.h"

#include "CircularLogBuffer.hpp"
#include <wx/config.h>
#include <wx/fs_zip.h>
#include <wx/help.h>
#include <wx/html/helpctrl.h>
#include <wx/msgdlg.h>
#include <wx/stdpaths.h>
#include <wxUI/wxUI.hpp>

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

auto CalChartApp::OnExceptionInMainLoop() -> bool
{
    // Handle exceptions in the main event loop
    // This is called when an unhandled exception occurs
    try {
        // Re-throw to get the exception
        throw;
    } catch (const std::exception& e) {
        // Standard library exception
        auto result = wxMessageDialog{
            nullptr,
            wxString::Format(
                "An unexpected error occurred:\n\n%s\n\n"
                "Would you like to open the bug report dialog?",
                e.what()),
            "CalChart Error",
            wxYES_NO | wxICON_ERROR
        }
                          .ShowModal();
        if (result == wxID_YES) {
            BugReportDialog{ wxCalChart::GetGlobalConfig(),
                nullptr,
                dynamic_cast<wxFrame*>(GetTopWindow()),
                "Report a Bug - Crash Report" }
                .ShowModal();
        } else {
            throw e;
        }
    } catch (...) {
        // Unknown exception
        auto result = wxMessageDialog{
            nullptr,
            "An unexpected error occurred.\n\n"
            "Would you like to open the bug report dialog?",
            "CalChart Error",
            wxYES_NO | wxICON_ERROR
        }
                          .ShowModal();
        if (result == wxID_YES) {
            BugReportDialog{ wxCalChart::GetGlobalConfig(),
                nullptr,
                dynamic_cast<wxFrame*>(GetTopWindow()),
                "Report a Bug - Crash Report" }
                .ShowModal();
        } else {
            throw;
        }
    }
    return true;
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

CalChart::CircularLogBuffer CalChartApp::GetLogBuffer() const
{
    return mLogTarget->GetLogBuffer();
}

void CalChartApp::InitAppAsServer()
{
    wxLogDebug("Initializing CalChart as server...");

    // Create log target (wxLog will take ownership and delete it)
    mLogTarget = new CalChartLogTarget(CalChart::CircularLogBuffer{ 100 });

    // Get the current active logger (if any) and chain it directly
    auto oldLogTarget = wxLog::SetActiveTarget(mLogTarget);

    // Chain the old logger to our target so messages flow through both
    if (oldLogTarget && oldLogTarget != mLogTarget) {
        mLogTarget->SetNextTarget(oldLogTarget);
    }
    wxLogDebug("Post Set up logging system...");

    //// Create a document manager
    mDocManager = new wxDocManager;

    //// Create a template relating drawing documents to their views
    (void)new wxDocTemplate(mDocManager, "CalChart Show", "*.shw", "", "shw", "CalChart Doc", "CalChart View", CLASSINFO(CalChartDoc), CLASSINFO(CalChartView));

    mHelpController = std::make_unique<wxHtmlHelpController>();
    mPrintDialogData = std::make_unique<wxPrintDialogData>();

    //// Create the main frame window
    auto frame = new CalChartSplash(mDocManager, nullptr, "CalChart", wxCalChart::GetGlobalConfig());

    // Required for advanced HTML help
    wxFileSystem::AddHandler(new wxZipFSHandler);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);

#if defined(__APPLE__) && (__APPLE__)
    auto helpfile = wxStandardPaths::Get().GetResourcesDir();
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

    // Start background update check: pass the current CC_VERSION and the user's ignored version from config.
    auto currentVersion = std::string(CC_VERSION);
    auto ignored = wxCalChart::GetGlobalConfig().Get_IgnoredUpdateVersion();
    CalChart::StartBackgroundCheck(currentVersion, ignored, [frame](std::string latestTag) {
        // Core invokes this callback on the worker thread. Marshal to the UI thread here.
        if (!wxTheApp) {
            // No wx application available; best-effort: log and do nothing.
            std::cerr << "[CalChartApp] wxTheApp not available to show update dialog for tag=" << latestTag << "\n";
        }
        wxTheApp->CallAfter([frame, latestTag]() {
            // Build a small dialog with a checkbox "Never show this again for this release".
            wxDialog dlg(frame, wxID_ANY, "CalChart Update", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
            using namespace wxUI;
            CheckBox::Proxy neverBox;
            VSizer{
                wxSizerFlags().Expand().Border(wxALL, 10),
                Text{ std::format("A new version of CalChart is available: {}\nWould you like to open the releases page?", latestTag) },
                CheckBox{ "Never show this again for this release" }.withProxy(neverBox),
                HSizer{
                    wxSizerFlags().Center().Border(wxALL, 5),
                    Button{ wxID_OK, "Open Releases" },
                    Button{ wxID_CANCEL, "Dismiss" },
                },
            }
                .fitTo(&dlg);
            auto res = dlg.ShowModal();
            if (*neverBox) {
                wxCalChart::GetGlobalConfig().Set_IgnoredUpdateVersion(latestTag);
                wxCalChart::GetGlobalConfig().FlushWriteQueue();
            }
            if (res == wxID_OK) {
                wxLaunchDefaultBrowser("https://github.com/calband/calchart/releases/latest");
            }
        });
    });

    // Get the file history
    auto* config = wxConfigBase::Get();
    config->SetPath("/FileHistory");
    mDocManager->FileHistoryLoad(*config);

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
    wxCalChart::GetGlobalConfig().FlushWriteQueue();
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
