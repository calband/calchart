#pragma once
/*
 * PrintPostScriptDialog.h
 * Dialox box for printing postscript
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

#include "platconf.h"

#include <set>
#include <wx/dialog.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class CalChartDoc;
namespace CalChart {
class Configuration;
}

class PrintPostScriptDialog : public wxDialog {
    DECLARE_CLASS(PrintPostScriptDialog)
    DECLARE_EVENT_TABLE()

public:
    PrintPostScriptDialog(
        const CalChartDoc* doc,
        CalChart::Configuration& config,
        wxFrame* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Print Dialog"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);
    ~PrintPostScriptDialog();

    void Init();

    bool Create(const CalChartDoc* show,
        wxFrame* parent,
        wxWindowID id = wxID_ANY,
        const wxString& caption = wxT("Print Dialog"),
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU);

    void CreateControls();

    void ShowPrintSelect(wxCommandEvent&);
    // because we modify setting, we need some way to reset them
    void ResetDefaults(wxCommandEvent&);

    // use these to get and set default values
    virtual bool TransferDataToWindow();
    virtual bool TransferDataFromWindow();

    // to print a show, call this function
    void PrintShow();

private:
    CalChartDoc const* mShow;
    CalChart::Configuration& mConfig;
    wxUI::TextCtrl::Proxy text_cmd;
#ifdef PRINT__RUN_CMD
    wxUI::TextCtrl::Proxy text_opts;
    wxUI::TextCtrl::Proxy text_view_cmd;
    wxUI::TextCtrl::Proxy text_view_opts;
#endif
    wxUI::TextCtrl::Proxy text_x;
    wxUI::TextCtrl::Proxy text_y;
    wxUI::TextCtrl::Proxy text_width;
    wxUI::TextCtrl::Proxy text_height;
    wxUI::TextCtrl::Proxy text_length;
    wxUI::TextCtrl::Proxy text_minyards;
    wxUI::RadioBox::Proxy radio_orient;
    wxUI::RadioBox::Proxy radio_method;
    wxUI::CheckBox::Proxy check_cont;
    wxUI::CheckBox::Proxy check_pages;
    wxUI::CheckBox::Proxy check_overview;
    std::set<size_t> mIsSheetPicked;
};
