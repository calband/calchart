/*
 * FieldCanvas
 * Canvas for the Field window
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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1

#include "FieldCanvas.h"

#include "CalChartConfiguration.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartFrame.h"
#include "CalChartMovePointsTool.h"
#include "CalChartShapes.h"
#include "CalChartView.h"
#include <ranges>

#include <wx/dcbuffer.h>

namespace {
auto TranslateMouseToCoord(wxClientDC& dc, wxMouseEvent& event)
{
    auto mousePos = event.GetPosition();
    return CalChart::Coord(tDIP(dc.DeviceToLogicalX(mousePos.x)), tDIP(dc.DeviceToLogicalY(mousePos.y)));
}

auto SNAPGRID(CalChart::Coord::units a, CalChart::Coord::units n, CalChart::Coord::units s)
{
    auto a2 = (a + (n >> 1)) & (~(n - 1));
    auto h = s >> 1;
    if ((a - a2) >= h) {
        return a2 + s;
    } else if ((a - a2) < -h) {
        return a2 - s;
    }
    return a2;
}

auto GenerateShapeBasedCommands(std::optional<CalChart::SelectTool> const& selectTool, CalChart::MovePointsTool const* movePointsTool, CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    if (selectTool) {
        CalChart::append(
            drawCmds,
            selectTool->GenerateDrawCommands());
    }
    if (movePointsTool) {
        CalChart::append(
            drawCmds,
            movePointsTool->GenerateDrawCommands());
    }
    return {
        CalChart::Draw::withBrush(
            CalChart::Brush::TransparentBrush(),
            CalChart::Draw::withPen(
                CalChart::toPen(config.Get_CalChartBrushAndPen(CalChart::Colors::SHAPES)),
                drawCmds))
    };
}

}

BEGIN_EVENT_TABLE(FieldCanvas, FieldCanvas::super)
EVT_CHAR(FieldCanvas::OnChar)
EVT_LEFT_DOWN(FieldCanvas::OnMouseLeftDown)
EVT_LEFT_UP(FieldCanvas::OnMouseLeftUp)
EVT_LEFT_DCLICK(FieldCanvas::OnMouseLeftDoubleClick)
EVT_RIGHT_DOWN(FieldCanvas::OnMouseRightDown)
EVT_MAGNIFY(FieldCanvas::OnMousePinchToZoom)
EVT_MOTION(FieldCanvas::OnMouseMove)
EVT_PAINT(FieldCanvas::OnFieldPaint)
EVT_ERASE_BACKGROUND(FieldCanvas::OnEraseBackground)
EVT_MOUSEWHEEL(FieldCanvas::OnMouseWheel)
END_EVENT_TABLE()

// Define a constructor for field canvas
FieldCanvas::FieldCanvas(wxWindow* parent, CalChartView* view, float def_zoom, CalChart::Configuration const& config)
    : super(config, parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL | wxNO_BORDER)
    , mView(view)
    , mConfig(config)
{
    Init();
    super::SetZoom(def_zoom);
}

void FieldCanvas::Init()
{
    SetCanvasSize(fDIP(wxSize{ mView->GetShowFullSize().x, mView->GetShowFullSize().y }));
}

void FieldCanvas::SetView(CalChartView* view)
{
    mView = view;
}

// Painting involves deferring to the view as it has much of the information about how to draw consistently
// across several different widgets
void FieldCanvas::OnFieldPaint(wxPaintEvent& event)
{
    OnPaint(event, mConfig);
}

void FieldCanvas::OnPaint(wxPaintEvent&, CalChart::Configuration const& config)
{
    if (!mView) {
        return;
    }
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    PaintBackground(dc, config);

    // draw Background Image
    mView->OnDrawBackground(dc);

    // draw the view
    mView->OnDraw(&dc);

    // draw the move points dots
    auto origin = mView->GetShowFieldOffset();
    auto drawCmds = mView->GeneratePhatomPointsDrawCommands(mUncommittedMovePoints);
    CalChart::append(drawCmds,
        GenerateShapeBasedCommands(mSelectTool, mMovePointsTool.get(), config));

    wxCalChart::Draw::DrawCommandList(dc, drawCmds + origin);
}

void FieldCanvas::PaintBackground(wxDC& dc, CalChart::Configuration const& config)
{
    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    wxCalChart::setBackground(dc, config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD));
    dc.Clear();
}

// We have a empty erase background to improve redraw performance.
void FieldCanvas::OnEraseBackground(wxEraseEvent&) { }

// When a left click down occurs:
// We could be doing picture adjustments, so handle that first
// Otherwise, we're either doing a Normal move or a complicated Move
// For the complicated move, we create one if it doesn't exist and then add this click to it.
void FieldCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    wxClientDC dc(this);
    PrepareDC(dc);

    if (mView->DoingPictureAdjustment()) {
        mView->OnBackgroundMouseLeftDown(event, dc);
        Refresh();
        return;
    }

    auto mousePos = TranslateMouseToCoord(dc, event);
    auto pos = mousePos - mView->GetShowFieldOffset();

    if (GetCurrentMove() == CalChart::MoveMode::Normal) {
        OnMouseLeftDown_NormalMove(pos, event.ShiftDown(), event.AltDown());
    } else {
        if (!mMovePointsTool) {
            mMovePointsTool = CalChart::MovePointsTool::Create(GetCurrentMove());
        }
        mMovePointsTool->OnClickDown(SnapToolToGrid(pos));
    }
    Refresh();
}

void FieldCanvas::OnMouseLeftDown_NormalMove(CalChart::Coord pos, bool shiftDown, bool altDown)
{
    // If we are doing a polygon we only care about when the mouse click is released
    if (mSelectTool && mView->GetSelect() == CalChart::Select::Poly) {
        return;
    }
    // If we're doing a Swap, handle swapping the marchers now.
    if (mView->GetSelect() == CalChart::Select::Swap) {
        OnMouseLeftDown_Swap(pos);
    }

    auto i = mView->FindMarcher(pos);
    // if we didn't click on anything, and we have no modifiers, we are starting a new selection
    if (!i.has_value() && !(shiftDown || altDown)) {
        mView->UnselectAll();
    }
    if (!i.has_value()) {
        // if no point selected, we grab using the current select
        BeginSelectDrag(mView->GetSelect(), pos);
        return;
    }

    // Now add whatever we clicked to the selection.
    auto select = CalChart::SelectionList{ *i };
    if (altDown) {
        mView->ToggleSelection(select);
    } else {
        mView->AddToSelection(select);
    }

    mMovePointsTool = CalChart::MovePointsTool::Create(CalChart::MoveMode::Normal);
    mMovePointsTool->OnClickDown(SnapToolToGrid(pos));
}

// Swap works by finding the points clicked on.
// if more than 1 point is in the current selectionList, we unselect everything
// then we add the newly clicked point to the selectionList.
// If we have 2 points, we're done, and we swap those two points (rotate)
// And then we unselect
void FieldCanvas::OnMouseLeftDown_Swap(CalChart::Coord pos)
{
    auto targetDotIndex = mView->FindMarcher(pos);
    if (!targetDotIndex.has_value()) {
        return;
    }
    CalChart::SelectionList targetDot;
    targetDot.insert(*targetDotIndex);
    if (mView->GetSelectionList().size() != 1) {
        mView->UnselectAll();
    }
    mView->AddToSelection(targetDot);
    if (mView->GetSelectionList().size() == 2) {
        mView->DoRotatePointPositions(1);
        mView->UnselectAll();
    }
}

// When a left click up occurs:
// We could be doing picture adjustments, so handle that first
// Otherwise, if we are moving points, then determine if we're done.
// Finally, if none of that, close up any select tools
void FieldCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    wxClientDC dc(this);
    PrepareDC(dc);

    if (mView->DoingPictureAdjustment()) {
        mView->OnBackgroundMouseLeftUp(event, dc);
        Refresh();
        return;
    }
    auto mousePos = TranslateMouseToCoord(dc, event);
    auto pos = mousePos - mView->GetShowFieldOffset();

    if (mMovePointsTool) {
        mMovePointsTool->OnClickUp(pos);
        if (mMovePointsTool->IsDone()) {
            mView->DoMovePoints(mUncommittedMovePoints);
            EndDrag();
            mView->SetCurrentMove(CalChart::MoveMode::Normal);
            static_cast<CalChartFrame*>(GetParent())->ToolBarSetCurrentMove(CalChart::MoveMode::Normal);
        }
    }
    if (mSelectTool) {
        mSelectTool->OnClickUp(pos);
        if (auto polygon = mSelectTool->GetPolygon(); polygon && mSelectTool->SelectDone()) {
            mView->SelectWithinPolygon(*mSelectTool->GetPolygon(), event.AltDown());
            EndDrag();
        }
    }
    Refresh();
}

// Allow double click to close polygons
void FieldCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    wxClientDC dc(this);
    PrepareDC(dc);

    if (mSelectTool && (CalChart::Select::Poly == mView->GetSelect())) {
        if (auto polygon = mSelectTool->GetPolygon(); polygon) {
            mView->SelectWithinPolygon(*polygon, event.AltDown());
        }
        EndDrag();
    }
    Refresh();
}

// Allow right click  to close polygons
void FieldCanvas::OnMouseRightDown(wxMouseEvent& event)
{
    OnMouseLeftDoubleClick(event);
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMouseMove(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    super::OnMouseMove(event);

    if (IsScrolling()) {
        Refresh();
        return;
    }

    wxClientDC dc(this);
    PrepareDC(dc);

    if (mView->DoingPictureAdjustment()) {
        mView->OnBackgroundMouseMove(event, dc);
        Refresh();
        return;
    }

    auto mousePos = TranslateMouseToCoord(dc, event);
    auto pos = mousePos - mView->GetShowFieldOffset();

    // if we are dragging with the left mouse down OR we are moving with the poly selection tool
    if ((event.Dragging() && event.LeftIsDown()) || (event.Moving() && mSelectTool && (CalChart::Select::Poly == mView->GetSelect()))) {
        MoveDrag(pos);
    }
    Refresh();
}

void FieldCanvas::MoveDrag(CalChart::Coord end)
{
    if (mSelectTool) {
        mSelectTool->OnMove(end, [this](auto c) { return SnapToolToGrid(c); });
    }
    if (mMovePointsTool) {
        mMovePointsTool->OnMove(end, [this](auto c) { return SnapToolToGrid(c); });
        std::map<int, CalChart::Coord> selected_points;
        for (auto i : mView->GetSelectionList()) {
            selected_points[i] = mView->PointPosition(i);
        }

        if (mMovePointsTool->IsReadyForMoving()) {
            mUncommittedMovePoints = mMovePointsTool->TransformPoints(selected_points);
            for (auto& i : mUncommittedMovePoints) {
                i.second = mView->ClipPositionToShowMode(SnapToGrid(i.second));
            }
        }
    }
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMousePinchToZoom(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    super::OnMousePinchToZoom(event);
    SetZoom(static_cast<CalChartFrame*>(GetParent())->ToolBarSetZoom(GetZoom()));
}

// Intercept character input
void FieldCanvas::OnChar(wxKeyEvent& event)
{
    if (!mView) {
        return;
    }
    if (event.GetKeyCode() == WXK_LEFT)
        mView->GoToPrevSheet();
    else if (event.GetKeyCode() == WXK_RIGHT)
        mView->GoToNextSheet();
    else if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DELETE || event.GetKeyCode() == WXK_BACK) {
        mView->OnBackgroundImageDelete();
        mView->DoDeletePoints();
    }
    if (event.GetKeyCode() == 'w') {
        MoveByKey(direction::north);
    } else if (event.GetKeyCode() == 'd') {
        MoveByKey(direction::east);
    } else if (event.GetKeyCode() == 's') {
        MoveByKey(direction::south);
    } else if (event.GetKeyCode() == 'a') {
        MoveByKey(direction::west);
    } else
        event.Skip();
}

// Zoom to fit length wise, seems best idea
float FieldCanvas::ZoomToFitFactor() const
{
    const wxSize screenSize = GetSize();
    return static_cast<float>(screenSize.GetX()) / mView->GetShowFullSize().x;
}

void FieldCanvas::SetZoom(float factor)
{
    super::SetZoom(factor);
    Refresh();
}

void FieldCanvas::SetZoomAroundCenter(float factor)
{
    auto size = GetSize();
    super::SetZoom(factor, { size.x / 2, size.y / 2 });
    Refresh();
}

void FieldCanvas::BeginSelectDrag(CalChart::Select type, CalChart::Coord start)
{
    mSelectTool = CalChart::SelectTool{ type, start, [this](int input) {
                                           wxClientDC dc(this);
                                           PrepareDC(dc);
                                           return dc.DeviceToLogicalXRel(input);
                                       } };
}

CalChart::Coord FieldCanvas::GetMoveAmount(direction dir)
{
    auto stepsize = std::get<0>(static_cast<CalChartFrame*>(GetParent())->GridChoice());
    stepsize = std::max(stepsize, CalChart::Int2CoordUnits(1));
    switch (dir) {
    case direction::north:
        return { 0, static_cast<CalChart::Coord::units>(-stepsize) };
    case direction::east:
        return { stepsize, 0 };
    case direction::south:
        return { 0, stepsize };
    case direction::west:
        return { static_cast<CalChart::Coord::units>(-stepsize), 0 };
    }
    // on the offchance somebody gets here
    return { 0, 0 };
}

CalChart::Coord FieldCanvas::SnapToGrid(CalChart::Coord c)
{
    CalChart::Coord::units gridn, grids;
    std::tie(gridn, grids) = static_cast<CalChartFrame*>(GetParent())->GridChoice();

    return {
        c.x = SNAPGRID(c.x, gridn, grids),
        // Adjust so 4 step grid will be on visible grid
        c.y = SNAPGRID(c.y - CalChart::Int2CoordUnits(2), gridn, grids) + CalChart::Int2CoordUnits(2)
    };
}

CalChart::Coord FieldCanvas::SnapToolToGrid(CalChart::Coord c)
{
    CalChart::Coord::units gridn, grids;
    std::tie(gridn, grids) = static_cast<CalChartFrame*>(GetParent())->ToolGridChoice();

    return {
        c.x = SNAPGRID(c.x, gridn, grids),
        // Adjust so 4 step grid will be on visible grid
        c.y = SNAPGRID(c.y - CalChart::Int2CoordUnits(2), gridn, grids) + CalChart::Int2CoordUnits(2)
    };
}

void FieldCanvas::MoveByKey(direction dir)
{
    if (!mView) {
        return;
    }
    if (mView->GetSelectionList().empty())
        return;
    std::map<int, CalChart::Coord> move_points;
    auto&& select_list = mView->GetSelectionList();
    auto pos = GetMoveAmount(dir);
    // saturate by mode
    for (auto i = select_list.begin(); i != select_list.end(); ++i) {
        move_points[*i] = mView->ClipPositionToShowMode(mView->PointPosition(*i) + pos);
    }
    mView->DoMovePoints(move_points);
}

void FieldCanvas::EndDrag()
{
    mUncommittedMovePoints.clear();
    mMovePointsTool.reset();
    mSelectTool = std::nullopt;
}

CalChart::Select FieldCanvas::GetCurrentSelect() const { return mView ? mView->GetSelect() : CalChart::Select::Box; }
CalChart::MoveMode FieldCanvas::GetCurrentMove() const { return mView ? mView->GetCurrentMove() : CalChart::MoveMode::Normal; }

void FieldCanvas::SetCurrentSelect(CalChart::Select select)
{
    if (mView) {
        mView->SetSelect(select);
    }
}

// implies a call to EndDrag()
void FieldCanvas::SetCurrentMove(CalChart::MoveMode move)
{
    EndDrag();
    mView->SetCurrentMove(move);
}
