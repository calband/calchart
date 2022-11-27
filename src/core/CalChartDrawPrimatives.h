#pragma once
/*
 * CalChartDrawPrimatives.h
 * Header for calchart draw primatives
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

/**
 * CalChart Draw Primatives
 * These are objects that drawing elements like Color, Brush, Pen, Font for drawing elements of the Show and UI.
 */

#include <vector>

namespace CalChart {

struct Color {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t alpha = 0xff;
    Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = 0xff)
        : red(red)
        , green(green)
        , blue(blue)
        , alpha(alpha)
    {
    }
};

inline bool operator==(Color lhs, Color rhs)
{
    return lhs.red == rhs.red
        && lhs.green == rhs.green && lhs.blue == rhs.blue && lhs.alpha == rhs.alpha;
}

struct Brush {
    Color color;
    enum class Style {
        Solid,
        Transparent
    };
    Style style = Style::Solid;
};

struct Pen {
    Color color{};
    int width = 1;
};

// for the cases where we share a color
struct BrushAndPen {
    Color color{};
    Brush::Style style = Brush::Style::Solid;
    int width = 1;
};

}
