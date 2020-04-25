#pragma once
/*
 * CustomListViewPanel.h
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

#include <set>
#include <vector>
#include <wx/wx.h>

// Drawable Cells are anything that is a small drawing surface
class DrawableCell {
public:
    DrawableCell() = default;
    virtual ~DrawableCell() = default;
    virtual void DrawToDC(wxDC&) = 0;
    virtual int Height() const = 0;
    virtual int Width() const = 0;
    virtual void OnClick(wxPoint const&) = 0;
    virtual void SetHighlight(void const*) = 0;
};

// the idea is that we will have a collection of smaller cells, that can be moved around.
// View for linking CalChartDoc with ContinuityBrowser
class CustomListViewPanel : public wxScrolledWindow {
    using super = wxScrolledWindow;

public:
    // Basic functions
    CustomListViewPanel(wxWindow* parent,
        wxWindowID winid = wxID_ANY,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxScrolledWindowStyle,
        const wxString& name = wxPanelNameStr);
    virtual ~CustomListViewPanel() = default;
    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnLeftDoubleClick(wxMouseEvent& event);
    void OnLeftDownMouseEvent(wxMouseEvent& event);
    void OnLeftUpMouseEvent(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void SetSelection(int which) { m_selected = static_cast<size_t>(which); }
    auto GetSelection() const { return m_selected; }
    void SetCells(std::vector<std::unique_ptr<DrawableCell>> cells);
    void SetHighlight(void const* highlight);

private:
    virtual void OnNewEntry(int cell);
    virtual void OnEditEntry(int cell);
    virtual void OnDeleteEntry(int cell);
    virtual void OnMoveEntry(int start_cell, int end_cell);
    size_t WhichCell(wxPoint const& point) const;
    int HeightToCell(int which) const;

    std::vector<std::unique_ptr<DrawableCell>> mCells;
    wxPoint m_firstPress;
    wxPoint m_lastLocation;
    size_t m_selected;
    bool m_dragging;

    DECLARE_EVENT_TABLE()
};
