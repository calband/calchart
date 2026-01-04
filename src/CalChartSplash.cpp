/*
 * CalChartSplash.cpp
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

#include "CalChartSplash.h"
#include "BugReportDialog.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartPreferences.h"
#include "ConfigurationDebugDialog.h"
#include "HelpDialog.hpp"
#include "StackDrawPlayground.h"
#include "SystemConfiguration.h"
#include "basic_ui.h"
#include "ccvers.h"

#include <wx/setup.h>
#if wxUSE_WEBVIEW
#include "WebViewDemoDialog.h"
#endif

#include <wx/dnd.h>
#include <wx/filename.h>
#include <wx/html/helpctrl.h>
#include <wx/icon.h>
#include <wx/stdpaths.h>
#include <wxUI/Hyperlink.hpp>
#include <wxUI/wxUI.hpp>

#include "calchart.xbm"
#include "calchart.xpm"
#include "platconf.h"

IMPLEMENT_CLASS(CalChartSplash, wxDocParentFrame)

struct CalChartSplashDropTarget : public wxFileDropTarget {
public:
    CalChartSplashDropTarget(wxDocManager* manager)
        : mManager(manager)
    {
    }
    virtual bool OnDropFiles(wxCoord, wxCoord, wxArrayString const& filenames) override
    {
        for (auto&& filename : filenames) {
            mManager->CreateDocument(filename, wxDOC_SILENT);
        }
        return true;
    }

private:
    wxDocManager* mManager;
};

auto BitmapWithBandIcon(wxSize const& size)
{

    wxImage image;
#if defined(__APPLE__) && (__APPLE__)
    const static auto kImageDir = wxStandardPaths::Get().GetResourcesDir().Append("/calchart.png");
#else
    const static auto kImageDir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR wxT("resources") PATH_SEPARATOR wxT("calchart.png"));
#endif
    if (image.LoadFile(kImageDir)) {
        if (size != wxDefaultSize) {
            image = image.Scale(size.GetX(), size.GetY(), wxIMAGE_QUALITY_HIGH);
        }
        return wxBitmap{ image };
    }
    return wxBitmap(BITMAP_NAME(calchart));
}

CalChartSplash::CalChartSplash(wxDocManager* manager, wxFrame* frame, std::string const& title, CalChart::Configuration& config)
    : super(manager, frame, wxID_ANY, title)
    , mConfig(config)
{
    // Give it an icon
    SetBandIcon(this);

    wxUI::MenuProxy fileMenu;
    wxUI::MenuBar{
        wxUI::Menu{
            "&File",
            wxUI::Item{ wxID_NEW, "&New Show\tCTRL-N", "Create a new show" },
            wxUI::Item{ wxID_OPEN, "&Open...\tCTRL-O", "Load a saved show" },
            wxUI::Item{ wxID_PREFERENCES, "&Preferences\tCTRL-,", [this] { Preferences(); } },
            wxUI::Item{ wxID_EXIT, "&Quit\tCTRL-Q", "Quit CalChart" },
        }
            .withProxy(fileMenu),
        wxUI::Menu{
            "&Debug",
            wxUI::Item{ "Stack Draw Playground", [this] {
                           StackDrawPlayground(this).ShowModal();
                       } },
            wxUI::Item{ "Debug Configuration", [this] {
                           ConfigurationDebug(this, wxConfigBase::Get()).ShowModal();
                       } },
#if wxUSE_WEBVIEW
            wxUI::Item{ "wxWebView Demo", [this] {
                           WebViewDemoDialog(this).ShowModal();
                       } },
#endif
        },
        wxUI::Menu{
            "&Help",
            wxUI::Item{ wxID_ABOUT, "&About CalChart...", "Information about the program", [] {
                           About();
                       } },
            wxUI::Item{ wxID_HELP, "&Help on CalChart...\tCTRL-H", "Help on using CalChart ", [] {
                           Help();
                       } },
            wxUI::Item{ "Report a &Bug...\tCTRL-SHIFT-B", "Report a bug to GitHub", [] {
                           ReportBug();
                       } },
        }
    }.fitTo(this);

    // A nice touch: a history of files visited. Use this menu.
    manager->FileHistoryUseMenu(fileMenu.control());

    SetDropTarget(new CalChartSplashDropTarget(manager));

    // create a sizer and populate
    auto fontTitle = wxFontInfo(GetTitleFontSize()).Family(wxFONTFAMILY_SWISS);
    auto fontSubTitle = wxFontInfo(GetSubTitleFontSize()).Family(wxFONTFAMILY_SWISS);
    auto fontSubSubTitle = wxFontInfo(GetSubSubTitleFontSize()).Family(wxFONTFAMILY_SWISS);
    wxUI::VSizer{
        BasicSizerFlags(),
        wxUI::Bitmap{ BitmapWithBandIcon(GetLogoSize()) }.withFlags(ExpandSizerFlags()),
        wxUI::Text{ "CalChart " CC_VERSION }
            .withFont(fontTitle),
        wxUI::Text{ std::string{ "Built with: " } + wxString{ wxVERSION_STRING }.ToStdString() }
            .withFont(fontSubTitle),
        wxUI::Line{}
            .withSize({ GetLogoLineSize(), -1 }),
        wxUI::HSizer{
            wxUI::Hyperlink{ "Check for latest.", "https://sourceforge.net/projects/calchart/" }
                .withFont(fontSubTitle)
                .withStyle(wxHL_DEFAULT_STYLE),
            wxUI::Text{ "        " }
                .withFont(fontSubTitle),
            wxUI::Button{ "Report a Bug." }
                .withFont(fontSubTitle)
                .withStyle(wxHL_DEFAULT_STYLE)
                .bind([]() {
                    ReportBug();
                }),
        },
        wxUI::Line{}
            .withSize({ GetLogoLineSize(), -1 }),
        wxUI::Text{ "Authors: Gurk Meeker, Richard Michael Powell" }
            .withFont(fontSubTitle),
        wxUI::Text{ "Contributors: Brandon Chinn, Kevin Durand,\nNoah Gilmore, David Strachan-Olson, Allan Yu" }
            .withFont(fontSubSubTitle),
    }
        .fitTo(this);
    Show(true);
}

void CalChartSplash::About()
{
    // clang-format off
    (void)wxMessageBox(
        "CalChart " CC_VERSION "\n"
        "Built with: " wxVERSION_STRING "\n"
        "Authors: Gurk Meeker, Richard Michael Powell\n"
        "\n"
        "Contributors: Brandon Chinn, Kevin Durand, Noah Gilmore, David Strachan-Olson, Allan Yu\n"
        "http://calchart.sourceforge.net\n"
        "Copyright (c) 1994-2019\n"
        "\n"
        "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
        "\n"
        "This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n"
        "\n"
        "You should have received a copy of the GNU General Public License along with this program.  If not, see <http://www.gnu.org/licenses/>.\n"
        "\n"
        "Report issues to:\nhttps://github.com/calband/calchart/issues/new\n"
        "\n"
        "Compiled on " __TDATE__ " at " __TTIME__,
        "About CalChart");
    // clang-format on
}

void CalChartSplash::Help()
{
#if wxUSE_WEBVIEW
    // Create and show the help dialog (modeless)
    wxWindow* parent = wxGetActiveWindow();
    if (!parent) {
        parent = nullptr; // Will use the app's top-level window
    }
    auto* helpDialog = new HelpDialog(parent, wxGetApp().GetGlobalHelpManager());
    helpDialog->DisplayIndex();
    helpDialog->Show();
    // Dialog is modeless and will delete itself on close
#else
    wxMessageBox(
        "Help system is not available.\n\n"
        "This build was compiled without wxWebView support.\n"
        "Please refer to the online documentation at:\n"
        "https://github.com/calband/calchart",
        "Help Not Available",
        wxOK | wxICON_INFORMATION);
#endif
}

void CalChartSplash::ReportBug()
{
    // Create and show the bug report dialog
    BugReportDialog dialog(wxCalChart::GetGlobalConfig(),
        nullptr, dynamic_cast<wxFrame*>(wxGetApp().GetTopWindow()));
    dialog.ShowModal();
}

void CalChartSplash::Preferences()
{
    // here's where we flush out the configuration.
    auto localConfig = mConfig.Copy();
    if (CalChartPreferences(this, localConfig).ShowModal() == wxID_OK) {
        wxCalChart::AssignConfig(localConfig);
    }
}
