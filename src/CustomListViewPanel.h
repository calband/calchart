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

#include "CalChartDrawCommand.h"
#include <memory>
#include <set>
#include <vector>
#include <wx/wx.h>

// Drawable Cells are anything that is a small drawing surface
class DrawableCell {
public:
    DrawableCell() = default;
    virtual ~DrawableCell() = default;
    [[nodiscard]] virtual auto GetDrawCommands(wxDC&) -> std::vector<CalChart::DrawCommand> = 0;
    [[nodiscard]] virtual auto Height() const -> int = 0;
    [[nodiscard]] virtual auto Width() const -> int = 0;
    virtual void OnClick(wxDC&, wxPoint const&) = 0;
    virtual void SetHighlight(void const*) = 0;
};

// A custom list view is a drawable item that has a number of cells in it that
// can be manipulated.
// the idea is that we will have a collection of smaller cells, that can be moved around.
// View for linking CalChartDoc with ContinuityBrowser
class CustomListViewPanel : public wxScrolledWindow {
    using super = wxScrolledWindow;
    DECLARE_EVENT_TABLE()

public:
    // Basic functions
    CustomListViewPanel(wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxScrolledWindowStyle, wxString const& name = wxPanelNameStr);
    ~CustomListViewPanel() override = default;

    void SetSelection(std::optional<size_t> which) { m_selected = which; }
    auto GetSelection() const { return m_selected; }
    void SetCells(std::vector<std::unique_ptr<DrawableCell>> cells);
    void SetHighlight(void const* highlight);

private:
    // events
    void OnPaint(wxPaintEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnLeftDoubleClick(wxMouseEvent& event);
    void OnLeftDownMouseEvent(wxMouseEvent& event);
    void OnLeftUpMouseEvent(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnEraseBackground(wxEraseEvent& event);

    // internals
    virtual void OnNewEntry(int cell);
    virtual void OnEditEntry(int cell);
    virtual void OnDeleteEntry(int cell);
    virtual void OnMoveEntry(int start_cell, int end_cell);
    [[nodiscard]] auto WhichCell(wxPoint const& point) const -> std::optional<size_t>;
    auto HeightToCell(int which) const -> int;

    std::vector<std::unique_ptr<DrawableCell>> mCells;
    wxPoint m_firstPress;
    wxPoint m_lastLocation;
    std::optional<size_t> m_selected;
    bool m_dragging;
};
