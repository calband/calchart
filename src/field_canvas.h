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

    CC_DRAG_TYPES GetCurrentLasso() const;
    void SetCurrentLasso(CC_DRAG_TYPES lasso);
    CC_MOVE_MODES GetCurrentMove() const;
    // implies a call to EndDrag()
    void SetCurrentMove(CC_MOVE_MODES move);

private:
//    std::map<int, CC_coord> GetTransformedPoints(const Matrix& transmat);

    // Variables
    FieldFrame* mFrame;
    FieldView& mView;
    CC_DRAG_TYPES curr_lasso;
    CC_MOVE_MODES curr_move;

    void BeginDrag(CC_DRAG_TYPES type, const CC_coord& start);
    void AddDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape);
    void MoveDrag(const CC_coord& end);
    void EndDrag();
    enum class direction { north,
        east,
        south,
        west };
    void MoveByKey(direction);
    CC_coord GetMoveAmount(direction dir);
    CC_coord SnapToGrid(CC_coord c);

    CC_DRAG_TYPES drag;
    using ShapeList = std::vector<std::unique_ptr<CC_shape>>;
    ShapeList shape_list;

    // Background Picture
    void OnPaint(wxPaintEvent& event, const CalChartConfiguration& config);
    void PaintBackground(wxDC& dc, const CalChartConfiguration& config);
    void PaintShapes(wxDC& dc, const CalChartConfiguration& config);

    void OnMouseLeftDown_CC_MOVE_LINE(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_LINE(CC_coord pos);

    void OnMouseLeftDown_CC_MOVE_ROTATE(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_ROTATE(CC_coord pos);

    void OnMouseLeftDown_CC_MOVE_SHEAR(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_SHEAR(CC_coord pos);

    void OnMouseLeftDown_CC_MOVE_REFL(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_REFL(CC_coord pos);

    void OnMouseLeftDown_CC_MOVE_SIZE(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_SIZE(CC_coord pos);

    void OnMouseLeftDown_CC_MOVE_GENIUS(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_GENIUS(CC_coord pos);

    void OnMouseLeftDown_CC_MOVE_SWAP(CC_coord pos);
    void OnMouseLeftUp_CC_MOVE_SWAP(CC_coord pos);

    void OnMouseLeftDown_default(wxMouseEvent& event, CC_coord pos);
    void OnMouseLeftUp_default(wxMouseEvent& event);

    // implies a call to EndDrag()
    void SetCurrentMoveInternal(CC_MOVE_MODES move);

    std::map<int, CC_coord> mMovePoints;
    std::function<std::map<int, CC_coord>(ShapeList const& shape_list, FieldView const&)> mTransformer;

    DECLARE_EVENT_TABLE()
};
