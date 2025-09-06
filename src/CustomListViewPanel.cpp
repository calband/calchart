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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1
#include "CustomListViewPanel.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartRanges.h"
#include <numeric>
#include <wx/dcbuffer.h>

namespace {

auto PrepareToDraw(wxDC& dc)
{
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.Clear();
}

auto CreateCellLines(std::vector<int> const& whereToDraw) -> std::vector<CalChart::Draw::DrawCommand>
{
    if (whereToDraw.empty()) {
        return {};
    }

    auto drawCmds = std::vector{
        CalChart::Draw::withBrush(
            wxCalChart::toBrush(*wxLIGHT_GREY_BRUSH),
            CalChart::Draw::withPen(
                wxCalChart::toPen(*wxBLACK),
                CalChart::Draw::Rectangle(CalChart::Coord(0, 0), CalChart::Coord(0x7FFF, whereToDraw.back()))))
    };
    // TODO:  use std::views::drop_last when available (see https://stackoverflow.com/questions/71689137/what-is-the-best-way-to-drop-last-element-using-c20-ranges)
    CalChart::append(drawCmds,
        whereToDraw | std::views::take(whereToDraw.size() - 1) | std::views::transform([](auto where_y) {
            return CalChart::Draw::Line(0, where_y, 0x7FFF, where_y);
        }));
    return drawCmds;
}

auto DrawNonSelected(wxDC& dc, std::vector<std::unique_ptr<DrawableCell>> const& cells, wxPoint lastLocation, std::optional<size_t> selected, bool dragging) -> std::vector<CalChart::Draw::DrawCommand>
{
    // first generate the heights:
    auto heights = CalChart::Ranges::ToVector<int>(cells | std::views::transform([](auto&& cell) { return cell->Height(); }));
    auto exScanHeights = std::vector<int>{};
    std::exclusive_scan(heights.begin(), heights.end(), std::back_inserter(exScanHeights), 0);
    auto inScanHeights = std::vector<int>{};
    std::inclusive_scan(heights.begin(), heights.end(), std::back_inserter(inScanHeights));
    auto where = std::distance(inScanHeights.begin(), std::lower_bound(inScanHeights.begin(), inScanHeights.end(), lastLocation.y));

    auto drawCmds = CreateCellLines(inScanHeights);

    // do something special for selected; find out it's height;
    auto selected_height = selected ? cells.at(*selected)->Height() : 0;
    // create a series of bitmaps, and draw them one at a time

    // When drawing, you have to modify the placement of the Heights.  If we are dragging "upwards",
    // then the cells "below" the selected cell's last location need to move downwards.  This is done
    // by adding the height of the selected cell.  So if this inclusive scan of the heights before:
    // ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
    // │  30  │  60  │  90  │ 120  │ 150  │ 180  │ 210  │ 240  │
    // └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
    //            ▲             ▲
    //            │             │
    //     lastLocation      selected
    // Then what we would do is add the selected height to the last location up to the selcted location:
    // ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
    // │  30  │  90  │ 120  │ 120  │ 150  │ 180  │ 210  │ 240  │
    // └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
    //            ▲             ▲
    //            │             │
    //     lastLocation      selected
    // If the dragging "downwards", then the cells above selected cell's last location needs to be moved
    // upwards, subtracting the selected height from the cells between the selection and including the last
    // location.  So if this is before:
    // ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
    // │  30  │  60  │  90  │ 120  │ 150  │ 180  │ 210  │ 240  │
    // └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
    //                          ▲                    ▲
    //                          │                    │
    //                       selected         lastLocation
    // Then afterwards:
    // ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
    // │  30  │  60  │  90  │ 120  │ 120  │ 150  │ 180  │ 240  │
    // └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
    //                          ▲                    ▲
    //                          │                    │
    //                       selected         lastLocation
    // Note: because this includes the last location, we have to take care when we may run off the end
    // of the array.
    if (selected && dragging) {
        auto select = static_cast<int>(*selected);
        if (where < select) {
            std::transform(exScanHeights.begin() + where, exScanHeights.begin() + select, exScanHeights.begin() + where, [selected_height](auto num) { return num + selected_height; });
        } else if (where > select) {
            auto which = static_cast<size_t>(where) == cells.size() ? where - 1 : where;
            std::transform(exScanHeights.begin() + select + 1, exScanHeights.begin() + which + 1, exScanHeights.begin() + select + 1, [selected_height](auto num) { return num - selected_height; });
        }
    }

    CalChart::append(drawCmds,
        CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(std::views::iota(0UL, cells.size())
            | std::views::filter([selected](auto which) { return selected != which; })
            | std::views::transform([&dc, &cells, exScanHeights](auto which) {
                  return cells.at(which)->GetDrawCommands(dc) + CalChart::Coord(0, exScanHeights.at(which));
              })
            | std::views::join));
    return drawCmds;
}

auto DrawSelected(wxDC& dc, std::vector<std::unique_ptr<DrawableCell>> const& cells, wxPoint firstPress, wxPoint lastLocation, std::optional<size_t> selected) -> std::vector<CalChart::Draw::DrawCommand>
{
    if (!selected) {
        return {};
    }
    // We figure out where the dragging selected cell would be drawn.  First find out where
    // the basis of the cell would have been using exclusive scan up to and including the selected cell.
    // Then add the offset which would be the delta between the first press and last location.  So:
    // ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┬──────┐
    // │  30  │  30  │  30  │  30  │  30  │  30  │  30  │  30  │
    // └──────┴──────┴──────┴──────┴──────┴──────┴──────┴──────┘
    //           ▲              ▲
    //           │              │
    //    lastLocation       selected
    // Becomes:
    // ┌──────┬──────┬──────┬──────┐
    // │  0   │  30  │  60  │  90  │
    // └──────┴──────┴──────┴──────┘
    //           ▲              ▲
    //           │              │
    //    lastLocation       selected
    // or 90 + (firstPress - lastLocation) = 90 - 60 = 30
    auto heights = CalChart::Ranges::ToVector<int>(cells | std::views::take(*selected + 1) | std::views::transform([](auto&& cell) { return cell->Height(); }));
    auto exScanHeights = std::vector<int>{};
    std::exclusive_scan(heights.begin(), heights.end(), std::back_inserter(exScanHeights), 0);
    auto whereToDraw = exScanHeights.back() + (lastLocation.y - firstPress.y);
    auto height = cells.at(*selected)->Height();

    auto drawCmds = std::vector{
        CalChart::Draw::withBrush(
            CalChart::Brush{ wxCalChart::toColor(wxColour{ "GREY" }) },
            CalChart::Draw::Rectangle(CalChart::Coord{}, CalChart::Coord(0x7FFF, height)))
    };
    CalChart::append(drawCmds, cells.at(*selected)->GetDrawCommands(dc));
    return drawCmds + CalChart::Coord(0, whereToDraw);
}

auto CreateListViewDrawCommands(wxDC& dc, std::vector<std::unique_ptr<DrawableCell>> const& cells, wxPoint firstPress, wxPoint lastLocation, std::optional<size_t> selected, bool dragging, wxPoint offset) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto drawCmds = DrawNonSelected(dc, cells, lastLocation, selected, dragging);
    CalChart::append(drawCmds, DrawSelected(dc, cells, firstPress, lastLocation, selected));
    return drawCmds + CalChart::Coord(-offset.x, -offset.y);
}
}

