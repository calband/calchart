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

#include "CalChartConstants.h"
#include "CalChartCoord.h"
#include "CalChartDrawPrimatives.h"
#include <span>
#include <variant>
#include <vector>

namespace CalChart {
class Point;

namespace DrawCommands {

    struct Ignore {
    };
    inline auto operator==([[maybe_unused]] Ignore lhs, [[maybe_unused]] Ignore rhs) { return true; }

    struct Line {
        Coord c1{};
        Coord c2{};

        // Assumed
        constexpr Line(Coord::units startx, Coord::units starty, Coord::units endx, Coord::units endy)
            : Line({ startx, starty }, { endx, endy })
        {
        }

        constexpr Line(Coord start, Coord end)
            : c1(start)
            , c2(end)
        {
        }
    };
    constexpr inline auto operator==(Line const& lhs, Line const& rhs)
    {
        return lhs.c1 == rhs.c1
            && lhs.c2 == rhs.c2;
    }

    struct Arc {
        Coord c1{};
        Coord c2{};
        Coord cc{};

        constexpr Arc(Coord::units startx, Coord::units starty, Coord::units endx, Coord::units endy, Coord::units centerx, Coord::units centery)
            : Arc({ startx, starty }, { endx, endy }, { centerx, centery })
        {
        }

        constexpr Arc(Coord start, Coord end, Coord center)
            : c1(start)
            , c2(end)
            , cc(center)
        {
        }
    };
    constexpr inline auto operator==(Arc const& lhs, Arc const& rhs)
    {
        return lhs.c1 == rhs.c1
            && lhs.c2 == rhs.c2
            && lhs.cc == rhs.cc;
    }

    struct Ellipse {
        Coord c1{};
        Coord c2{};

        constexpr Ellipse(Coord::units startx, Coord::units starty, Coord::units endx, Coord::units endy)
            : Ellipse({ startx, starty }, { endx, endy })
        {
        }

        constexpr Ellipse(Coord start, Coord end)
            : c1(start)
            , c2(end)
        {
        }
    };
    constexpr inline auto operator==(Ellipse const& lhs, Ellipse const& rhs)
    {
        return lhs.c1 == rhs.c1
            && lhs.c2 == rhs.c2;
    }

    struct Circle {
        Coord c1{};
        Coord::units radius{};
        bool filled = true;

        constexpr Circle(Coord::units startx, Coord::units starty, Coord::units radius, bool filled = true)
            : Circle({ startx, starty }, radius, filled)
        {
        }

        constexpr Circle(Coord start, int radius, bool filled = true)
            : c1(start)
            , radius(radius)
            , filled(filled)
        {
        }
    };
    constexpr inline auto operator==(Circle const& lhs, Circle const& rhs)
    {
        return lhs.c1 == rhs.c1
            && lhs.radius == rhs.radius
            && lhs.filled == rhs.filled;
    }

    struct Text {
        enum class TextAnchor : uint32_t {
            None = 0U,
            Top = 1U << 0U,
            VerticalCenter = 1U << 1U,
            Bottom = 1U << 2U,
            Left = 1U << 3U,
            HorizontalCenter = 1U << 4U,
            Right = 1U << 5U,
            ScreenTop = 1U << 6U,
            ScreenBottom = 1U << 7U,
            ScreenRight = 1U << 8U,
            ScreenLeft = 1U << 9U,
        };

        Coord c1{};
        std::string text;
        TextAnchor anchor{};
        bool withBackground{};

        Text(Coord::units startx, Coord::units starty, std::string text = "", TextAnchor anchor = TextAnchor::None, bool withBackground = false)
            : Text(Coord{ startx, starty }, text, anchor, withBackground)
        {
        }

        explicit Text(Coord start, std::string const& text = "", TextAnchor anchor = TextAnchor::None, bool withBackground = false)
            : c1(start)
            , text(text)
            , anchor(anchor)
            , withBackground(withBackground)
        {
        }
    };
    constexpr inline auto operator|(Text::TextAnchor lhs, Text::TextAnchor rhs)
    {
        return static_cast<Text::TextAnchor>(toUType(lhs) | toUType(rhs));
    }
    constexpr inline auto operator&(Text::TextAnchor lhs, Text::TextAnchor rhs)
    {
        return static_cast<Text::TextAnchor>(toUType(lhs) & toUType(rhs));
    }

