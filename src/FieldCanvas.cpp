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

#include "FieldCanvas.h"

#include "CalChartConfiguration.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartFrame.h"
#include "CalChartMovePointsTool.h"
#include "CalChartShapes.h"
#include "CalChartUtils.h"
#include "CalChartView.h"
#include <optional>
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

template <typename Function>
struct Defer {
    Defer(Function function)
        : function(function)
    {
    }
    ~Defer() { function(); }
    Defer(Defer const&) = delete;
    Defer(Defer&&) = delete;
    auto operator=(Defer const&) -> Defer& = delete;
    auto operator=(Defer&&) -> Defer& = delete;

private:
    Function function;
};

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

namespace {
auto GenerateCurveControlPoints(std::vector<CalChart::Coord> const& points, CalChart::Coord::units boxSize) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(points | std::views::transform([boxSize](auto&& point) {
        return CalChart::Draw::Rectangle(point - CalChart::Coord(boxSize, boxSize) / 2, CalChart::Coord(boxSize, boxSize));
    }));
}

auto GenerateCurves(CalChart::Curve const& curve, CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto boxSize = CalChart::Float2CoordUnits(config.Get_ControlPointRatio());
    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::DRAW_CURVE),
            curve.GetCC_DrawCommand()),
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::DRAW_CURVE_CONTROL_POINT),
            GenerateCurveControlPoints(curve.GetControlPoints(), boxSize)),
    };
    return drawCmds;
}

auto DrawCurve(CalChart::Curve const& curve, std::set<size_t> selectedPoints, CalChart::Configuration const& config) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto drawCmds = GenerateCurves(curve, config);
    auto boxSize = 2 * CalChart::Float2CoordUnits(config.Get_ControlPointRatio());
    CalChart::append(drawCmds,
        CalChart::Draw::withBrushAndPen(
            config.Get_CalChartBrushAndPen(CalChart::Colors::DRAW_CURVE_CONTROL_POINT),
            selectedPoints | std::views::transform([&curve](auto&& index) { return curve.GetControlPoints().at(index); })
                | std::views::transform([boxSize](auto&& point) {
                      return CalChart::Draw::Rectangle(point - CalChart::Coord(boxSize, boxSize) / 2, CalChart::Coord(boxSize, boxSize));
                  })));
    return drawCmds;
}
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

    if (mCurve.has_value()) {
        CalChart::append(drawCmds,
            DrawCurve(*mCurve, mCurvePointsSelected, config));
    }

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
    auto _ = Defer([this]() { Refresh(); });
    wxClientDC dc(this);
    PrepareDC(dc);

    if (mView->DoingPictureAdjustment()) {
        mView->OnBackgroundMouseLeftDown(event, dc);
        return;
    }

    auto mousePos = TranslateMouseToCoord(dc, event);
    auto pos = mousePos - mView->GetShowFieldOffset();

    mDragStart = pos;

    if (GetCurrentMove() == CalChart::MoveMode::Normal) {
        OnMouseLeftDown_NormalMove(pos, event.ShiftDown(), event.AltDown());
        return;
    }
    if (!mMovePointsTool) {
        mMovePointsTool = CalChart::MovePointsTool::Create(GetCurrentMove());
    }
    mMovePointsTool->OnClickDown(SnapToolToGrid(pos));
}

