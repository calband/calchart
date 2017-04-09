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
#include "modes.h"
#include "linmath.h"
#include "math_utils.h"
#include "background_image.h"
#include "cc_shapes.h"
#include "cc_drawcommand.h"
#include "draw.h"

#include <wx/dcbuffer.h>

static std::map<int, CC_coord> GetTransformedPoints(const Matrix& transmat, FieldView const& view)
{
    std::map<int, CC_coord> result;
    for (auto i : view.GetSelectionList()) {
        auto c = view.PointPosition(i);
        Vector v(c.x, c.y, 0);
        v = transmat * v;
        v.Homogenize();
        c = CC_coord(RoundToCoord(v.GetX()), RoundToCoord(v.GetY()));
        result[i] = c;
    }
    return result;
}


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
    , curr_lasso(CC_DRAG_BOX)
    , curr_move(CC_MOVE_NORMAL)
    , drag(CC_DRAG_NONE)
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

    PaintShapes(dc, config);
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
    const CalChartConfiguration& config)
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

// We have a empty erase background to improve redraw performance.
void FieldCanvas::OnEraseBackground(wxEraseEvent& event) {}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_LINE(CC_coord pos)
{
    pos = SnapToGrid(pos);

    // move is a 1 mouse click action
    BeginDrag(CC_DRAG_LINE, pos);

    mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
        assert(shape_list.size() == 1);
        const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
        auto start = shape->GetOrigin();
        auto second = shape->GetPoint();
        auto curr_pos = start;
        std::map<int, CC_coord> result;
        auto&& select_list = view.GetSelectionList();
        for (auto i = select_list.begin(); i != select_list.end(); ++i, curr_pos += second - start) {
            // should this have a snap to grid?
            result[*i] = curr_pos;
        }
        return result;
    };
}

void FieldCanvas::OnMouseLeftUp_CC_MOVE_LINE(CC_coord pos)
{
    mView.DoMovePoints(mMovePoints);
    SetCurrentMoveInternal(CC_MOVE_NORMAL);
}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_ROTATE(CC_coord pos)
{
    pos = SnapToGrid(pos);
    // rotate is a 2-distinct mouse click action
    if (shape_list.empty()) {
        BeginDrag(CC_DRAG_CROSS, pos);
    }
    else if (((CC_shape_1point*)shape_list.back().get())->GetOrigin() != pos) {
        // and this is a point not on the origin.
        AddDrag(CC_DRAG_LINE,
            std::unique_ptr<CC_shape>(new CC_shape_arc(
                ((CC_shape_1point*)shape_list.back().get())->GetOrigin(), pos)));
        // set up the place where points moves
        mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
            assert(shape_list.size() == 2);
            auto start = dynamic_cast<CC_shape_1point&>(*shape_list[0]).GetOrigin();
            auto r = -((CC_shape_arc*)shape_list.back().get())->GetAngle();
            auto m = TranslationMatrix(Vector(-start.x, -start.y, 0)) * ZRotationMatrix(r) * TranslationMatrix(Vector(start.x, start.y, 0));
            return GetTransformedPoints(m, view);
        };
    }
}

void FieldCanvas::OnMouseLeftUp_CC_MOVE_ROTATE(CC_coord pos)
{
    if (shape_list.size() > 1) {
        const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            // reset
            BeginDrag(CC_DRAG_CROSS, pos);
        }
        else {
            // apply move
            mView.DoMovePoints(mMovePoints);
            SetCurrentMoveInternal(CC_MOVE_NORMAL);
        }
    }
}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_SHEAR(CC_coord pos)
{
    pos = SnapToGrid(pos);
    // shear is a 2-distinct mouse click action
    if (shape_list.empty()) {
        BeginDrag(CC_DRAG_CROSS, pos);
    }
    else if (((CC_shape_1point*)shape_list.back().get())->GetOrigin() != pos) {
        CC_coord vect(pos - ((CC_shape_1point*)shape_list.back().get())->GetOrigin());
        // rotate vect 90 degrees
        AddDrag(CC_DRAG_LINE, std::unique_ptr<CC_shape>(new CC_shape_angline(
                                  pos, CC_coord(-vect.y, vect.x))));

        // set up the place where points moves
        mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
            assert(shape_list.size() == 2);
            auto start = dynamic_cast<CC_shape_1point&>(*shape_list[0]).GetOrigin();
            const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
            auto c1 = shape->GetOrigin();
            auto c2 = shape->GetPoint();
            auto v1 = c1 - start;
            auto v2 = c2 - c1;
            float amount = v2.Magnitude() / v1.Magnitude();
            if (BoundDirectionSigned(v1.Direction() - (c2 - start).Direction()) < 0) {
                amount = -amount;
            }
            float ang = -v1.Direction() * M_PI / 180.0;
            auto m = TranslationMatrix(Vector(-start.x, -start.y, 0)) * ZRotationMatrix(-ang) * YXShearMatrix(amount) * ZRotationMatrix(ang) * TranslationMatrix(Vector(start.x, start.y, 0));
            return GetTransformedPoints(m, view);
        };
    }
}

