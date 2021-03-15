#pragma once
/*
 * FieldCanvas.h
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

#include "basic_ui.h"
#include "cc_types.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <wx/docview.h>

class CalChartView;
class CalChartFrame;
class BackgroundImage;
namespace CalChart {
class Shape;
class Coord;
class Shape_2point;
}
class Matrix;
class CalChartConfiguration;
class MovePoints;

using ShapeList = std::vector<std::unique_ptr<CalChart::Shape>>;

// Field Canvas controls how to paint and the first line control of user input
class FieldCanvas : public ClickDragCtrlScrollCanvas {
    using super = ClickDragCtrlScrollCanvas;
    DECLARE_EVENT_TABLE()

public:
    // Basic functions
    FieldCanvas(float def_zoom, CalChartView* view, wxWindow* parent, wxWindowID winid = wxID_ANY, wxPoint const& pos = wxDefaultPosition, wxSize const& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxNO_BORDER);
    ~FieldCanvas() override = default;

    void SetView(CalChartView* view);

    // Misc show functions
    float ZoomToFitFactor() const;
    virtual void SetZoom(float factor);

    auto GetCurrentLasso() const { return curr_lasso; }
    void SetCurrentLasso(CC_DRAG lasso);
    auto GetCurrentMove() const { return curr_move; }
    // implies a call to EndDrag()
    void SetCurrentMove(CC_MOVE_MODES move);

    void OnChar(wxKeyEvent& event);

private:
    void Init();

    // Event Handlers
    void OnFieldPaint(wxPaintEvent& event);
    void OnEraseBackground(wxEraseEvent& event);
    void OnMouseLeftDown(wxMouseEvent& event);
    void OnMouseLeftUp(wxMouseEvent& event);
    void OnMouseLeftDoubleClick(wxMouseEvent& event);
    void OnMouseRightDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event) override;
    void OnMousePinchToZoom(wxMouseEvent& event) override;

    // Internals
    void BeginSelectDrag(CC_DRAG type, const CalChart::Coord& start);
    void AddMoveDrag(CC_DRAG type, std::unique_ptr<CalChart::Shape> shape);
    void MoveDrag(const CalChart::Coord& end);
    void EndDrag();
    enum class direction { north,
        east,
        south,
        west };
    void MoveByKey(direction);
    CalChart::Coord GetMoveAmount(direction dir);
    CalChart::Coord SnapToGrid(CalChart::Coord c);
    CalChart::Coord SnapToolToGrid(CalChart::Coord c);

    // Background Picture
    void OnPaint(wxPaintEvent& event, const CalChartConfiguration& config);
    void PaintBackground(wxDC& dc, const CalChartConfiguration& config);
    void PaintShapes(wxDC& dc, CalChartConfiguration const& config, ShapeList const&);
    void PaintSelectShapes(wxDC& dc, CalChartConfiguration const& config);
    void PaintMoveShapes(wxDC& dc, CalChartConfiguration const& config);

    void OnMouseLeftDown_default(CalChart::Coord pos, bool shiftDown, bool altDown);
    void OnMouseLeftUp_default(CalChart::Coord pos, bool altDown);

    void OnMouseLeftDown_CC_DRAG_SWAP(CalChart::Coord pos);

    void OnMouseLeftUp_CC_DRAG_BOX(CalChart::Coord pos, bool altDown);
    void OnMouseLeftUp_CC_DRAG_LASSO(CalChart::Coord pos, bool altDown);
    void OnMouseLeftUp_CC_DRAG_POLY(CalChart::Coord pos, bool altDown);

    // utility
    CalChart::Coord TranslateMouseToCoord(wxClientDC& dc, wxMouseEvent& event);

    CalChartView* mView{};
    CC_DRAG curr_lasso = CC_DRAG::BOX;
    CC_MOVE_MODES curr_move = CC_MOVE_NORMAL;
    std::unique_ptr<MovePoints> m_move_points;
    std::map<int, CalChart::Coord> mMovePoints;
    CC_DRAG select_drag = CC_DRAG::NONE;
    ShapeList m_select_shape_list;
};
