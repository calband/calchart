/*
 * field_canvas.h
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

#pragma once

#include "basic_ui.h"
#include "cc_types.h"

#include <wx/docview.h>

#include <vector>
#include <memory>
#include <functional>
#include <map>

class FieldView;
class FieldFrame;
class BackgroundImage;
namespace CalChart {
class Shape;
class Coord;
class Shape_2point;
}
class Matrix;
class CalChartConfiguration;
class MovePoints;

using ShapeList = std::vector<std::unique_ptr<CalChart::Shape> >;

// Field Canvas controls how to paint and the first line control of user input
class FieldCanvas : public ClickDragCtrlScrollCanvas {
    using super = ClickDragCtrlScrollCanvas;

public:
    // Basic functions
    FieldCanvas(wxWindow* win, FieldView& view, FieldFrame* frame, float def_zoom);
    virtual ~FieldCanvas() = default;
    void OnPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    virtual void OnMouseLeftDown(wxMouseEvent& event);
    virtual void OnMouseLeftUp(wxMouseEvent& event);
    virtual void OnMouseLeftDoubleClick(wxMouseEvent& event);
    virtual void OnMouseRightDown(wxMouseEvent& event);
    virtual void OnMouseMove(wxMouseEvent& event);
    virtual void OnMousePinchToZoom(wxMouseEvent& event);
    void OnChar(wxKeyEvent& event);

    // Misc show functions
    float ZoomToFitFactor() const;
    virtual void SetZoom(float factor);

    auto GetCurrentLasso() const { return curr_lasso; }
    void SetCurrentLasso(CC_DRAG_TYPES lasso);
    auto GetCurrentMove() const { return curr_move; }
    // implies a call to EndDrag()
    void SetCurrentMove(CC_MOVE_MODES move);

private:
    // Variables
    FieldFrame* mFrame;
    FieldView& mView;
    CC_DRAG_TYPES curr_lasso = CC_DRAG_BOX;
    CC_MOVE_MODES curr_move = CC_MOVE_NORMAL;
    std::unique_ptr<MovePoints> m_move_points;
    std::map<int, CalChart::Coord> mMovePoints;
    CC_DRAG_TYPES select_drag = CC_DRAG_NONE;
    ShapeList m_select_shape_list;

    void BeginSelectDrag(CC_DRAG_TYPES type, const CalChart::Coord& start);
    void AddMoveDrag(CC_DRAG_TYPES type, std::unique_ptr<CalChart::Shape> shape);
    void MoveDrag(const CalChart::Coord& end);
    void EndDrag();
    enum class direction { north,
        east,
        south,
        west };
    void MoveByKey(direction);
    CalChart::Coord GetMoveAmount(direction dir);
    CalChart::Coord SnapToGrid(CalChart::Coord c);

    // Background Picture
    void OnPaint(wxPaintEvent& event, const CalChartConfiguration& config);
    void PaintBackground(wxDC& dc, const CalChartConfiguration& config);
    void PaintShapes(wxDC& dc, CalChartConfiguration const& config, ShapeList const&);
    void PaintSelectShapes(wxDC& dc, CalChartConfiguration const& config);
    void PaintMoveShapes(wxDC& dc, CalChartConfiguration const& config);

    void OnMouseLeftDown_default(wxMouseEvent& event, CalChart::Coord pos);
    void OnMouseLeftUp_default(wxMouseEvent& event, CalChart::Coord pos);

    void OnMouseLeftDown_CC_DRAG_SWAP(CalChart::Coord pos);

    void OnMouseLeftUp_CC_DRAG_BOX(wxMouseEvent& event, CalChart::Coord pos);
    void OnMouseLeftUp_CC_DRAG_LASSO(wxMouseEvent& event, CalChart::Coord);
    void OnMouseLeftUp_CC_DRAG_POLY(wxMouseEvent& event, CalChart::Coord);

    DECLARE_EVENT_TABLE()
};