void FieldCanvas::OnMouseLeftUp_CC_MOVE_SHEAR(CC_coord pos)
{
    if (shape_list.size() > 1) {
        const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            BeginDrag(CC_DRAG_CROSS, pos);
        }
        else {
            mView.DoMovePoints(mMovePoints);
            SetCurrentMoveInternal(CC_MOVE_NORMAL);
        }
    }
}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_REFL(CC_coord pos)
{
    pos = SnapToGrid(pos);

    // reflect is a 1 mouse click action
    BeginDrag(CC_DRAG_LINE, pos);

    mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
        assert(shape_list.size() == 1);
        const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
        CC_coord c1 = shape->GetOrigin();
        CC_coord c2 = shape->GetPoint() - c1;
        float ang = -c2.Direction() * M_PI / 180.0;
        auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) * ZRotationMatrix(-ang) * YReflectionMatrix() * ZRotationMatrix(ang) * TranslationMatrix(Vector(c1.x, c1.y, 0));
        return GetTransformedPoints(m, view);
    };
}

void FieldCanvas::OnMouseLeftUp_CC_MOVE_REFL(CC_coord pos)
{
    mView.DoMovePoints(mMovePoints);
    SetCurrentMoveInternal(CC_MOVE_NORMAL);
}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_SIZE(CC_coord pos)
{
    pos = SnapToGrid(pos);
    // move is a 2-distinct mouse click action
    if (shape_list.empty()) {
        BeginDrag(CC_DRAG_CROSS, pos);
    }
    else if (((CC_shape_1point*)shape_list.back().get())->GetOrigin() != pos) {
        AddDrag(CC_DRAG_LINE,
            std::unique_ptr<CC_shape>(new CC_shape_line(pos)));
        mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
            auto& origin = dynamic_cast<CC_shape_1point&>(*shape_list[0]);
            CC_coord c1 = origin.GetOrigin();
            const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
            CC_coord c2 = shape->GetPoint() - c1;
            float sx = c2.x;
            float sy = c2.y;
            c2 = shape->GetOrigin() - c1;
            if ((c2.x != 0) || (c2.y != 0)) {
                if (c2.x != 0) {
                    sx /= c2.x;
                }
                else {
                    sx = 1;
                }
                if (c2.y != 0) {
                    sy /= c2.y;
                }
                else {
                    sy = 1;
                }
                auto m = TranslationMatrix(Vector(-c1.x, -c1.y, 0)) * ScaleMatrix(Vector(sx, sy, 0)) * TranslationMatrix(Vector(c1.x, c1.y, 0));
                return GetTransformedPoints(m, view);
            }
            return std::map<int, CC_coord>{};
        };
    }
}

void FieldCanvas::OnMouseLeftUp_CC_MOVE_SIZE(CC_coord pos)
{
    if (shape_list.size() > 1) {
        const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
        if (shape->GetOrigin() == shape->GetPoint()) {
            BeginDrag(CC_DRAG_CROSS, pos);
        }
        else {
            mView.DoMovePoints(mMovePoints);
            SetCurrentMoveInternal(CC_MOVE_NORMAL);
        }
    }
}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_GENIUS(CC_coord pos)
{
    pos = SnapToGrid(pos);
    if (shape_list.size() < 2)
    {
        AddDrag(CC_DRAG_LINE, std::unique_ptr<CC_shape>(new CC_shape_line(pos)));
    }
    else {
        mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
            CC_shape_2point* v1 = (CC_shape_2point*)shape_list[0].get();
            CC_shape_2point* v2 = (CC_shape_2point*)shape_list[1].get();
            CC_shape_2point* v3 = (CC_shape_2point*)shape_list[2].get();

            CC_coord s1 = v1->GetOrigin();
            CC_coord e1 = v1->GetPoint();
            CC_coord s2 = v2->GetOrigin();
            CC_coord e2 = v2->GetPoint();
            CC_coord s3 = v3->GetOrigin();
            CC_coord e3 = v3->GetPoint();
            auto d = (float)s1.x * (float)s2.y - (float)s2.x * (float)s1.y + (float)s3.x * (float)s1.y - (float)s1.x * (float)s3.y + (float)s2.x * (float)s3.y - (float)s3.x * (float)s2.y;
            if (IS_ZERO(d)) {
                return std::map<int, CC_coord>{};
            }
            else {
                Matrix A = Matrix(Vector(e1.x, e2.x, 0, e3.x), Vector(e1.y, e2.y, 0, e3.y),
                    Vector(0, 0, 0, 0), Vector(1, 1, 0, 1));
                Matrix Binv = Matrix(
                    Vector((float)s2.y - (float)s3.y, (float)s3.x - (float)s2.x, 0,
                        (float)s2.x * (float)s3.y - (float)s3.x * (float)s2.y),
                    Vector((float)s3.y - (float)s1.y, (float)s1.x - (float)s3.x, 0,
                        (float)s3.x * (float)s1.y - (float)s1.x * (float)s3.y),
                    Vector(0, 0, 0, 0),
                    Vector((float)s1.y - (float)s2.y, (float)s2.x - (float)s1.x, 0,
                        (float)s1.x * (float)s2.y - (float)s2.x * (float)s1.y));
                Binv /= d;
                Matrix m = Binv * A;
                return GetTransformedPoints(m, view);
            }
        };
    }
}

