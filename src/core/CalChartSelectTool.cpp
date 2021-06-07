/*
 * CalChartMovePoints.cpp
 * MovePoints represent the shapes and transformation of Dot manipulations.
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

#include "CalChartSelectTool.h"

#include "CalChartShapes.h"
#include "linmath.h"
#include "math_utils.h"

namespace CalChart {

SelectTool::SelectTool(CalChart::Select select, CalChart::Coord start, std::function<int(int)> userScale)
    : mSelect(select)
    , mUserScale(userScale)
{
    // Factory for creating a specific move.
    switch (select) {
    case CalChart::Select::Box:
        mLassoShape = std::make_unique<CalChart::Shape_rect>(start);
        break;
    case CalChart::Select::Poly:
        mLassoShape = std::make_unique<CalChart::Poly>(start);
        mSelectComplete = false;
        break;
    case CalChart::Select::Lasso:
        mLassoShape = std::make_unique<CalChart::Lasso>(start);
        break;
    default:
        break;
    }
}

void SelectTool::OnMove(CalChart::Coord p, CalChart::Coord snapped_p)
{
    if (mLassoShape) {
        mLassoShape->OnMove(p, snapped_p);
    }
}

void SelectTool::OnClickUp(CalChart::Coord pos)
{
    if (mSelect != CalChart::Select::Poly) {
        return;
    }
    static constexpr auto CLOSE_ENOUGH_TO_CLOSE = 10;
    auto* p = ((CalChart::Lasso*)mLassoShape.get())->FirstPoint();
    auto numPnts = ((CalChart::Lasso*)mLassoShape.get())->NumPoints();
    if (p != NULL && numPnts > 2) {
        // need to know where the scale is, so we need the device.
        auto polydist = mUserScale(CLOSE_ENOUGH_TO_CLOSE);
        auto d = p->x - pos.x;
        if (std::abs(d) < polydist) {
            d = p->y - pos.y;
            if (std::abs(d) < polydist) {
                mSelectComplete = true;
                return;
            }
        }
    }
    ((CalChart::Lasso*)mLassoShape.get())->Append(pos);
}

}