BEGIN_EVENT_TABLE(CustomListViewPanel, CustomListViewPanel::super)
EVT_PAINT(CustomListViewPanel::OnPaint)
EVT_CHAR(CustomListViewPanel::OnChar)
EVT_LEFT_DOWN(CustomListViewPanel::OnLeftDownMouseEvent)
EVT_LEFT_UP(CustomListViewPanel::OnLeftUpMouseEvent)
EVT_LEFT_DCLICK(CustomListViewPanel::OnLeftDoubleClick)
EVT_MOTION(CustomListViewPanel::OnMouseMove)
EVT_ERASE_BACKGROUND(CustomListViewPanel::OnEraseBackground)
END_EVENT_TABLE()

// Define a constructor for field canvas
CustomListViewPanel::CustomListViewPanel(wxWindow* parent, wxWindowID winid, wxPoint const& pos, wxSize const& size, long style, wxString const& name)
    : super(parent, winid, pos, size, style, name)
    , m_dragging(false)
{
}

// Define the repainting behaviour
void CustomListViewPanel::SetCells(std::vector<std::unique_ptr<DrawableCell>> cells)
{
    mCells = std::move(cells);
    if (m_selected) {
        if (mCells.empty()) {
            m_selected = std::nullopt;
        } else {
            m_selected = std::min(*m_selected, mCells.size() - 1);
        }
    }

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
void CustomListViewPanel::OnPaint(wxPaintEvent&)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);
    PrepareToDraw(dc);
    auto offset = GetViewStart();
    wxCalChart::Draw::DrawCommandList(dc, CreateListViewDrawCommands(dc, mCells, m_firstPress, m_lastLocation, m_selected, m_dragging, offset));
}

