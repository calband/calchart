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
#include <algorithm>
#include <compare>
#include <optional>
#include <span>
#include <variant>
#include <vector>

namespace CalChart {
class Point;

namespace Draw {

    struct Ignore {
        friend auto operator==(Ignore const&, Ignore const&) -> bool = default;
    };
    inline constexpr auto operator+([[maybe_unused]] Ignore lhs, [[maybe_unused]] Coord rhs) { return Ignore{}; }
    inline constexpr auto operator+([[maybe_unused]] Coord lhs, [[maybe_unused]] Ignore rhs) { return Ignore{}; }

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
        friend auto operator==(Line const&, Line const&) -> bool = default;
    };
    inline constexpr auto operator+(Line lhs, Coord rhs) { return Line{ lhs.c1 + rhs, lhs.c2 + rhs }; }
    inline constexpr auto operator+(Coord lhs, Line rhs) { return rhs + lhs; }

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
        friend auto operator==(Arc const&, Arc const&) -> bool = default;
    };
    inline constexpr auto operator+(Arc lhs, Coord rhs) { return Arc{ lhs.c1 + rhs, lhs.c2 + rhs, lhs.cc + rhs }; }
    inline constexpr auto operator+(Coord lhs, Arc rhs) { return rhs + lhs; }

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
        friend auto operator==(Ellipse const&, Ellipse const&) -> bool = default;
    };
    inline constexpr auto operator+(Ellipse lhs, Coord rhs) { return Ellipse{ lhs.c1 + rhs, lhs.c2 + rhs }; }
    inline constexpr auto operator+(Coord lhs, Ellipse rhs) { return rhs + lhs; }

    struct Circle {
        Coord c1{};
        Coord::units radius{};
        bool filled = true;

        constexpr Circle(Coord::units startx, Coord::units starty, Coord::units radius, bool filled = true)
            : Circle({ startx, starty }, radius, filled)
        {
        }

        constexpr Circle(Coord start, Coord::units radius, bool filled = true)
            : c1(start)
            , radius(radius)
            , filled(filled)
        {
        }
        friend auto operator==(Circle const&, Circle const&) -> bool = default;
    };
    inline constexpr auto operator+(Circle lhs, Coord rhs) { return Circle{ lhs.c1 + rhs, lhs.radius, lhs.filled }; }
    inline constexpr auto operator+(Coord lhs, Circle rhs) { return rhs + lhs; }

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
            : Text(Coord{ startx, starty }, std::move(text), anchor, withBackground)
        {
        }

        explicit Text(Coord start, std::string text = "", TextAnchor anchor = TextAnchor::None, bool withBackground = false)
            : c1(start)
            , text(std::move(text))
            , anchor(anchor)
            , withBackground(withBackground)
        {
        }
        friend auto operator==(Text const&, Text const&) -> bool = default;
    };
    constexpr inline auto operator|(Text::TextAnchor lhs, Text::TextAnchor rhs)
    {
        return static_cast<Text::TextAnchor>(toUType(lhs) | toUType(rhs));
    }
    constexpr inline auto operator&(Text::TextAnchor lhs, Text::TextAnchor rhs)
    {
        return static_cast<Text::TextAnchor>(toUType(lhs) & toUType(rhs));
    }
    inline auto operator+(Text const& lhs, Coord rhs) { return Text{ lhs.c1 + rhs, lhs.text, lhs.anchor, lhs.withBackground }; }
    inline auto operator+(Coord lhs, Text const& rhs) { return rhs + lhs; }

    // DeviceDetails: This is the way that we specify the color/font for all the descent commands.
    // These are forward declared because they are composed of DrawCommand instances
    struct OverrideFont;
    struct OverrideBrush;
    struct OverridePen;
    struct OverrideBrushAndPen;
    struct OverrideTextForeground;
}

using DrawCommand = std::variant<
    Draw::Ignore,
    Draw::Line,
    Draw::Arc,
    Draw::Ellipse,
    Draw::Circle,
    Draw::Text,
    Draw::OverrideFont,
    Draw::OverrideBrush,
    Draw::OverridePen,
    Draw::OverrideBrushAndPen,
    Draw::OverrideTextForeground>;

namespace Draw {
    struct OverrideFont {
        std::optional<Font> font;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideFont const& lhs, Coord rhs) -> OverrideFont;
    inline auto operator+(Coord lhs, OverrideFont const& rhs) -> OverrideFont;

    struct OverrideBrush {
        std::optional<Brush> brush;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideBrush const& lhs, Coord rhs) -> OverrideBrush;
    inline auto operator+(Coord lhs, OverrideBrush const& rhs) -> OverrideBrush;

