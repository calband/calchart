#pragma once
/*
 * CalChartDrawPrimatives.h
 * Header for calchart draw primatives
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

/**
 * CalChart Draw Primatives
 * These are objects that drawing elements like Color, Brush, Pen, Font for drawing elements of the Show and UI.
 */

#include <cstdint>
#include <vector>

namespace CalChart {

struct Color {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    uint8_t alpha = std::numeric_limits<uint8_t>::max();
    constexpr explicit Color(uint8_t red = 0, uint8_t green = 0, uint8_t blue = 0, uint8_t alpha = std::numeric_limits<uint8_t>::max())
        : red(red)
        , green(green)
        , blue(blue)
        , alpha(alpha)
    {
    }
    friend auto operator==(Color const&, Color const&) -> bool = default;
};

struct Brush {
    Color color;
    enum class Style {
        Solid,
        Transparent
    };
    Style style = Style::Solid;
    friend auto operator==(Brush const&, Brush const&) -> bool = default;

    static auto TransparentBrush() -> Brush
    {
        return CalChart::Brush{ Color(), Brush::Style::Transparent };
    }
};

struct Pen {
    Color color{};
    enum class Style {
        Solid,
        ShortDash,
    };
    Style style = Pen::Style::Solid;
    int width = 1;
    friend auto operator==(Pen const&, Pen const&) -> bool = default;
};

// for the cases where we share a color
struct BrushAndPen {
    Color color{};
    Brush::Style brushStyle = Brush::Style::Solid;
    Pen::Style penStyle = Pen::Style::Solid;
    int width = 1;
    friend auto operator==(BrushAndPen const&, BrushAndPen const&) -> bool = default;
};

inline auto toBrush(BrushAndPen b) { return Brush{ b.color, b.brushStyle }; }
inline auto toPen(BrushAndPen b) { return Pen{ b.color, b.penStyle, b.width }; }

struct Font {
    enum class Family {
        Swiss,
        Roman,
        Modern,
    };
    enum class Style {
        Normal,
        Italic,
    };
    enum class Weight {
        Normal,
        Bold,
    };
    Font(int size = 8, Family family = Family::Swiss, Style style = Style::Normal, Weight weight = Weight::Normal)
        : size(size)
        , family(family)
        , style(style)
        , weight(weight)
    {
    }

    int size = 8;
    Family family = Family::Swiss;
    Style style = Style::Normal;
    Weight weight = Weight::Normal;
    friend auto operator==(Font const&, Font const&) -> bool = default;
};
}
