#pragma once
/*
 * CalChartMovePointsTool.h
 * MovePointsTool represent the shapes and transformation of Marcher moves.
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

#include "CalChartCoord.h"
#include "CalChartShapes.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace CalChart {

enum class MoveMode {
    Normal,
    ShapeLine,
    ShapeX,
    ShapeCross,
    ShapeRectange,
    ShapeEllipse,
    ShapeDraw,
    MoveLine,
    MoveRotate,
    MoveShear,
    MoveReflect,
    MoveSize,
    MoveGenius,
};

class MovePointsTool {
public:
    // Factory for creating a specific move.
    static std::unique_ptr<MovePointsTool> Create(CalChart::MoveMode);
    virtual ~MovePointsTool() = default;

    auto& GetShapeList() const { return m_shape_list; }
    virtual std::map<int, CalChart::Coord> TransformPoints(std::map<int, CalChart::Coord> const& select_list) const = 0;

    // provides both the high resolution point and the low resolution point.

    template <typename Function>
    void OnMove(CalChart::Coord p, Function snapper)
    {
        if (!m_shape_list.empty()) {
            m_shape_list.back()->OnMove(p, snapper);
        }
    }

    virtual void OnClickDown(CalChart::Coord pos) = 0;
    virtual void OnClickUp(CalChart::Coord){};
    virtual bool IsDone() const { return true; }
    virtual bool IsReadyForMoving() const { return true; }

protected:
    enum class Drag {
        NONE,
        BOX,
        POLY,
        LASSO,
        LINE,
        CROSSHAIRS,
        SHAPE_ELLIPSE,
        SHAPE_X,
        SHAPE_CROSS,
        SWAP,
    };

    void BeginMoveDrag(Drag type, CalChart::Coord start);
    void AddMoveDrag(Drag type, std::unique_ptr<CalChart::Shape> shape);

    std::vector<std::unique_ptr<CalChart::Shape>> m_shape_list;
    Drag move_drag = Drag::NONE;
};

}