    inline auto operator==(Text const& lhs, Text const& rhs)
    {
        return lhs.c1 == rhs.c1
            && lhs.text == rhs.text
            && lhs.anchor == rhs.anchor
            && lhs.withBackground == rhs.withBackground;
    }

    // DeviceDetails: This is the way that we specify the color/font for all the descent commands.
    struct OverrideFont;
    struct OverrideBrushAndPen;
}

using DrawCommand = std::variant<DrawCommands::Ignore, DrawCommands::Line, DrawCommands::Arc, DrawCommands::Ellipse, DrawCommands::Circle, DrawCommands::Text, DrawCommands::OverrideFont, DrawCommands::OverrideBrushAndPen>;

namespace DrawCommands {
    struct OverrideFont {
        Font font;
        std::vector<DrawCommand> commands{};
    };

    struct OverrideBrushAndPen {
        BrushAndPen brushAndPen;
        std::vector<DrawCommand> commands{};
    };

    inline auto withFont(Font font, std::vector<DrawCommand> commands) -> DrawCommand
    {
        return OverrideFont{
            font,
            std::move(commands)
        };
    }

    inline auto withFont(Font font, DrawCommand command)
    {
        return withFont(font, std::vector<DrawCommand>{ std::move(command) });
    }

    inline auto withBrushAndPen(BrushAndPen brushAndPen, std::vector<DrawCommand> commands) -> DrawCommand
    {
        return OverrideBrushAndPen{
            brushAndPen,
            std::move(commands)
        };
    }

    inline auto withBrushAndPen(BrushAndPen brushAndPen, DrawCommand command)
    {
        return withBrushAndPen(brushAndPen, std::vector<DrawCommand>{ std::move(command) });
    }

    constexpr inline auto operator==(OverrideFont const& lhs, OverrideFont const& rhs)
    {
        return lhs.font == rhs.font
            && lhs.commands == rhs.commands;
    }
    constexpr inline auto operator==(OverrideBrushAndPen const& lhs, OverrideBrushAndPen const& rhs)
    {
        return lhs.brushAndPen == rhs.brushAndPen
            && lhs.commands == rhs.commands;
    }
}
namespace DrawCommands {

    // What follows are a collection of functions to create different common "images" for CalChart.
    namespace Point {
        // Construct commands for drawing a point
        auto CreatePoint(CalChart::Point const& point, CalChart::Coord const& pos, std::string const& label, CalChart::SYMBOL_TYPE symbol, double dotRatio, double pLineRatio, double sLineRatio) -> std::vector<CalChart::DrawCommand>;
    }
    namespace Field {
        // Construct commands for the field outline
        auto CreateOutline(CalChart::Coord const& fieldsize, CalChart::Coord const& border1) -> std::vector<CalChart::DrawCommand>;

        // Construct commands for the vertical solid line down from the top at 8 step spacing
        auto CreateVerticalSolidLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int step1) -> std::vector<CalChart::DrawCommand>;

        // Construct commands for the vertical dotted lines in between the Solid lines
        auto CreateVerticalDottedLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int step1) -> std::vector<CalChart::DrawCommand>;

        // Construct commands for the horizontal dotted lines
        auto CreateHorizontalDottedLine(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>;

        // Draw the hashes
        auto CreateHashes(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>;
        auto CreateHashTicks(CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>;

        auto CreateYardlineLabels(std::span<std::string const> yard_text, CalChart::Coord const& fieldsize, CalChart::Coord const& border1, int offset, int step1) -> std::vector<CalChart::DrawCommand>;
    }

}

// Implementation Details
namespace DrawCommands::details {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    static_assert(Line{ 1, 2, 3, 4 }.c1.x == 1);
    static_assert(Line{ 1, 2, 3, 4 }.c1.y == 2);
    static_assert(Line{ 1, 2, 3, 4 }.c2.x == 3);
    static_assert(Line{ 1, 2, 3, 4 }.c2.y == 4);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c1.x == 1);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c1.y == 2);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c2.x == 3);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c2.y == 4);
    static_assert(Line{ 1, 2, 3, 4 } == Line{ { 1, 2 }, { 3, 4 } });

    static_assert(Arc{ 1, 2, 3, 4, 5, 6 } == Arc{ { 1, 2 }, { 3, 4 }, { 5, 6 } });
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}
}
