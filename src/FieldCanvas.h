#pragma once
/*
 * FieldCanvas.h
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

#include "CalChartMovePointsTool.h"
#include "CalChartPoint.h"
#include "CalChartSelectTool.h"
#include "CalChartShapes.h"
#include "CalChartTypes.h"
#include "basic_ui.h"

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
class Shape_2point;
class MovePoints;
class Configuration;
}
class Matrix;

using ShapeList = std::vector<std::unique_ptr<CalChart::Shape>>;

// Field Canvas controls how to paint and the first line control of user input
class FieldCanvas : public ClickDragCtrlScrollCanvas {
    using super = ClickDragCtrlScrollCanvas;
    DECLARE_EVENT_TABLE()

public:
    // Basic functions
    FieldCanvas(wxWindow* parent, CalChartView* view, float def_zoom, CalChart::Configuration const& config);
    ~FieldCanvas() override = default;

    void SetView(CalChartView* view);

    // Misc show functions
    float ZoomToFitFactor() const;
    virtual void SetZoom(float factor);
    virtual void SetZoomAroundCenter(float factor);

    CalChart::Select GetCurrentSelect() const;
    void SetCurrentSelect(CalChart::Select select);
    CalChart::MoveMode GetCurrentMove() const;
    // implies a call to EndDrag()
    void SetCurrentMove(CalChart::MoveMode move);
    void SetDrawingCurve(bool drawingCurve);

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
    void BeginSelectDrag(CalChart::Select select, CalChart::Coord start);
    void MoveDrag(CalChart::Coord end);
    void EndDrag();
    void BeginCurveControlPointDrag();
    void EndCurveControlPointDrag();

    enum class direction {
        north,
        east,
        south,
        west
    };
    void MoveByKey(direction);
    CalChart::Coord GetMoveAmount(direction dir);
    CalChart::Coord SnapToGrid(CalChart::Coord c);
    CalChart::Coord SnapToolToGrid(CalChart::Coord c);

    // Background Picture
    void OnPaint(wxPaintEvent& event, const CalChart::Configuration& config);
    void PaintBackground(wxDC& dc, const CalChart::Configuration& config);

    void OnMouseLeftDown_Normal(CalChart::Coord pos, bool shiftDown, bool altDown);
    void OnMouseLeftDown_Swap(CalChart::Coord pos);
    void OnMouseLeftDown_DrawingCurve(CalChart::Coord pos, bool shiftDown, bool altDown);
    void OnMouseLeftDown_FoundCurveControl(size_t curveIndex, size_t curveControl, bool shiftDown, bool altDown);
    void OnMouseLeftDown_FoundCurve(size_t curveIndex);
    void OnMouseLeftDoubleClick_Curve(CalChart::Coord pos);
    void OnMouseLeftDoubleClick_FoundCurve(CalChart::Coord pos, size_t whichCurve, size_t whereToInsert);

    void OnMouseMove_DrawCurve(CalChart::Coord pos);
    void OnMouseMove_DragCurveControlPoint(CalChart::Coord pos);
    void OnDelete_Curve();
    void OnEscape_Curve();

    [[nodiscard]] auto IsCurveDrawingMode() const -> bool;

    CalChartView* mView{};
    // The current selection
    std::optional<CalChart::SelectTool> mSelectTool;
    // we maintain the transient movement of points, and the selection list in Canvas.
    std::unique_ptr<CalChart::MovePointsTool> mMovePointsTool;
    // A cached list of the place where the selection list will move
    CalChart::MarcherToPosition mUncommittedMovePoints;
    CalChart::Configuration const& mConfig;

    CalChart::Coord mDragStart;

    // Curve related members
    struct ExistingCurveInfo {
        size_t mCurveSelected{};
        bool mShouldFlush{};
    };
    struct CurveDrawInfo {
        CalChart::Curve mCurve;
        std::set<size_t> mPointsSelected;
        std::optional<ExistingCurveInfo> mExistingCurve;
    };
    std::optional<CurveDrawInfo> mCurve;
};