    struct OverridePen {
        std::optional<Pen> pen;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverridePen const& lhs, Coord rhs) -> OverridePen;
    inline auto operator+(Coord lhs, OverridePen const& rhs) -> OverridePen;

    struct OverrideBrushAndPen {
        std::optional<BrushAndPen> brushAndPen;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideBrushAndPen const& lhs, Coord rhs) -> OverrideBrushAndPen;
    inline auto operator+(Coord lhs, OverrideBrushAndPen const& rhs) -> OverrideBrushAndPen;

    struct OverrideTextForeground {
        std::optional<BrushAndPen> brushAndPen;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideTextForeground const& lhs, Coord rhs) -> OverrideTextForeground;
    inline auto operator+(Coord lhs, OverrideTextForeground const& rhs) -> OverrideTextForeground;

    constexpr inline auto operator==(OverrideFont const& lhs, OverrideFont const& rhs)
    {
        return lhs.font == rhs.font
            && lhs.commands == rhs.commands;
    }
    constexpr inline auto operator==(OverrideBrush const& lhs, OverrideBrush const& rhs)
    {
        return lhs.brush == rhs.brush
            && lhs.commands == rhs.commands;
    }
    constexpr inline auto operator==(OverridePen const& lhs, OverridePen const& rhs)
    {
        return lhs.pen == rhs.pen
            && lhs.commands == rhs.commands;
    }
    constexpr inline auto operator==(OverrideBrushAndPen const& lhs, OverrideBrushAndPen const& rhs)
    {
        return lhs.brushAndPen == rhs.brushAndPen
            && lhs.commands == rhs.commands;
    }
    constexpr inline auto operator==(OverrideTextForeground const& lhs, OverrideTextForeground const& rhs)
    {
        return lhs.brushAndPen == rhs.brushAndPen
            && lhs.commands == rhs.commands;
    }

    inline auto operator+(CalChart::DrawCommand const& lhs, Coord rhs) -> CalChart::DrawCommand
    {
        return std::visit(overloaded{
                              [rhs](auto arg) { return CalChart::DrawCommand{ arg + rhs }; },
                          },
            lhs);
    }

    inline auto operator+(std::vector<CalChart::DrawCommand> const& lhs, Coord rhs) -> std::vector<CalChart::DrawCommand>
    {
        std::vector<DrawCommand> result;
        result.reserve(lhs.size());
        std::transform(lhs.begin(), lhs.end(), std::back_inserter(result), [rhs](auto const& a) { return a + rhs; });
        return result;
    }
    inline auto operator+(Coord lhs, std::vector<CalChart::DrawCommand> const& rhs) -> std::vector<CalChart::DrawCommand> { return rhs + lhs; }