void FieldCanvas::OnMouseLeftUp_CC_MOVE_GENIUS(CC_coord pos)
{
    if (shape_list.size() > 2) {
        mView.DoMovePoints(mMovePoints);
        SetCurrentMoveInternal(CC_MOVE_NORMAL);
    }
}

void FieldCanvas::OnMouseLeftDown_CC_MOVE_SWAP(CC_coord pos)
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

void FieldCanvas::OnMouseLeftDown_default(wxMouseEvent& event, CC_coord pos)
{
    static constexpr auto CLOSE_ENOUGH_TO_CLOSE = 10;
    switch (drag) {
        case CC_DRAG_POLY: {
            auto* p = ((CC_lasso*)shape_list.back().get())->FirstPoint();
            if (p != NULL) {
                // need to know where the scale is, so we need the device.
                wxClientDC dc(this);
                PrepareDC(dc);
                Coord polydist = dc.DeviceToLogicalXRel(CLOSE_ENOUGH_TO_CLOSE);
                auto d = p->x - pos.x;
                if (std::abs(d) < polydist) {
                    d = p->y - pos.y;
                    if (std::abs(d) < polydist) {
                        mView.SelectWithLasso((CC_lasso*)shape_list.back().get(),
                            event.AltDown());
                        EndDrag();
                        break;
                    }
                }
            }
            ((CC_lasso*)shape_list.back().get())->Append(pos);
        }
            break;
        default:
            if (!(event.ShiftDown() || event.AltDown())) {
                mView.UnselectAll();
            }
            auto i = mView.FindPoint(pos);
            if (i < 0) {
                // if no point selected, we grab using the current lasso
                BeginDrag(curr_lasso, pos);
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

                BeginDrag(CC_DRAG_LINE, mView.PointPosition(i));
                mTransformer = [](ShapeList const& shape_list, FieldView const& view) {
                    const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
                    CC_coord pos = shape->GetPoint() - shape->GetOrigin();
                    std::map<int, CC_coord> result;
                    auto&& select_list = view.GetSelectionList();
                    for (auto i = select_list.begin(); i != select_list.end(); ++i) {
                        result[*i] = view.PointPosition(*i) + pos;
                    }
                    return result;
                };
            }
    }
}

