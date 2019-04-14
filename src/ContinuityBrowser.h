/*
 * ContinuityBrowser
 * Header for continuity editors
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

#pragma once

#include <wx/docview.h>

class CalChartDoc;
class ContinuityBrowserView;
class ContinuityBrowserPerCont;

class ContinuityBrowser : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    ContinuityBrowser(CalChartDoc* dcr, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxScrolledWindowStyle,
        const wxString& name = wxPanelNameStr);
    virtual ~ContinuityBrowser() override = default;

    void OnCmdHelp(wxCommandEvent& event);
    virtual void Update() override; // Refresh all window controls

private:
    void CreateControls();
    void OnChar(wxKeyEvent& event);

    CalChartDoc* mDoc;
    std::unique_ptr<ContinuityBrowserView> mView;
    std::vector<ContinuityBrowserPerCont*> mPerCont;

    DECLARE_EVENT_TABLE()
};
