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

#include <wx/wx.h>

// CalChartApp represents the wxWidgets App for CalChart.  The document manager creates
// the actual CalChart Document instances.  This just serves the purpose of being the first
// thing that runs, holds much of the top level logic.

class CalChartApp;
class HostAppInterface;
class wxHtmlHelpController;
class wxDocManager;
class wxPrintDialogData;
namespace CalChart {
class ShowMode;
}

DECLARE_APP(CalChartApp)

// Define a new application
class CalChartApp : public wxApp
{
public:
    virtual bool OnInit() override;
    int OnExit() override;

    void OpenFile(wxString const& fileName);
    void OpenFileOnHost(wxString const& filename);
#if defined(__APPLE__) && (__APPLE__)
    virtual void MacOpenFile(wxString const& fileName) override;
    virtual void MacOpenFiles(wxArrayString const& fileNames) override;
#endif // defined(__APPLE__) && (__APPLE__)

    // the global help system:
    wxHtmlHelpController& GetGlobalHelpController();
    wxPrintDialogData& GetGlobalPrintDialog();

private:
    void ProcessArguments();

    void InitAppAsServer();
    void InitAppAsClient();
    void ExitAppAsServer();
    void ExitAppAsClient();

    wxDocManager* mDocManager{};
    std::unique_ptr<wxHtmlHelpController> mHelpController;
    std::unique_ptr<HostAppInterface> mHostInterface;
    std::unique_ptr<wxPrintDialogData> mPrintDialogData;
};