    inline auto operator+(OverrideFont const& lhs, Coord rhs) -> OverrideFont { return OverrideFont{ lhs.font, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideFont const& rhs) -> OverrideFont { return rhs + lhs; }
    inline auto operator+(OverrideBrush const& lhs, Coord rhs) -> OverrideBrush { return OverrideBrush{ lhs.brush, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideBrush const& rhs) -> OverrideBrush { return rhs + lhs; }
    inline auto operator+(OverridePen const& lhs, Coord rhs) -> OverridePen { return OverridePen{ lhs.pen, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverridePen const& rhs) -> OverridePen { return rhs + lhs; }
    inline auto operator+(OverrideBrushAndPen const& lhs, Coord rhs) -> OverrideBrushAndPen { return OverrideBrushAndPen{ lhs.brushAndPen, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideBrushAndPen const& rhs) -> OverrideBrushAndPen { return rhs + lhs; }
    inline auto operator+(OverrideTextForeground const& lhs, Coord rhs) -> OverrideTextForeground { return OverrideTextForeground{ lhs.brushAndPen, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideTextForeground const& rhs) -> OverrideTextForeground { return rhs + lhs; }

    inline auto withFont(Font font, std::vector<DrawCommand> commands) -> DrawCommand { return OverrideFont{ font, std::move(commands) }; }
    inline auto withFont(Font font, DrawCommand command) { return withFont(font, std::vector<DrawCommand>{ std::move(command) }); }

    inline auto withBrush(Brush brush, std::vector<DrawCommand> commands) -> DrawCommand { return OverrideBrush{ brush, std::move(commands) }; }
    inline auto withBrush(Brush brush, DrawCommand command) { return withBrush(brush, std::vector<DrawCommand>{ std::move(command) }); }

    inline auto withPen(Pen pen, std::vector<DrawCommand> commands) -> DrawCommand { return OverridePen{ pen, std::move(commands) }; }
    inline auto withPen(Pen pen, DrawCommand command) { return withPen(pen, std::vector<DrawCommand>{ std::move(command) }); }

    inline auto withBrushAndPen(std::optional<BrushAndPen> brushAndPen, std::vector<DrawCommand> commands) -> DrawCommand { return OverrideBrushAndPen{ brushAndPen, std::move(commands) }; }
    inline auto withBrushAndPen(std::optional<BrushAndPen> brushAndPen, DrawCommand command) { return withBrushAndPen(brushAndPen, std::vector<DrawCommand>{ std::move(command) }); }

    inline auto withTextForeground(BrushAndPen brushAndPen, std::vector<DrawCommand> commands) -> DrawCommand { return OverrideTextForeground{ brushAndPen, std::move(commands) }; }
    inline auto withTextForeground(BrushAndPen brushAndPen, DrawCommand command) { return withTextForeground(brushAndPen, std::vector<DrawCommand>{ std::move(command) }); }

}

namespace Draw {

    // What follows are a collection of functions to create different common "images" for CalChart.
    namespace Point {
        // Construct commands for drawing a point
        auto CreatePoint(CalChart::Point const& point, CalChart::Coord const& pos, std::string const& label, CalChart::SYMBOL_TYPE symbol, double dotRatio, double pLineRatio, double sLineRatio) -> std::vector<CalChart::DrawCommand>;
    }
    namespace Field {
        // Construct commands for the field outline
        auto CreateOutline(CalChart::Coord const& fieldsize) -> std::vector<CalChart::DrawCommand>;

        // Construct commands for the vertical solid line down from the top at 8 step spacing
        auto CreateVerticalSolidLine(CalChart::Coord const& fieldsize, int step1) -> std::vector<CalChart::DrawCommand>;

        // Construct commands for the vertical dotted lines in between the Solid lines
        auto CreateVerticalDottedLine(CalChart::Coord const& fieldsize, int step1) -> std::vector<CalChart::DrawCommand>;

        // Construct commands for the horizontal dotted lines
        auto CreateHorizontalDottedLine(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>;

        // Draw the hashes
        auto CreateHashes(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>;
        auto CreateHashTicks(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<CalChart::DrawCommand>;

        auto CreateYardlineLabels(std::vector<std::string> const& yard_text, CalChart::Coord const& fieldsize, int offset, int step1) -> std::vector<CalChart::DrawCommand>;
    }

}

// Implementation Details
namespace Draw::details {
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    static_assert(Ignore{} == Ignore{});
    static_assert(Ignore{} == (Ignore{} + Coord{}));
    static_assert(Ignore{} == (Coord{} + Ignore{}));

    static_assert(Line{ 1, 2, 3, 4 }.c1.x == 1);
    static_assert(Line{ 1, 2, 3, 4 }.c1.y == 2);
    static_assert(Line{ 1, 2, 3, 4 }.c2.x == 3);
    static_assert(Line{ 1, 2, 3, 4 }.c2.y == 4);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c1.x == 1);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c1.y == 2);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c2.x == 3);
    static_assert(Line{ { 1, 2 }, { 3, 4 } }.c2.y == 4);
    static_assert(Line{ 1, 2, 3, 4 } == Line{ { 1, 2 }, { 3, 4 } });
    static_assert(Line{ { 1, 3 }, { 3, 5 } } == (Line{ { 0, 1 }, { 2, 3 } } + Coord{ 1, 2 }));
    static_assert(Line{ { 3, 6 }, { 5, 8 } } == (Coord{ 2, 4 } + Line{ { 1, 2 }, { 3, 4 } }));

    static_assert(Arc{ 1, 2, 3, 4, 5, 6 } == Arc{ { 1, 2 }, { 3, 4 }, { 5, 6 } });
    static_assert(Arc{ { 1, 3 }, { 3, 5 }, { 5, 7 } } == (Arc{ { 0, 1 }, { 2, 3 }, { 4, 5 } } + Coord{ 1, 2 }));
    static_assert(Arc{ { 3, 6 }, { 5, 8 }, { 7, 10 } } == (Coord{ 2, 4 } + Arc{ { 1, 2 }, { 3, 4 }, { 5, 6 } }));

    static_assert(Ellipse{ 1, 2, 3, 4 } == Ellipse{ { 1, 2 }, { 3, 4 } });
    static_assert(Ellipse{ { 1, 3 }, { 3, 5 } } == (Ellipse{ { 0, 1 }, { 2, 3 } } + Coord{ 1, 2 }));
    static_assert(Ellipse{ { 3, 6 }, { 5, 8 } } == (Coord{ 2, 4 } + Ellipse{ { 1, 2 }, { 3, 4 } }));

    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}
}
