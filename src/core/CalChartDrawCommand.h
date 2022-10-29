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
#include <span>
#include <variant>
#include <vector>

namespace CalChart {

namespace DrawCommands {

    struct Ignore {
    };

    struct Line {
        int x1{}, y1{}, x2{}, y2{};

        Line(int startx, int starty, int endx, int endy)
            : x1(startx)
            , y1(starty)
            , x2(endx)
            , y2(endy)
        {
        }

        Line(Coord start, Coord end)
            : Line(start.x, start.y, end.x, end.y)
        {
        }
    };

    struct Arc {
        int x1{}, y1{}, x2{}, y2{};
        int xc{}, yc{};

        Arc(int startx, int starty, int endx, int endy)
            : x1(startx)
            , y1(starty)
            , x2(endx)
            , y2(endy)
        {
        }

        Arc(Coord start, Coord end)
            : Arc(start.x, start.y, end.x, end.y)
        {
        }

        Arc(int startx, int starty, int endx, int endy, int centerx, int centery)
            : x1(startx)
            , y1(starty)
            , x2(endx)
            , y2(endy)
            , xc(centerx)
            , yc(centery)
        {
        }
    };

    struct Ellipse {
        int x1{}, y1{}, x2{}, y2{};

        Ellipse(int startx, int starty, int endx, int endy)
            : x1(startx)
            , y1(starty)
            , x2(endx)
            , y2(endy)
        {
        }

        Ellipse(Coord start, Coord end)
            : Ellipse(start.x, start.y, end.x, end.y)
        {
        }
    };

    struct Text {
        enum class TextAnchor : uint32_t {
            None = 0,
            Top = 1 << 0,
            VerticalCenter = 1 << 1,
            Bottom = 1 << 2,
            Left = 1 << 3,
            HorizontalCenter = 1 << 4,
            Right = 1 << 5,
            ScreenTop = 1 << 6,
            ScreenBottom = 1 << 7,
            ScreenRight = 1 << 8,
            ScreenLeft = 1 << 9,
        };
        using TextAnchor_t = std::underlying_type_t<TextAnchor>;

        int x{}, y{};
        std::string text;
        TextAnchor_t anchor{};
        bool withBackground{};

        Text(int startx, int starty, std::string const& text = "", TextAnchor_t anchor = toUType(TextAnchor::None), bool withBackground = false)
            : x(startx)
            , y(starty)
            , text(text)
            , anchor(anchor)
            , withBackground(withBackground)
        {
        }

        Text(Coord start, std::string const& text = "", TextAnchor_t anchor = toUType(TextAnchor::None), bool withBackground = false)
            : Text(start.x, start.y, text, anchor, withBackground)
        {
        }
    };

}

using DrawCommand = std::variant<DrawCommands::Ignore, DrawCommands::Line, DrawCommands::Arc, DrawCommands::Ellipse, DrawCommands::Text>;

namespace DrawCommands {

    // What follows are a collection of functions to create different common "images" for CalChart.
    namespace Field {
        // Construct commands for the field outline
        std::vector<CalChart::DrawCommand> CreateOutline(CalChart::Coord const& fieldsize, CalChart::Coord const& border1);

        // Construct commands for the vertical solid line down from the top at 8 step spacing
        std::vector<CalChart::DrawCommand> CreateVerticalSolidLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int step1);

        // Construct commands for the vertical dotted lines in between the Solid lines
        std::vector<CalChart::DrawCommand> CreateVerticalDottedLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int step1);

        // Construct commands for the horizontal dotted lines
        std::vector<CalChart::DrawCommand> CreateHorizontalDottedLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1);

        // Draw the hashes
        std::vector<CalChart::DrawCommand> CreateHashes(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1);
        std::vector<CalChart::DrawCommand> CreateHashTicks(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1);

        std::vector<CalChart::DrawCommand> CreateYardlineLabels(std::span<std::string const> yard_text, CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int offset, int step1);
    }

}
}
