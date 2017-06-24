#pragma once
/*
 * field_canvas_shapes.h
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

#include "cc_types.h"

#include <wx/docview.h>

#include <vector>
#include <memory>
#include <functional>
#include <map>

class CC_shape;
class CC_coord;
class CalChartConfiguration;

class FieldCanvasShapes {
public:
    virtual ~FieldCanvasShapes() = default;
    void OnMove(const CC_coord& p, const CC_coord& snapped_p);
    auto& GetShapeList() const { return m_shape_list; }

    virtual void OnMouseLeftDown(CC_coord pos) = 0;
    virtual bool OnMouseUpDone(CC_coord const&) { return true; }
protected:
    void BeginMoveDrag(CC_DRAG_TYPES type, const CC_coord& start);
    void AddMoveDrag(CC_DRAG_TYPES type, std::unique_ptr<CC_shape> shape);

    std::vector<std::unique_ptr<CC_shape> > m_shape_list;
    CC_DRAG_TYPES move_drag = CC_DRAG_NONE;
};

class MovePoints : public FieldCanvasShapes {
public:
    virtual std::map<int, CC_coord> TransformPoints(std::map<int, CC_coord> const& select_list) const = 0;
};

std::unique_ptr<MovePoints> Create_MovePoints(CC_MOVE_MODES);