auto CustomListViewPanel::WhichCell(wxPoint const& point) const -> std::optional<size_t>
{
    auto y_start = 0;
    auto which_cell = 0ul;
    for (; which_cell < mCells.size(); ++which_cell) {
        if ((point.y >= y_start) && (point.y < (y_start + mCells.at(which_cell)->Height()))) {
            return which_cell;
        }
        y_start += mCells.at(which_cell)->Height();
    }
    return {};
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
    m_firstPress = m_lastLocation = CalcUnscrolledPosition(event.GetPosition());
    wxClientDC dc{ this };

    // which cell would this be?
    auto which_cell = WhichCell(m_firstPress);
    m_selected = which_cell;
    m_dragging = true;
    if (which_cell) {
        auto mouse_click = m_firstPress;
        mouse_click.y -= HeightToCell(*which_cell);
        mCells.at(*which_cell)->OnClick(dc, mouse_click);
    }
    Refresh();
}

void CustomListViewPanel::OnLeftDoubleClick(wxMouseEvent& event)
{
    // which cell would this be?
    auto which_cell = WhichCell(CalcUnscrolledPosition(event.GetPosition()));
    if (which_cell) {
        OnEditEntry(*which_cell);
    }
    Refresh();
}

void CustomListViewPanel::OnLeftUpMouseEvent(wxMouseEvent& event)
{
    auto starting_cell = WhichCell(m_firstPress);
    m_lastLocation = m_firstPress;
    // which cell would this be?
    auto which_cell = WhichCell(CalcUnscrolledPosition(event.GetPosition()));
    if (which_cell && starting_cell && *starting_cell != *which_cell) {
        // reorder.
        OnMoveEntry(*starting_cell, *which_cell);
    }
    m_selected = which_cell;
    m_dragging = false;
    Refresh();
}

// Define the repainting behaviour
void CustomListViewPanel::OnMouseMove(wxMouseEvent& event)
{
    if (event.LeftIsDown() && event.GetPosition().y >= 0) {
        m_lastLocation = CalcUnscrolledPosition(event.GetPosition());
    }
    Refresh();
}

// Define the repainting behaviour
void CustomListViewPanel::OnChar(wxKeyEvent& event)
{
    // ignore keypresses if dragging:
    if (m_dragging)
        return;
    if (event.GetKeyCode() == WXK_UP && m_selected < mCells.size() && m_selected && (*m_selected > 0)) {
        --(*m_selected);
    } else if (event.GetKeyCode() == WXK_DOWN && m_selected && (*m_selected < (mCells.size() - 1))) {
        ++(*m_selected);
    } else if (event.GetKeyCode() == WXK_RETURN) {
        // with the shift key we go one above.
        // -1 means the end.
        // empty means 0 all the time
        //
        auto where = m_selected ? *m_selected : 0;
        if (!event.ShiftDown()) {
            ++where;
        }
        OnNewEntry(std::min(mCells.size(), where));
    } else if (m_selected && (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DELETE || event.GetKeyCode() == WXK_BACK)) {
        OnDeleteEntry(*m_selected);
    } else {
        event.Skip();
    }
    Refresh();
}

void CustomListViewPanel::OnNewEntry(int)
{
}

void CustomListViewPanel::OnEditEntry(int)
{
}

void CustomListViewPanel::OnDeleteEntry(int)
{
}

void CustomListViewPanel::OnMoveEntry(int, int)
{
}

void CustomListViewPanel::OnEraseBackground(wxEraseEvent&) { }
