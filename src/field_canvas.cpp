/*
 * field_canvas.cpp
 * Canvas for the Field window
 */

/*
   Copyright (C) 1995-2012  Garrick Brian Meeker, Richard Michael Powell

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

#include "field_canvas.h"

#include "field_view.h"
#include "field_frame.h"
#include "confgr.h"
#include "linmath.h"
#include "math_utils.h"
#include "background_image.h"
#include "cc_shapes.h"
#include "cc_drawcommand.h"
#include "draw.h"
#include "field_canvas_shapes.h"

#include <wx/dcbuffer.h>

BEGIN_EVENT_TABLE(FieldCanvas, ClickDragCtrlScrollCanvas)
EVT_CHAR(FieldCanvas::OnChar)
EVT_LEFT_DOWN(FieldCanvas::OnMouseLeftDown)
EVT_LEFT_UP(FieldCanvas::OnMouseLeftUp)
EVT_LEFT_DCLICK(FieldCanvas::OnMouseLeftDoubleClick)
EVT_RIGHT_DOWN(FieldCanvas::OnMouseRightDown)
EVT_MAGNIFY(FieldCanvas::OnMousePinchToZoom)
EVT_MOTION(FieldCanvas::OnMouseMove)
EVT_PAINT(FieldCanvas::OnPaint)
EVT_ERASE_BACKGROUND(FieldCanvas::OnEraseBackground)
EVT_MOUSEWHEEL(FieldCanvas::OnMouseWheel)
END_EVENT_TABLE()

// Define a constructor for field canvas
FieldCanvas::FieldCanvas(FieldView& view, FieldFrame* frame, float def_zoom)
    : ClickDragCtrlScrollCanvas(frame, wxID_ANY)
    , mFrame(frame)
    , mView(view)
{
    SetCanvasSize(wxSize{ mView.GetShowFieldSize().x, mView.GetShowFieldSize().y });
    SetZoom(def_zoom);
}

// Define the repainting behaviour
void FieldCanvas::OnPaint(wxPaintEvent& event)
{
    const auto& config = CalChartConfiguration::GetGlobalConfig();
    OnPaint(event, config);
}

void FieldCanvas::OnPaint(wxPaintEvent& event,
    const CalChartConfiguration& config)
{
    wxBufferedPaintDC dc(this);
    PrepareDC(dc);

    // draw the background
    PaintBackground(dc, config);

    // draw Background Image
    mView.OnDrawBackground(dc);

    // draw the view
    mView.OnDraw(&dc);

    // draw the move points dots
    mView.DrawOtherPoints(dc, mMovePoints);

    PaintSelectShapes(dc, config);
    PaintMoveShapes(dc, config);
}

void FieldCanvas::PaintBackground(wxDC& dc,
    const CalChartConfiguration& config)
{
    // draw the background
    dc.SetBackgroundMode(wxTRANSPARENT);
    dc.SetBackground(config.Get_CalChartBrushAndPen(COLOR_FIELD).first);
    dc.Clear();
}

void FieldCanvas::PaintShapes(wxDC& dc,
    CalChartConfiguration const& config,
    ShapeList const& shape_list)
{
    if (!shape_list.empty()) {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(config.Get_CalChartBrushAndPen(COLOR_SHAPES).second);
        auto origin = mView.GetShowFieldOffset();
        for (auto&& i : shape_list) {
            DrawCC_DrawCommandList(dc, i->GetCC_DrawCommand(origin.x, origin.y));
        }
    }
}

void FieldCanvas::PaintSelectShapes(wxDC& dc,
    const CalChartConfiguration& config)
{
    return PaintShapes(dc, config, m_select_shape_list);
}

void FieldCanvas::PaintMoveShapes(wxDC& dc,
    const CalChartConfiguration& config)
{
    if (m_move_points) {
        PaintShapes(dc, config, m_move_points->GetShapeList());
    }
}

// We have a empty erase background to improve redraw performance.
void FieldCanvas::OnEraseBackground(wxEraseEvent& event) {}

void FieldCanvas::OnMouseLeftDown_default(wxMouseEvent& event, CC_coord pos)
{
    if (select_drag == CC_DRAG_POLY) {
        return;
    }
    if (curr_lasso == CC_DRAG_SWAP) {
        OnMouseLeftDown_CC_DRAG_SWAP(pos);
    }
    if (!(event.ShiftDown() || event.AltDown())) {
        mView.UnselectAll();
    }
    auto i = mView.FindPoint(pos);
    if (i < 0) {
        // if no point selected, we grab using the current lasso
        BeginSelectDrag(curr_lasso, pos);
    }
    else {
        SelectionList select;
        select.insert(i);
        if (event.AltDown()) {
            mView.ToggleSelection(select);
        }
        else {
            mView.AddToSelection(select);
        }

        m_move_points = Create_MovePoints(CC_MOVE_NORMAL);
        m_move_points->OnMouseLeftDown(pos);
    }
}

void FieldCanvas::OnMouseLeftUp_CC_DRAG_BOX(wxMouseEvent& event, CC_coord)
{
    const CC_shape_2point* shape = (CC_shape_2point*)m_select_shape_list.back().get();
    mView.SelectPointsInRect(shape->GetOrigin(), shape->GetPoint(),
        event.AltDown());
    EndDrag();
}

void FieldCanvas::OnMouseLeftUp_CC_DRAG_LASSO(wxMouseEvent& event, CC_coord pos)
{
    ((CC_lasso*)m_select_shape_list.back().get())->End();
    mView.SelectWithLasso((CC_lasso*)m_select_shape_list.back().get(), event.AltDown());
    EndDrag();
}

void FieldCanvas::OnMouseLeftUp_CC_DRAG_POLY(wxMouseEvent& event, CC_coord pos)
{
    static constexpr auto CLOSE_ENOUGH_TO_CLOSE = 10;
    auto* p = ((CC_lasso*)m_select_shape_list.back().get())->FirstPoint();
    auto numPnts = ((CC_lasso*)m_select_shape_list.back().get())->NumPoints();
    if (p != NULL && numPnts > 2) {
        // need to know where the scale is, so we need the device.
        wxClientDC dc(this);
        PrepareDC(dc);
        Coord polydist = dc.DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
        auto d = p->x - pos.x;
        if (std::abs(d) < polydist) {
            d = p->y - pos.y;
            if (std::abs(d) < polydist) {
                mView.SelectWithLasso((CC_lasso*)m_select_shape_list.back().get(),
                    event.AltDown());
                EndDrag();
                return;
            }
        }
    }
    ((CC_lasso*)m_select_shape_list.back().get())->Append(pos);
}

void FieldCanvas::OnMouseLeftUp_default(wxMouseEvent& event, CC_coord pos)
{
    switch (select_drag) {
    case CC_DRAG_BOX:
        OnMouseLeftUp_CC_DRAG_BOX(event, pos);
        break;
    case CC_DRAG_LASSO:
        OnMouseLeftUp_CC_DRAG_LASSO(event, pos);
        break;
    case CC_DRAG_POLY:
        OnMouseLeftUp_CC_DRAG_POLY(event, pos);
        break;
    default:
        break;
    }
}

void FieldCanvas::OnMouseLeftDown_CC_DRAG_SWAP(CC_coord pos)
{
    int targetDotIndex = mView.FindPoint(pos);
    if (targetDotIndex >= 0) {
        SelectionList targetDot;
        targetDot.insert(targetDotIndex);
        if (mView.GetSelectionList().size() != 1) {
            mView.UnselectAll();
        }
        mView.AddToSelection(targetDot);
        if (mView.GetSelectionList().size() == 2) {
            mView.DoRotatePointPositions(1);
            mView.UnselectAll();
        }
    }
    else {
        mView.UnselectAll();
    }
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMouseLeftDown(wxMouseEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    wxCoord x, y;
    event.GetPosition(&x, &y);
    x = dc.DeviceToLogicalX(x);
    y = dc.DeviceToLogicalY(y);

    if (mView.DoingPictureAdjustment()) {
        mView.OnBackgroundMouseLeftDown(event, dc);
    }
    else {
        CC_coord pos = mView.GetShowFieldOffset();
        pos.x = (x - pos.x);
        pos.y = (y - pos.y);

        if (curr_move != CC_MOVE_NORMAL) {
            m_move_points = Create_MovePoints(curr_move);
            m_move_points->OnMouseLeftDown(SnapToGrid(pos));
        }
        else {
            OnMouseLeftDown_default(event, pos);
        }
    }
    Refresh();
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMouseLeftUp(wxMouseEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    if (mView.DoingPictureAdjustment()) {
        mView.OnBackgroundMouseLeftUp(event, dc);
    }
    else {
        CC_coord pos = mView.GetShowFieldOffset();
        pos.x = (x - pos.x);
        pos.y = (y - pos.y);

        if (m_move_points) {
            if (m_move_points->OnMouseUpDone(pos)) {
                mView.DoMovePoints(mMovePoints);
                mFrame->SetCurrentMove(CC_MOVE_NORMAL);
            }
        }
        if (!(m_select_shape_list.empty())) {
            OnMouseLeftUp_default(event, pos);
        }
    }
    Refresh();
}

// Allow double click to close polygons
void FieldCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);

    if (!m_select_shape_list.empty() && (CC_DRAG_POLY == select_drag)) {
        mView.SelectWithLasso((CC_lasso*)m_select_shape_list.back().get(), event.AltDown());
        EndDrag();
    }
    Refresh();
}

// Allow right click  to close polygons
void FieldCanvas::OnMouseRightDown(wxMouseEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);

    if (!m_select_shape_list.empty() && (CC_DRAG_POLY == select_drag)) {
        mView.SelectWithLasso((CC_lasso*)m_select_shape_list.back().get(), event.AltDown());
        EndDrag();
    }
    Refresh();
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMouseMove(wxMouseEvent& event)
{
    super::OnMouseMove(event);

    if (!IsScrolling()) {
        wxClientDC dc(this);
        PrepareDC(dc);
        auto point = event.GetPosition();
        auto x = dc.DeviceToLogicalX(point.x);
        auto y = dc.DeviceToLogicalY(point.y);

        if (mView.DoingPictureAdjustment()) {
            mView.OnBackgroundMouseMove(event, dc);
        }
        else {
            CC_coord pos = mView.GetShowFieldOffset();
            pos.x = (x - pos.x);
            pos.y = (y - pos.y);

            if (event.Dragging() && event.LeftIsDown()) {
                MoveDrag(pos);
            }
            if (event.Moving() && !m_select_shape_list.empty() && (CC_DRAG_POLY == select_drag)) {
                MoveDrag(pos);
            }
        }
    }
    Refresh();
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMousePinchToZoom(wxMouseEvent& event)
{
    super::OnMousePinchToZoom(event);
    mFrame->do_zoom(GetZoom());
    Refresh();
}

// Intercept character input
void FieldCanvas::OnChar(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_LEFT)
        mView.GoToPrevSheet();
    else if (event.GetKeyCode() == WXK_RIGHT)
        mView.GoToNextSheet();
    else if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DELETE || event.GetKeyCode() == WXK_BACK) {
        mView.OnBackgroundImageDelete();
        mView.DoDeletePoints();
    }
    if (event.GetKeyCode() == 'w') {
        MoveByKey(direction::north);
    }
    if (event.GetKeyCode() == 'd') {
        MoveByKey(direction::east);
    }
    if (event.GetKeyCode() == 's') {
        MoveByKey(direction::south);
    }
    if (event.GetKeyCode() == 'a') {
        MoveByKey(direction::west);
    }
    else
        event.Skip();
}

// Zoom to fit length wise, seems best idea
float FieldCanvas::ZoomToFitFactor() const
{
    const wxSize screenSize = GetSize();
    return static_cast<float>(screenSize.GetX()) / mView.GetShowFieldSize().x;
}

void FieldCanvas::SetZoom(float factor)
{
    super::SetZoom(factor);
    Refresh();
}

void FieldCanvas::BeginSelectDrag(CC_DRAG_TYPES type, const CC_coord& start)
{
    select_drag = type;
    m_select_shape_list.clear();
    switch (type) {
    case CC_DRAG_BOX:
        m_select_shape_list.emplace_back(new CC_shape_rect(start));
        break;
    case CC_DRAG_POLY:
        m_select_shape_list.emplace_back(new CC_poly(start));
        break;
    case CC_DRAG_LASSO:
        m_select_shape_list.emplace_back(new CC_lasso(start));
        break;
    case CC_DRAG_LINE:
        m_select_shape_list.emplace_back(new CC_shape_line(start));
        break;
    case CC_DRAG_CROSSHAIRS:
        m_select_shape_list.emplace_back(new CC_shape_crosshairs(start, Int2Coord(2)));
        break;
    case CC_DRAG_SHAPE_ELLIPSE:
        m_select_shape_list.emplace_back(new CC_shape_ellipse(start));
        break;
    case CC_DRAG_SHAPE_X:
        m_select_shape_list.emplace_back(new CC_shape_x(start));
        break;
    case CC_DRAG_SHAPE_CROSS:
        m_select_shape_list.emplace_back(new CC_shape_cross(start));
        break;
    default:
        break;
    }
}

void FieldCanvas::MoveDrag(const CC_coord& end)
{
    if (!m_select_shape_list.empty()) {
        m_select_shape_list.back()->OnMove(end, SnapToGrid(end));
    }
    if (m_move_points) {
        m_move_points->OnMove(end, SnapToGrid(end));
        std::map<int, CC_coord> selected_points;
        for (auto i : mView.GetSelectionList()) {
            selected_points[i] = mView.PointPosition(i);
        }

        mMovePoints = m_move_points->TransformPoints(selected_points);
        for (auto& i : mMovePoints) {
            i.second = mView.ClipPositionToShowMode(SnapToGrid(i.second));
        }
        // saturate by mode
    }
}

CC_coord FieldCanvas::GetMoveAmount(direction dir)
{
    auto stepsize = std::get<0>(mFrame->GridChoice());
    stepsize = std::max(stepsize, Int2Coord(1));
    switch (dir) {
    case direction::north:
        return { 0, static_cast<Coord>(-stepsize) };
    case direction::east:
        return { stepsize, 0 };
    case direction::south:
        return { 0, stepsize };
    case direction::west:
        return { static_cast<Coord>(-stepsize), 0 };
    }
    // on the offchance somebody gets here
    return { 0, 0 };
}

static inline Coord SNAPGRID(Coord a, Coord n, Coord s)
{
    Coord a2 = (a + (n >> 1)) & (~(n - 1));
    Coord h = s >> 1;
    if ((a - a2) >= h)
        return a2 + s;
    else if ((a - a2) < -h)
        return a2 - s;
    else
        return a2;
}

CC_coord FieldCanvas::SnapToGrid(CC_coord c)
{
    Coord gridn, grids;
    std::tie(gridn, grids) = mFrame->GridChoice();

    return {
        c.x = SNAPGRID(c.x, gridn, grids),
        // Adjust so 4 step grid will be on visible grid
        c.y = SNAPGRID(c.y - Int2Coord(2), gridn, grids) + Int2Coord(2)
    };
}

void FieldCanvas::MoveByKey(direction dir)
{
    if (mView.GetSelectionList().empty())
        return;
    std::map<int, CC_coord> move_points;
    auto&& select_list = mView.GetSelectionList();
    auto pos = GetMoveAmount(dir);
    for (auto i = select_list.begin(); i != select_list.end(); ++i) {
        move_points[*i] = mView.ClipPositionToShowMode(mView.PointPosition(*i) + pos);
    }
    // saturate by mode
    mView.DoMovePoints(move_points);
}

void FieldCanvas::EndDrag()
{
    mMovePoints.clear();
    m_move_points.reset();
    m_select_shape_list.clear();
    select_drag = CC_DRAG_NONE;
}

void FieldCanvas::SetCurrentLasso(CC_DRAG_TYPES lasso) { curr_lasso = lasso; }

// implies a call to EndDrag()
void FieldCanvas::SetCurrentMove(CC_MOVE_MODES move)
{
    EndDrag();
    curr_move = move;
}
