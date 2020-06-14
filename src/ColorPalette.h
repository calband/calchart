#pragma once
/*
 * ColorPalettePanel
 * For selecting color palette
 */

/*
   Copyright (C) 1995-2012  Richard Michael Powell

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

class CalChartView;
// holds an instance of animation for the reference to draw.
class ColorPalettePanel : public wxControl {
    using super = wxControl;
    wxDECLARE_EVENT_TABLE();

public:
    ColorPalettePanel(wxWindow* parent, wxWindowID winid = wxID_ANY);
    ~ColorPalettePanel() override = default;

    void OnUpdate(); // Refresh from the View
    void SetView(CalChartView* view) { mView = view; }
    auto GetView() const { return mView; }

private:
    void Init();

    // Event Handlers
    void OnPaint(wxPaintEvent& event);
    void OnLeftClick(wxMouseEvent& event);
    void OnLeftDoubleClick(wxMouseEvent& event);

    // Internals
    int WhichBox(wxPoint const& where);

    CalChartView* mView{};
};
