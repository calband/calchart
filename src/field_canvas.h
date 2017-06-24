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
class CC_shape;
class BackgroundImage;
class CC_coord;
class Matrix;
class CalChartConfiguration;
class CC_shape_2point;
class MovePoints;

using ShapeList = std::vector<std::unique_ptr<CC_shape> >;

// Field Canvas controls how to paint and the first line control of user input
class FieldCanvas : public ClickDragCtrlScrollCanvas {
    using super = ClickDragCtrlScrollCanvas;

public:
    // Basic functions
    FieldCanvas(FieldView& view, FieldFrame* frame, float def_zoom);
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
    std::map<int, CC_coord> mMovePoints;
    CC_DRAG_TYPES select_drag = CC_DRAG_NONE;
    ShapeList m_select_shape_list;

    void BeginSelectDrag(CC_DRAG_TYPES type, const CC_coord& start);
    void AddMoveDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape);
    void MoveDrag(const CC_coord& end);
    void EndDrag();
    enum class direction { north,
        east,
        south,
        west };
    void MoveByKey(direction);
    CC_coord GetMoveAmount(direction dir);
    CC_coord SnapToGrid(CC_coord c);

    // Background Picture
    void OnPaint(wxPaintEvent& event, const CalChartConfiguration& config);
    void PaintBackground(wxDC& dc, const CalChartConfiguration& config);
    void PaintShapes(wxDC& dc, CalChartConfiguration const& config, ShapeList const&);
    void PaintSelectShapes(wxDC& dc, CalChartConfiguration const& config);
    void PaintMoveShapes(wxDC& dc, CalChartConfiguration const& config);

    void OnMouseLeftDown_default(wxMouseEvent& event, CC_coord pos);
    void OnMouseLeftUp_default(wxMouseEvent& event, CC_coord pos);

    void OnMouseLeftDown_CC_DRAG_SWAP(CC_coord pos);

    void OnMouseLeftUp_CC_DRAG_BOX(wxMouseEvent& event, CC_coord pos);
    void OnMouseLeftUp_CC_DRAG_LASSO(wxMouseEvent& event, CC_coord);
    void OnMouseLeftUp_CC_DRAG_POLY(wxMouseEvent& event, CC_coord);

    DECLARE_EVENT_TABLE()
};
