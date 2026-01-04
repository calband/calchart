#pragma once
/*
 * CalChartApp.h
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

#include <memory>
#include <wx/wx.h>

// Forward declarations
class CalChartApp;
class HostAppInterface;
class HelpManager;
class ViewerServer;
class wxDocManager;
class wxPrintDialogData;
class CalChartLogTarget;
namespace CalChart {
class ShowMode;
class CircularLogBuffer;
}

DECLARE_APP(CalChartApp)

// Define a new application
class CalChartApp : public wxApp {
public:
    virtual bool OnInit() override;
    int OnExit() override;
    auto OnExceptionInMainLoop() -> bool override;

    void OpenFile(wxString const& fileName);
    void OpenFileOnHost(wxString const& filename);
#if defined(__APPLE__) && (__APPLE__)
    virtual void MacOpenFile(wxString const& fileName) override;
    virtual void MacOpenFiles(wxArrayString const& fileNames) override;
#endif // defined(__APPLE__) && (__APPLE__)

    // the global help system:
    HelpManager& GetGlobalHelpManager();
    wxPrintDialogData& GetGlobalPrintDialog();

    // Get a copy of the global log buffer for bug reporting
    CalChart::CircularLogBuffer GetLogBuffer() const;

    // Get the viewer server
    ViewerServer& GetViewerServer();

private:
    void ProcessArguments();

    void InitAppAsServer();
    void InitAppAsClient();
    void ExitAppAsServer();
    void ExitAppAsClient();

    wxDocManager* mDocManager{};
    std::unique_ptr<HelpManager> mHelpManager;
    std::unique_ptr<HostAppInterface> mHostInterface;
    std::unique_ptr<wxPrintDialogData> mPrintDialogData;
    std::unique_ptr<ViewerServer> mViewerServer;
    CalChartLogTarget* mLogTarget = nullptr; // Owned by wxLog, don't delete
};
