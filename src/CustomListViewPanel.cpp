/*
 * CustomListViewPanel.cpp
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

#include "CustomListViewPanel.h"
#include "confgr.h"
#include <wx/dcbuffer.h>

#include <numeric>

BEGIN_EVENT_TABLE(CustomListViewPanel, wxScrolledWindow)
EVT_PAINT(CustomListViewPanel::OnPaint)
EVT_CHAR(CustomListViewPanel::OnChar)
EVT_LEFT_DOWN(CustomListViewPanel::OnLeftDownMouseEvent)
EVT_LEFT_UP(CustomListViewPanel::OnLeftUpMouseEvent)
EVT_LEFT_DCLICK(CustomListViewPanel::OnLeftDoubleClick)
EVT_MOTION(CustomListViewPanel::OnMouseMove)
EVT_ERASE_BACKGROUND(CustomListViewPanel::OnEraseBackground)
END_EVENT_TABLE()

// Define a constructor for field canvas
CustomListViewPanel::CustomListViewPanel(wxWindow* parent,
    wxWindowID winid,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name)
    : super(parent, winid, pos, size, style, name)
    , m_selected(-1)
    , m_dragging(false)
{
}

// Define the repainting behaviour
void CustomListViewPanel::SetCells(std::vector<std::unique_ptr<DrawableCell>> cells)
{
    mCells = std::move(cells);
    auto total_y = std::accumulate(mCells.begin(), mCells.end(), 0, [](auto&& a, auto&& b) {
        return a + b->Height();
    });
    auto max_x_elem = std::max_element(mCells.begin(), mCells.end(), [](auto&& a, auto&& b) {
        return a->Width() < b->Width();
    });
    // give a slight little padding on max_x
    auto max_x = max_x_elem != mCells.end() ? ((*max_x_elem)->Width() * 1.1) : 0;

    SetVirtualSize(wxSize{ int(max_x), total_y });
    SetScrollRate(1, 1);
}

void CustomListViewPanel::SetHighlight(void const* highlight)
{
    for (auto&& i : mCells) {
        i->SetHighlight(highlight);
    }
}

// Define the repainting behaviour
void CustomListViewPanel::OnPaint(wxPaintEvent& event)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(*wxLIGHT_GREY_BRUSH);
    dc.SetBrush(*wxWHITE_BRUSH);
    dc.SetPen(*wxBLACK_PEN);
    dc.Clear();
    // do something special for selected; find out it's height;
    auto selected_height = (m_selected < mCells.size()) ? mCells.at(m_selected)->Height() : 0;
    // create a series of bitmaps, and draw them one at a time
    auto offset = GetViewStart();
    auto x_start = -offset.x;
    auto y_start = -offset.y;
    // draw the non-selected first;
    for (auto i = 0u; i < mCells.size(); ++i) {
        dc.DrawLine(x_start, y_start, x_start + 0xFFFF, y_start);
        if (m_selected == i) {
            if (!m_dragging || (m_lastLocation.y >= y_start && m_lastLocation.y < (y_start + selected_height))) {
                y_start += selected_height;
            }
            continue;
        }
        if ((m_selected < mCells.size()) && m_dragging && (m_lastLocation.y >= y_start && m_lastLocation.y < (y_start + selected_height))) {
            // make a hole for the thing
            y_start += selected_height;
        }
        dc.SetDeviceOrigin(x_start, y_start);
        mCells.at(i)->DrawToDC(dc);
        dc.SetDeviceOrigin(0, 0);
        y_start += mCells.at(i)->Height();
    }
    dc.DrawLine(x_start, y_start, x_start + 0xFFFF, y_start);

    // draw the selected now
    x_start = -offset.x;
    y_start = -offset.y;
    for (auto i = 0u; (m_selected < mCells.size()) && i < mCells.size(); ++i) {
        if (i != m_selected) {
            y_start += mCells.at(i)->Height();
            continue;
        }
        auto&& current_brush = dc.GetBrush();
        dc.SetBrush(wxBrush(wxT("GREY")));
        auto y_delta = m_lastLocation.y - m_firstPress.y;
        auto height = mCells.at(i)->Height();
        dc.DrawRectangle(x_start, y_start + y_delta, 0xFFFF, height);
        dc.SetBrush(current_brush);
        dc.SetDeviceOrigin(x_start, y_start + y_delta);
        mCells.at(i)->DrawToDC(dc);
        dc.SetDeviceOrigin(0, 0);
        y_start += mCells.at(i)->Height();
    }
}

size_t CustomListViewPanel::WhichCell(wxPoint const& point) const
{
    auto y_start = 0;
    auto which_cell = 0ul;
    for (; which_cell < mCells.size(); ++which_cell) {
        if ((point.y >= y_start) && (point.y < (y_start + mCells.at(which_cell)->Height()))) {
            break;
        }
        y_start += mCells.at(which_cell)->Height();
    }
    return which_cell;
}

int CustomListViewPanel::HeightToCell(int which) const
{
    auto iter = mCells.begin();
    auto result = 0;
    while (which-- && iter != mCells.end()) {
        result += (*iter)->Height();
        ++iter;
    }
    return result;
}

void CustomListViewPanel::OnLeftDownMouseEvent(wxMouseEvent& event)
{
    m_firstPress = m_lastLocation = event.GetPosition();
    //    printf("Mouse down @(%d, %d)\n", event.GetPosition().x, event.GetPosition().y);
    // which cell would this be?
    auto which_cell = WhichCell(event.GetPosition());
    m_selected = which_cell;
    m_dragging = true;
    if (which_cell < mCells.size()) {
        auto mouse_click = event.GetPosition();
        mouse_click.y -= HeightToCell(which_cell);
        mCells.at(which_cell)->OnClick(mouse_click);
    }
    Refresh();
}

void CustomListViewPanel::OnLeftDoubleClick(wxMouseEvent& event)
{
    //    printf("Mouse down @(%d, %d)\n", event.GetPosition().x, event.GetPosition().y);
    // which cell would this be?
    auto which_cell = WhichCell(event.GetPosition());
    if (which_cell < mCells.size()) {
        OnEditEntry(which_cell);
    }
    Refresh();
}

void CustomListViewPanel::OnLeftUpMouseEvent(wxMouseEvent& event)
{
    auto starting_cell = WhichCell(m_firstPress);
    m_lastLocation = m_firstPress;
    //    printf("Mouse up @(%d, %d)\n", event.GetPosition().x, event.GetPosition().y);
    // which cell would this be?
    auto which_cell = WhichCell(event.GetPosition());
    if (starting_cell != which_cell) {
        // reorder.
        OnMoveEntry(starting_cell, which_cell);
    }
    m_selected = which_cell;
    m_dragging = false;
    Refresh();
}

// Define the repainting behaviour
void CustomListViewPanel::OnMouseMove(wxMouseEvent& event)
{
    if (event.LeftIsDown() && event.GetPosition().y >= 0) {
        m_lastLocation = event.GetPosition();
    }
    Refresh();
}

// Define the repainting behaviour
void CustomListViewPanel::OnChar(wxKeyEvent& event)
{
    // ignore keypresses if dragging:
    if (m_dragging)
        return;
    if (event.GetKeyCode() == WXK_UP && m_selected < mCells.size() && m_selected > 0) {
        --m_selected;
    } else if (event.GetKeyCode() == WXK_DOWN && m_selected < (mCells.size() - 1)) {
        ++m_selected;
    } else if (event.GetKeyCode() == WXK_RETURN) {
        // with the shift key we go one above.
        // -1 means the end.
        // empty means 0 all the time
        //
        if (!event.ShiftDown()) {
            ++m_selected;
        }
        OnNewEntry(std::min(mCells.size(), m_selected));
    } else if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DELETE || event.GetKeyCode() == WXK_BACK) {
        OnDeleteEntry(m_selected);
    } else {
        event.Skip();
    }
    Refresh();
}

void CustomListViewPanel::OnNewEntry(int cell)
{
}

void CustomListViewPanel::OnEditEntry(int cell)
{
}

void CustomListViewPanel::OnDeleteEntry(int cell)
{
}

void CustomListViewPanel::OnMoveEntry(int start_cell, int end_cell)
{
}

void CustomListViewPanel::OnEraseBackground(wxEraseEvent& event) {}
