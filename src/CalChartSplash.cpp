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
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartPreferences.h"
#include "ConfigurationDebugDialog.h"
#include "StackDrawPlayground.h"
#include "SystemConfiguration.h"
#include "basic_ui.h"
#include "ccvers.h"

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

BEGIN_EVENT_TABLE(CalChartSplash, wxDocParentFrame)
EVT_MENU(wxID_ABOUT, CalChartSplash::OnCmdAbout)
EVT_MENU(wxID_HELP, CalChartSplash::OnCmdHelp)
EVT_MENU(wxID_PREFERENCES, CalChartSplash::OnCmdPreferences)
END_EVENT_TABLE()

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
    const static wxString kImageDir = wxT("CalChart.app/Contents/Resources/calchart.png");
#else
    const static wxString kImageDir = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath().Append(PATH_SEPARATOR wxT("resources") PATH_SEPARATOR wxT("calchart.png"));
#endif
    if (image.LoadFile(kImageDir)) {
        if (size != wxDefaultSize) {
            image = image.Scale(size.GetX(), size.GetY(), wxIMAGE_QUALITY_HIGH);
        }
        return wxBitmap{ image };
    }
    return wxBitmap(BITMAP_NAME(calchart));
}

CalChartSplash::CalChartSplash(wxDocManager* manager, wxFrame* frame, wxString const& title, CalChart::Configuration& config)
    : super(manager, frame, wxID_ANY, title)
    , mConfig(config)
{
    // Give it an icon
    SetBandIcon(this);

    wxUI::MenuBar{
        wxUI::Menu{
            "&File",
            wxUI::Item{ wxID_NEW, "&New Show\tCTRL-N", "Create a new show" },
            wxUI::Item{ wxID_OPEN, "&Open...\tCTRL-O", "Load a saved show" },
            wxUI::Item{ wxID_PREFERENCES, "&Preferences\tCTRL-,", [this](wxCommandEvent& event) { OnCmdPreferences(event); } },
            wxUI::Item{ wxID_EXIT, "&Quit\tCTRL-Q", "Quit CalChart" },
        },
        wxUI::Menu{
            "&Debug",
            wxUI::Item{ "Stack Draw Playground", [this] {
                           StackDrawPlayground(this).ShowModal();
                       } },
            wxUI::Item{ "Debug Configuration", [this] {
                           ConfigurationDebug(this, wxConfigBase::Get()).ShowModal();
                       } },
        },
        wxUI::Menu{
            "&Help",
            wxUI::Item{ wxID_ABOUT, "&About CalChart...", "Information about the program", [] {
                           About();
                       } },
            wxUI::Item{ wxID_HELP, "&Help on CalChart...\tCTRL-H", "Help on using CalChart ", [] {
                           Help();
                       } },
        }
    }.fitTo(this);

    // A nice touch: a history of files visited. Use this menu.
    auto which = GetMenuBar()->FindMenu("&File");
    manager->FileHistoryUseMenu(GetMenuBar()->GetMenu(which));

    SetDropTarget(new CalChartSplashDropTarget(manager));

    // create a sizer and populate
    using namespace wxUI;
    auto fontTitle = wxFontInfo(GetTitleFontSize()).Family(wxFONTFAMILY_SWISS);
    auto fontSubTitle = wxFontInfo(GetSubTitleFontSize()).Family(wxFONTFAMILY_SWISS);
    auto fontSubSubTitle = wxFontInfo(GetSubSubTitleFontSize()).Family(wxFONTFAMILY_SWISS);
    VSizer{
        BasicSizerFlags(),
        Bitmap{ BitmapWithBandIcon(GetLogoSize()) }.withFlags(ExpandSizerFlags()),
        Text{ "CalChart " CC_VERSION }
            .withFont(fontTitle),
        Line{}
            .withSize({ GetLogoLineSize(), -1 }),
        HSizer{
            Hyperlink{ "Check for latest.", "https://sourceforge.net/projects/calchart/" }
                .withFont(fontSubTitle)
                .withStyle(wxHL_DEFAULT_STYLE),
            Text{ "        " }
                .withFont(fontSubTitle),
            Hyperlink{ "Report an issue.", "https://github.com/calband/calchart/issues/new" }
                .withFont(fontSubTitle)
                .withStyle(wxHL_DEFAULT_STYLE),
        },
        Line{}
            .withSize({ GetLogoLineSize(), -1 }),
        Text{ "Authors: Gurk Meeker, Richard Michael Powell" }
            .withFont(fontSubTitle),
        Text{ "Contributors: Brandon Chinn, Kevin Durand,\nNoah Gilmore, David Strachan-Olson, Allan Yu" }
            .withFont(fontSubSubTitle),
    }
        .fitTo(this);

    Show(true);
}

void CalChartSplash::OnCmdAbout(wxCommandEvent&) { CalChartSplash::About(); }

void CalChartSplash::OnCmdHelp(wxCommandEvent&) { CalChartSplash::Help(); }

void CalChartSplash::About()
{
    // clang-format off
    (void)wxMessageBox(
        "CalChart " CC_VERSION "\n"
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
    wxGetApp().GetGlobalHelpController().LoadFile();
    wxGetApp().GetGlobalHelpController().DisplayContents();
}

void CalChartSplash::OnCmdPreferences(wxCommandEvent&)
{
    // here's where we flush out the configuration.
    auto localConfig = mConfig.Copy();
    if (CalChartPreferences(this, localConfig).ShowModal() == wxID_OK) {
        wxCalChart::AssignConfig(localConfig);
    }
}
