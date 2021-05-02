#pragma once
/*
 * CC_DrawCommand.h
 * Class for how to draw
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartCoord.h"

namespace CalChart {

struct DrawCommand {
    typedef enum { Ignore,
        Line,
        Arc,
        Ellipse } DrawType;
    DrawType mType;
    int x1, y1, x2, y2;
    int xc, yc;
    // nothing version
    DrawCommand()
        : mType(Ignore)
    {
    }

    // Line version
    DrawCommand(int startx, int starty, int endx, int endy)
        : mType(Line)
        , x1(startx)
        , y1(starty)
        , x2(endx)
        , y2(endy)
    {
    }

    // Line version
    DrawCommand(const Coord& start, const Coord& end)
        : mType(Line)
        , x1(start.x)
        , y1(start.y)
        , x2(end.x)
        , y2(end.y)
    {
    }

    // Arc version
    DrawCommand(int startx, int starty, int endx, int endy, int centerx,
        int centery)
        : mType(Arc)
        , x1(startx)
        , y1(starty)
        , x2(endx)
        , y2(endy)
        , xc(centerx)
        , yc(centery)
    {
    }

    // Generic version
    DrawCommand(DrawType t, int startx, int starty, int endx, int endy)
        : mType(t)
        , x1(startx)
        , y1(starty)
        , x2(endx)
        , y2(endy)
    {
    }

    // Generic version
    DrawCommand(DrawType t, const Coord& start, const Coord& end)
        : mType(t)
        , x1(start.x)
        , y1(start.y)
        , x2(end.x)
        , y2(end.y)
    {
    }
};
}
