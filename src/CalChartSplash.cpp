/*
 * CalChartSplash.cpp
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

#include "CalChartSplash.h"
#include "CalChartApp.h"
#include "CalChartPreferences.h"
#include "basic_ui.h"
#include "ccvers.h"

#include <wx/dnd.h>
#include <wx/html/helpctrl.h>

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

CalChartSplash::CalChartSplash(wxDocManager* manager, wxFrame* frame, wxString const& title)
    : super(manager, frame, wxID_ANY, title)
{
    // Give it an icon
    SetBandIcon(this);

    auto file_menu = new wxMenu;
    file_menu->Append(wxID_NEW, wxT("&New Show\tCTRL-N"), wxT("Create a new show"));
    file_menu->Append(wxID_OPEN, wxT("&Open...\tCTRL-O"), wxT("Load a saved show"));
    file_menu->Append(wxID_PREFERENCES, wxT("&Preferences\tCTRL-,"));
    file_menu->Append(wxID_EXIT, wxT("&Quit\tCTRL-Q"), wxT("Quit CalChart"));

    // A nice touch: a history of files visited. Use this menu.
    manager->FileHistoryUseMenu(file_menu);

    auto help_menu = new wxMenu;
    help_menu->Append(wxID_ABOUT, wxT("&About CalChart..."), wxT("Information about the program"));
    // this comes up as a leak on Mac?
    help_menu->Append(wxID_HELP, wxT("&Help on CalChart...\tCTRL-H"), wxT("Help on using CalChart"));

    auto menu_bar = new wxMenuBar;
    menu_bar->Append(file_menu, wxT("&File"));
    menu_bar->Append(help_menu, wxT("&Help"));
    SetMenuBar(menu_bar);

    SetDropTarget(new CalChartSplashDropTarget(manager));

    // create a sizer and populate
    SetSizer(VStack([this](auto sizer) {
        // add a horizontal bar to make things clear:
        AddToSizerExpand(sizer, BitmapWithBandIcon(this, GetLogoSize()));
        AddToSizerBasic(sizer, TextStringWithSize(this, "CalChart " CC_VERSION, GetTitleFontSize()));
        AddToSizerBasic(sizer, LineWithLength(this, GetLogoLineSize()));

        HStack(sizer, BasicSizerFlags(), [this](auto sizer) {
            AddToSizerBasic(sizer, LinkStringWithSize(this, "Check for latest.", "https://sourceforge.net/projects/calchart/", GetSubTitleFontSize()));
            AddToSizerBasic(sizer, TextStringWithSize(this, "        ", GetSubTitleFontSize()));
            AddToSizerBasic(sizer, LinkStringWithSize(this, "Report an issue.", "https://github.com/calband/calchart/issues/new", GetSubTitleFontSize()));
        });
        AddToSizerBasic(sizer, LineWithLength(this, GetLogoLineSize()));
        AddToSizerBasic(sizer, TextStringWithSize(this, "Authors: Gurk Meeker, Richard Michael Powell", GetSubTitleFontSize()));
        AddToSizerBasic(sizer, TextStringWithSize(this, "Contributors: Brandon Chinn, Kevin Durand,\nNoah Gilmore, David Strachan-Olson, Allan Yu", GetSubSubTitleFontSize()));
    }));

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
    CalChartPreferences dialog1(this);
    if (dialog1.ShowModal() == wxID_OK) {
    }
}
