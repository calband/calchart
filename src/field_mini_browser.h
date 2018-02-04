#pragma once
/*
 * field_mini_browser
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

#include "calchartdoc.h"

#include <wx/dialog.h>
#include <wx/docview.h>

class FieldBrowserView;
class FieldBrowserPerCont;

class FieldBrowser : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    FieldBrowser(CalChartDoc* dcr, wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxScrolledWindowStyle,
        const wxString& name = wxPanelNameStr);
    virtual ~FieldBrowser() override = default;

    void OnCmdHelp(wxCommandEvent& event);
    virtual void OnUpdate(); // Refresh all window controls

private:
    void CreateControls();
    void OnChar(wxKeyEvent& event);
    void OnPaint(wxPaintEvent& event);
    wxSize SizeOfOneCell() const;
    int WhichCell(wxPoint const& p) const;
    void HandleKey(wxKeyEvent& event);
    void HandleMouseDown(wxMouseEvent& event);

    CalChartDoc* mDoc;
    std::unique_ptr<FieldBrowserView> mView;
    std::vector<FieldBrowserPerCont*> mPerCont;

    const int x_left_padding;
    const int x_right_padding;
    const int y_upper_padding;
    const int y_name_size;
    const int y_name_padding;

    DECLARE_EVENT_TABLE()
};
