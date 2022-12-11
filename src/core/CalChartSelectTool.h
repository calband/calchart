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

/**
 * CalChart SelectTool
 * SelectTool represent the way the selection tool that for the field,
 * effectively the collection of points that represents the box/polygon/lasso
 * that you use to select the points.
 */

#include "CalChartShapes.h"
#include "CalChartTypes.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace CalChart {
class Shape;
struct Coord;

enum class Select {
    Box,
    Poly,
    Lasso,
    Swap,
};

class SelectTool {
public:
    SelectTool(CalChart::Select, CalChart::Coord start, std::function<int(int)> userScale);

    [[nodiscard]] auto GetShapeList() const -> CalChart::Shape const*
    {
        return mLassoShape.get();
    }

    [[nodiscard]] auto GetPolygon() const -> std::optional<CalChart::RawPolygon_t>
    {
        return mLassoShape ? mLassoShape->GetPolygon() : std::optional<CalChart::RawPolygon_t>{};
    }

    // provides both the high resolution point and the low resolution point.
    template <typename Function>
    void OnMove(CalChart::Coord p, Function snapper)
    {
        if (mLassoShape) {
            mLassoShape->OnMove(p, snapper);
        }
    }

    void OnClickUp(CalChart::Coord);

    [[nodiscard]] auto SelectDone() const
    {
        return mSelectComplete;
    }

private:
    std::unique_ptr<CalChart::Shape> mLassoShape;
    CalChart::Select mSelect = CalChart::Select::Box;
    bool mSelectComplete = true;
    std::function<int(int)> mUserScale;
};

}
