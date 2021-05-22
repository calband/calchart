#pragma once
/*
 * CalChartSelectTool.h
 * Represents a way to select Marchers
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

#include "CalChartTypes.h"
#include "CalChartCoord.h"
#include "CalChartShapes.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

/**
 * CalChart MovePoints
 * MovePoints represent the way that a user may move Marchers around on a field.  These objects control both the shapes
 * that should be drawn to represent the move, but also the calculations on where the marchers will be after they move.
 * This allows the UI to create objects that represent how the user wants to move marchers on the field.  The "what" to draw
 * is returned by reading the ShapeList of the Move Points, and TransformPoints translates each point position to the new placement.
 *
 * Some MovePoints objects require multiple "clicks", for example like Rotate.  You would first click at the radius, then click
 * and drag around the arc.  So multiple clicks state are stored in each MovePoint.  Only when the "IsReadyForMoving" is true
 * does the TransformPoints result in meaningful data.
 *
 * Because the user may click again in a place they have clicked, we have the concept of "Done".  For example, on a rotate move
 * command the user may press on the Radius twice, for example to re-center.  So the IsDone means that this MovePoints object
 * will not handle any more "Clicks.
 */


namespace CalChart {
class Shape;

enum class Select {
    Box,
    Poly,
    Lasso,
    Swap,
};

class SelectTool {
public:
    SelectTool(CalChart::Select, CalChart::Coord start, std::function<int(int)> userScale);
    ~SelectTool() = default;

    CalChart::Shape const* GetShapeList() const { return mLassoShape.get(); }
    std::optional<CalChart::RawPolygon_t> GetPolygon() const { return mLassoShape ? mLassoShape->GetPolygon() : std::optional<CalChart::RawPolygon_t>{}; }

    // provides both the high resolution point and the low resolution point.
    void OnMove(CalChart::Coord p, CalChart::Coord snapped_p);

    void OnClickUp(CalChart::Coord);
    virtual bool SelectDone() const { return mSelectComplete; }

protected:

    std::unique_ptr<CalChart::Shape> mLassoShape;
    CalChart::Select mSelect = CalChart::Select::Box;
    bool mSelectComplete = true;
    std::function<int(int)> mUserScale;
};

}