void FieldCanvas::OnMouseLeftDown_NormalMove(CalChart::Coord pos, bool shiftDown, bool altDown)
{
    // if we are drawing a curve, then we do the curve path.
    if (mView->IsDrawingCurve()) {
        OnMouseLeftDown_DrawingCurve(pos);
        return;
    }
    // If we are doing a polygon we only care about when the mouse click is released
    if (mSelectTool && mView->GetSelect() == CalChart::Select::Poly) {
        return;
    }
    // If we're doing a Swap, handle swapping the marchers now.
    if (mView->GetSelect() == CalChart::Select::Swap) {
        OnMouseLeftDown_Swap(pos);
        return;
    }

    auto foundCurveControls = mView->FindCurveControlPoint(pos);
    // If we clicked on a curve point, modify that point.
    if (foundCurveControls.has_value()) {
        OnMouseLeftDown_FoundCurveControl(*foundCurveControls, shiftDown, altDown);
        return;
    }
    auto foundCurve = mView->FindCurve(pos);
    // If we clicked on a curve select the whole curve.
    if (foundCurve.has_value()) {
        OnMouseLeftDown_FoundCurve(*foundCurve);
        return;
    }
    // didn't click on a curve point, we're not doing any curve things
    mCurve = std::nullopt;
    mCurveSelected = std::nullopt;
    EndCurveControlPointDrag();

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

void FieldCanvas::OnMouseLeftDown_DrawingCurve(CalChart::Coord pos)
{
    if (!mCurve.has_value()) {
        mCurve = CalChart::Curve(pos);
        return;
    }
    mCurve->Append(pos);
}

void FieldCanvas::OnMouseLeftDown_FoundCurveControl(std::tuple<size_t, size_t> curveIndex, bool shiftDown, bool altDown)
{
    // What are the cases?
    // If we don't have a curve, then get the curve and the point -- shifts/alts don't matter here
    // If we have a different curve, then treat it like we don't have a curve.
    // If we are already selecting the curve, then shift means add it, alt means toggle,
    if (!mCurveSelected.has_value() || (mCurveSelected.has_value() && *mCurveSelected != std::get<0>(curveIndex))) {
        mCurve = mView->GetCurrentCurve(std::get<0>(curveIndex));
        mCurveSelected = std::get<0>(curveIndex);
        mCurvePointsSelected.insert(std::get<1>(curveIndex));
    } else {
        if (!shiftDown && !altDown) {
            mCurvePointsSelected.clear();
            mCurvePointsSelected.insert(std::get<1>(curveIndex));
        }
        if (shiftDown) {
            mCurvePointsSelected.insert(std::get<1>(curveIndex));
        } else if (altDown) {
            if (mCurvePointsSelected.contains(std::get<1>(curveIndex))) {
                mCurvePointsSelected.erase(std::get<1>(curveIndex));
            } else {
                mCurvePointsSelected.insert(std::get<1>(curveIndex));
            }
        }
    }
    BeginCurveControlPointDrag();
}

void FieldCanvas::OnMouseLeftDown_FoundCurve(std::tuple<size_t, size_t> curveIndex)
{
    mCurve = mView->GetCurrentCurve(std::get<0>(curveIndex));
    mCurveSelected = std::get<0>(curveIndex);
    auto pointIndexes = std::views::iota(0UL, mCurve->GetControlPoints().size());
    mCurvePointsSelected.clear();
    for (auto index : pointIndexes) {
        mCurvePointsSelected.insert(index);
    }
    BeginCurveControlPointDrag();
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
    auto _ = Defer([this]() { Refresh(); });
    wxClientDC dc(this);
    PrepareDC(dc);

    if (mView->DoingPictureAdjustment()) {
        mView->OnBackgroundMouseLeftUp(event, dc);
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
    if (mCurvePointDragging) {
        EndCurveControlPointDrag();
    }
}

// Allow double click to close polygons
void FieldCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event)
{
    if (!mView) {
        return;
    }
    wxClientDC dc(this);
    PrepareDC(dc);
    auto _ = Defer([this]() { Refresh(); });

    if (mSelectTool && (CalChart::Select::Poly == mView->GetSelect())) {
        if (auto polygon = mSelectTool->GetPolygon(); polygon) {
            mView->SelectWithinPolygon(*polygon, event.AltDown());
        }
        EndDrag();
        return;
    }
    if (mView->IsDrawingCurve()) {
        OnMouseLeftDoubleClick_Curve();
        return;
    }

    auto mousePos = TranslateMouseToCoord(dc, event);
    auto pos = mousePos - mView->GetShowFieldOffset();
    auto foundCurve = mView->FindCurve(pos);

    // If we clicked on a curve point, modify that point.
    if (foundCurve.has_value()) {
        OnMouseLeftDoubleClick_FoundCurve(pos, *foundCurve);
        return;
    }
}

void FieldCanvas::OnMouseLeftDoubleClick_FoundCurve(CalChart::Coord pos, std::tuple<size_t, size_t> curveIndex)
{
    auto [whichCurve, whereToInsert] = curveIndex;
    auto points = mView->GetCurrentCurve(whichCurve).GetControlPoints();
    points.insert(points.begin() + whereToInsert + 1, pos);
    mCurvePointsSelected.clear();
    mCurvePointsSelected.insert(whereToInsert + 1);
    mCurve = CalChart::Curve(points);
    mView->DoReplaceSheetCurveCommand(*mCurve, whichCurve);
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
    auto _ = Defer([this]() { Refresh(); });
    super::OnMouseMove(event);

    if (IsScrolling()) {
        return;
    }

    wxClientDC dc(this);
    PrepareDC(dc);

    if (mView->DoingPictureAdjustment()) {
        mView->OnBackgroundMouseMove(event, dc);
        return;
    }

    auto mousePos = TranslateMouseToCoord(dc, event);
    auto pos = mousePos - mView->GetShowFieldOffset();

    if (mView->IsDrawingCurve()) {
        OnMouseMove_DrawCurve(pos);
    }

    // if we are dragging with the left mouse down OR we are moving with the poly selection tool
    if ((event.Dragging() && event.LeftIsDown()) || (event.Moving() && mSelectTool && (CalChart::Select::Poly == mView->GetSelect()))) {
        MoveDrag(pos);
        return;
    }
}

void FieldCanvas::OnMouseMove_DrawCurve(CalChart::Coord pos)
{
    if (!mCurve.has_value()) {
        return;
    }
    mCurve->OnMove(pos, [](auto c) { return c; });
}

void FieldCanvas::OnMouseMove_DragCurveControlPoint(CalChart::Coord pos)
{
    if (!mCurve.has_value() || !mCurveSelected.has_value()) {
        return;
    }
    mCurveShouldFlush = true;
    auto translate = pos - mDragStart;
    // collect all the control points, change the one control point.
    auto points = mView->GetCurrentCurve(*mCurveSelected).GetControlPoints();

    for (auto index : mCurvePointsSelected) {
        points.at(index) += translate;
    }
    mCurve = CalChart::Curve(points);
}

void FieldCanvas::OnDelete_Curve()
{
    AbortDrawing();
    if (!mCurve.has_value() || !mCurveSelected.has_value()) {
        return;
    }
    // collect all the control points, change the one control point.
    auto points = mCurve->GetControlPoints();

    mCurveShouldFlush = false;
    mCurvePointDragging = false;

    if ((points.size() - mCurvePointsSelected.size()) < 2) {
        mView->DoRemoveSheetCurveCommand(*mCurveSelected);
    } else {
        for (auto index : mCurvePointsSelected) {
            points.erase(points.begin() + index);
        }
        mView->DoReplaceSheetCurveCommand(CalChart::Curve(points), *mCurveSelected);
    }
    mCurveSelected = std::nullopt;
    mCurvePointsSelected.clear();
    mCurve = std::nullopt;
}

void FieldCanvas::OnEscape_Curve()
{
    AbortDrawing();
}

void FieldCanvas::AbortDrawing()
{
    if (mView->IsDrawingCurve()) {
        mCurve = std::nullopt;
        mView->SetDrawingCurve(false);
        static_cast<CalChartFrame*>(GetParent())->ToolBarUnsetDrawingCurve();
    }
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
    if (mCurvePointDragging) {
        OnMouseMove_DragCurveControlPoint(end);
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
    auto _ = Defer([this]() { Refresh(); });
    if (event.GetKeyCode() == WXK_LEFT) {
        mView->GoToPrevSheet();
        return;
    }
    if (event.GetKeyCode() == WXK_RIGHT) {
        mView->GoToNextSheet();
    }
    if (event.GetKeyCode() == WXK_DELETE || event.GetKeyCode() == WXK_NUMPAD_DELETE || event.GetKeyCode() == WXK_BACK) {
        mView->OnBackgroundImageDelete();
        mView->DoDeletePoints();
        OnDelete_Curve();
        return;
    }
    if (event.GetKeyCode() == WXK_ESCAPE) {
        OnEscape_Curve();
        return;
    }
    if (event.GetKeyCode() == 'w') {
        MoveByKey(direction::north);
        return;
    }
    if (event.GetKeyCode() == 'd') {
        MoveByKey(direction::east);
        return;
    }
    if (event.GetKeyCode() == 's') {
        MoveByKey(direction::south);
        return;
    }
    if (event.GetKeyCode() == 'a') {
        MoveByKey(direction::west);
        return;
    }
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

void FieldCanvas::BeginCurveControlPointDrag()
{
    mCurvePointDragging = true;
}

void FieldCanvas::EndCurveControlPointDrag()
{
    mCurvePointDragging = false;
    if (mCurveShouldFlush) {
        mView->DoReplaceSheetCurveCommand(*mCurve, *mCurveSelected);
        mCurveShouldFlush = false;
        mCurveSelected = std::nullopt;
        mCurvePointsSelected.clear();
        mCurve = std::nullopt;
    }
}

void FieldCanvas::OnMouseLeftDoubleClick_Curve()
{
    if (mCurve.has_value()) {
        mView->DoAddSheetCurveCommand(*mCurve);
        mCurve = std::nullopt;
    }
    mView->SetDrawingCurve(false);
    static_cast<CalChartFrame*>(GetParent())->ToolBarUnsetDrawingCurve();
}

CalChart::Select FieldCanvas::GetCurrentSelect() const { return mView ? mView->GetSelect() : CalChart::Select::Box; }
CalChart::MoveMode FieldCanvas::GetCurrentMove() const { return mView ? mView->GetCurrentMove() : CalChart::MoveMode::Normal; }
auto FieldCanvas::IsDrawingCurve() const -> bool { return mView ? mView->IsDrawingCurve() : false; }

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

void FieldCanvas::SetDrawingCurve(bool drawingCurve)
{
    if (mView) {
        mView->SetDrawingCurve(drawingCurve);
    }
}