void FieldCanvas::OnMouseLeftUp_default(wxMouseEvent& event)
{
    switch (drag) {
        case CC_DRAG_BOX:
        {
            const CC_shape_2point* shape = (CC_shape_2point*)shape_list.back().get();
            mView.SelectPointsInRect(shape->GetOrigin(), shape->GetPoint(),
                event.AltDown());
            EndDrag();
        }
            break;
        case CC_DRAG_LINE:
            mView.DoMovePoints(mMovePoints);
            EndDrag();
            break;
        case CC_DRAG_LASSO:
            ((CC_lasso*)shape_list.back().get())->End();
            mView.SelectWithLasso((CC_lasso*)shape_list.back().get(), event.AltDown());
            EndDrag();
            break;
        default:
            break;
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

        switch (curr_move) {
        case CC_MOVE_LINE:
            OnMouseLeftDown_CC_MOVE_LINE(pos);
            break;
        case CC_MOVE_ROTATE:
            OnMouseLeftDown_CC_MOVE_ROTATE(pos);
            break;
        case CC_MOVE_SHEAR:
            OnMouseLeftDown_CC_MOVE_SHEAR(pos);
            break;
        case CC_MOVE_REFL:
            OnMouseLeftDown_CC_MOVE_REFL(pos);
            break;
        case CC_MOVE_SIZE:
            OnMouseLeftDown_CC_MOVE_SIZE(pos);
            break;
        case CC_MOVE_GENIUS:
            OnMouseLeftDown_CC_MOVE_GENIUS(pos);
            break;
        case CC_MOVE_SWAP:
            OnMouseLeftDown_CC_MOVE_SWAP(pos);
            break;
        default:
            OnMouseLeftDown_default(event, pos);
            break;
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

        if (!shape_list.empty()) {
            switch (curr_move) {
            case CC_MOVE_LINE:
                OnMouseLeftUp_CC_MOVE_LINE(pos);
                break;
            case CC_MOVE_ROTATE:
                OnMouseLeftUp_CC_MOVE_ROTATE(pos);
                break;
            case CC_MOVE_SHEAR:
                OnMouseLeftUp_CC_MOVE_SHEAR(pos);
                break;
            case CC_MOVE_REFL:
                OnMouseLeftUp_CC_MOVE_REFL(pos);
                break;
            case CC_MOVE_SIZE:
                OnMouseLeftUp_CC_MOVE_SIZE(pos);
                break;
            case CC_MOVE_GENIUS:
                OnMouseLeftUp_CC_MOVE_GENIUS(pos);
                break;
            default:
                OnMouseLeftUp_default(event);
                break;
            }
        }
    }
    Refresh();
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);

    if (!shape_list.empty() && (CC_DRAG_POLY == drag)) {
        mView.SelectWithLasso((CC_lasso*)shape_list.back().get(), event.AltDown());
        EndDrag();
    }
    Refresh();
}

// Allow clicking within pixels to close polygons
void FieldCanvas::OnMouseRightDown(wxMouseEvent& event)
{
    wxClientDC dc(this);
    PrepareDC(dc);

    if (!shape_list.empty() && (CC_DRAG_POLY == drag)) {
        mView.SelectWithLasso((CC_lasso*)shape_list.back().get(), event.AltDown());
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

            if ((event.Dragging() && event.LeftIsDown() && !shape_list.empty()) || (event.Moving() && !shape_list.empty() && (CC_DRAG_POLY == drag))) {
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

void FieldCanvas::BeginDrag(CC_DRAG_TYPES type, const CC_coord& start)
{
    drag = type;
    shape_list.clear();
    switch (type) {
    case CC_DRAG_BOX:
        AddDrag(type, std::unique_ptr<CC_shape>(new CC_shape_rect(start)));
        break;
    case CC_DRAG_POLY:
        AddDrag(type, std::unique_ptr<CC_shape>(new CC_poly(start)));
        break;
    case CC_DRAG_LASSO:
        AddDrag(type, std::unique_ptr<CC_shape>(new CC_lasso(start)));
        break;
    case CC_DRAG_LINE:
        AddDrag(type, std::unique_ptr<CC_shape>(new CC_shape_line(start)));
        break;
    case CC_DRAG_CROSS:
        AddDrag(type,
            std::unique_ptr<CC_shape>(new CC_shape_cross(start, Int2Coord(2))));
    default:
        break;
    }
    std::map<unsigned, CC_coord> mPositions;
}

void FieldCanvas::AddDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape)
{
    drag = type;
    shape_list.emplace_back(std::move(shape));
}

void FieldCanvas::MoveDrag(const CC_coord& end)
{
    if (!shape_list.empty()) {
        shape_list.back()->OnMove(end, SnapToGrid(end));
    }
    if (mTransformer) {
        mMovePoints = mTransformer(shape_list, mView);
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
        move_points[*i] = mView.PointPosition(*i) + pos;
    }
    mView.DoMovePoints(move_points);
}

void FieldCanvas::EndDrag()
{
    mMovePoints.clear();
    shape_list.clear();
    mTransformer = nullptr;
    drag = CC_DRAG_NONE;
}

CC_DRAG_TYPES
FieldCanvas::GetCurrentLasso() const { return curr_lasso; }

void FieldCanvas::SetCurrentLasso(CC_DRAG_TYPES lasso) { curr_lasso = lasso; }

CC_MOVE_MODES
FieldCanvas::GetCurrentMove() const { return curr_move; }

// implies a call to EndDrag()
void FieldCanvas::SetCurrentMove(CC_MOVE_MODES move)
{
    EndDrag();
    curr_move = move;
}

// implies a call to EndDrag()
void FieldCanvas::SetCurrentMoveInternal(CC_MOVE_MODES move)
{
    mFrame->SetCurrentMove(move);
    EndDrag();
    curr_move = move;
}

