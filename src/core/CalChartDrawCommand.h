#pragma once
/*
 * CC_DrawCommand.h
 * Class for how to draw
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

#include "CalChartConstants.h"
#include "CalChartCoord.h"
#include "CalChartDrawPrimatives.h"
#include "CalChartImage.h"
#include <algorithm>
#include <compare>
#include <functional>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <variant>
#include <vector>

namespace CalChart {
class Point;

// These provide a way for CalChart to specify how to draw specific shapes/images.
// The offset and size allows greater control of placing the image.
//
// We have 3 different types of things that we draw:
//  * DrawItems, which are the draw primatives like lines, ellipses, and text.
//  * MetaManipulators, which modify meta aspects of drawing like colors and fonts of a collection of DrawItems.
//  * Layout, which dictate how a collection of items should be stacked.
// this allows the building up of tree like data structures for drawing, with the DrawItems being the leaf nodes.
//
// There are 3 types of Layout, Horizontal (X-axis), Vertical (Y-axis), and Depthwise (Z-axis).  There are 4 ways
// of stacking items:
// Layed out so they are directly adjacent to each other at the beginning of a layout (`StackAlign::Begin`):
// | o o o           |
// Layed out so they are directly adjacent to each other at the end of a layout (`StackAlign::End`):
// |           o o o |
// Layed out so they are spread out so there is 1/2 the spacing at the edges as a uniform layout (`StackAlign::Uniform`):
// |   o    o    o   |
// Layed out so they are spread evenly from edge to edge (`StackAlign::Justified`):
// | o      o      o |
//
// A layout will have a concept of a minimum size.  A top level layout will spread to the widge of the area, but
// a nested layout will be constrained to the minimum size.
// For example: HStack[middle]( Circle, Circle, Circle ):
// | o      o      o |
// Whereas: HStack[middle]( HStack[middle]( Circle, Circle, Circle ) )
// |      o o o      |
//
//
namespace Draw {
    // items
    struct Ignore;
    struct Line;
    struct Arc;
    struct Ellipse;
    struct Circle;
    struct Rectangle;
    struct Text;
    struct Image;
    struct Tab;
    using DrawItems = std::variant<
        Ignore,
        Line,
        Arc,
        Ellipse,
        Circle,
        Rectangle,
        Text,
        Image,
        Tab>;

    // meta manipulators
    // This is the way that we specify the color/font for all the descent commands.
    struct OverrideFont;
    struct OverrideTextForeground;
    struct OverrideBrush;
    struct OverridePen;
    struct OverrideBrushAndPen;
    using DrawManipulators = std::variant<
        OverrideFont,
        OverrideTextForeground,
        OverrideBrush,
        OverridePen,
        OverrideBrushAndPen>;

    // These are the way we can specify how to lay out images.
    struct VStack;
    struct HStack;
    struct ZStack;
    using DrawStack = std::variant<
        VStack,
        HStack,
        ZStack>;

    using DrawCommand = std::variant<
        Draw::DrawItems,
        Draw::DrawManipulators,
        Draw::DrawStack>;

    struct Ignore {
        friend auto operator==(Ignore const&, Ignore const&) -> bool = default;
    };
    inline constexpr auto operator+([[maybe_unused]] Ignore lhs, [[maybe_unused]] Coord rhs) { return Ignore{}; }
    inline constexpr auto operator+([[maybe_unused]] Coord lhs, [[maybe_unused]] Ignore rhs) { return Ignore{}; }
    inline constexpr auto operator-([[maybe_unused]] Ignore lhs, [[maybe_unused]] Coord rhs) { return Ignore{}; }
    inline constexpr auto operator-([[maybe_unused]] Coord lhs, [[maybe_unused]] Ignore rhs) { return Ignore{}; }

    struct Tab {
        // given the number of spaces, size, return the next spaces it should go to, effectively a ceiling
        std::function<int(int)> tabsToStop = [](int i) { return ((i / 8) + 1) * 8; };
    };
    inline auto operator+(Tab lhs, [[maybe_unused]] Coord rhs) { return Tab{ lhs.tabsToStop }; }
    inline auto operator+([[maybe_unused]] Coord lhs, Tab rhs) { return Tab{ rhs.tabsToStop }; }
    inline auto operator-(Tab lhs, [[maybe_unused]] Coord rhs) { return Tab{ lhs.tabsToStop }; }
    inline auto operator-([[maybe_unused]] Coord lhs, Tab rhs) { return Tab{ rhs.tabsToStop }; }
    // we assume tabs always compare true... This may not always be the case, but it is the choice we make.
    inline auto operator==([[maybe_unused]] Tab const&, [[maybe_unused]] Tab const&) -> bool { return true; }

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
    inline constexpr auto operator+(Coord lhs, Line rhs) { return Line{ lhs + rhs.c1, lhs + rhs.c2 }; }
    inline constexpr auto operator-(Line lhs, Coord rhs) { return Line{ lhs.c1 - rhs, lhs.c2 - rhs }; }
    inline constexpr auto operator-(Coord lhs, Line rhs) { return Line{ lhs - rhs.c1, lhs - rhs.c2 }; }

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
    inline constexpr auto operator+(Coord lhs, Arc rhs) { return Arc{ lhs + rhs.c1, lhs + rhs.c2, lhs + rhs.cc }; }
    inline constexpr auto operator-(Arc lhs, Coord rhs) { return Arc{ lhs.c1 - rhs, lhs.c2 - rhs, lhs.cc - rhs }; }
    inline constexpr auto operator-(Coord lhs, Arc rhs) { return Arc{ lhs - rhs.c1, lhs - rhs.c2, lhs - rhs.cc }; }

    struct Ellipse {
        Coord c1{};
        Coord c2{};
        bool filled = true;

        std::optional<Coord> size;
        constexpr Ellipse(Coord::units startx, Coord::units starty, Coord::units endx, Coord::units endy, bool filled = true)
            : Ellipse({ startx, starty }, { endx, endy }, filled)
        {
        }

        constexpr Ellipse(Coord start, Coord end, bool filled = true)
            : c1(start)
            , c2(end)
            , filled(filled)
        {
        }
        friend auto operator==(Ellipse const&, Ellipse const&) -> bool = default;
    };
    inline constexpr auto operator+(Ellipse lhs, Coord rhs) { return Ellipse{ lhs.c1 + rhs, lhs.c2 + rhs, lhs.filled }; }
    inline constexpr auto operator+(Coord lhs, Ellipse rhs) { return Ellipse{ lhs + rhs.c1, lhs + rhs.c2, rhs.filled }; }
    inline constexpr auto operator-(Ellipse lhs, Coord rhs) { return Ellipse{ lhs.c1 - rhs, lhs.c2 - rhs, lhs.filled }; }
    inline constexpr auto operator-(Coord lhs, Ellipse rhs) { return Ellipse{ lhs - rhs.c1, lhs - rhs.c2, rhs.filled }; }

    struct Circle {
        Coord c1{};
        Coord::units radius{};
        bool filled = true;
        std::optional<Coord> size;

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

        constexpr Circle(Coord size, Coord start, Coord::units radius, bool filled = true)
            : c1(start)
            , radius(radius)
            , filled(filled)
            , size(size)
        {
        }
        friend auto operator==(Circle const&, Circle const&) -> bool = default;
    };
    inline constexpr auto operator+(Circle lhs, Coord rhs) { return Circle{ lhs.c1 + rhs, lhs.radius, lhs.filled }; }
    inline constexpr auto operator+(Coord lhs, Circle rhs) { return Circle{ lhs + rhs.c1, rhs.radius, rhs.filled }; }
    inline constexpr auto operator-(Circle lhs, Coord rhs) { return Circle{ lhs.c1 - rhs, lhs.radius, lhs.filled }; }
    inline constexpr auto operator-(Coord lhs, Circle rhs) { return Circle{ lhs - rhs.c1, rhs.radius, rhs.filled }; }

    struct Rectangle {
        Coord start{};
        Coord size{};
        Coord::units rounding{};
        bool filled = true;

        constexpr Rectangle(Coord::units startx, Coord::units starty, Coord::units endx, Coord::units endy, bool filled = true)
            : Rectangle({ startx, starty }, { static_cast<Coord::units>(endx - startx), static_cast<Coord::units>(endy - starty) }, 0, filled)
        {
        }

        constexpr Rectangle(Coord start, Coord size, bool filled = true)
            : Rectangle(start, size, 0, filled)
        {
        }
        constexpr Rectangle(Coord::units startx, Coord::units starty, Coord::units endx, Coord::units endy, Coord::units rounding, bool filled = true)
            : Rectangle({ startx, starty }, { static_cast<Coord::units>(endx - startx), static_cast<Coord::units>(endy - starty) }, rounding, filled)
        {
        }

        constexpr Rectangle(Coord start, Coord size, Coord::units rounding, bool filled = true)
            : start(start)
            , size(size)
            , rounding(rounding)
            , filled(filled)
        {
        }
        friend auto operator==(Rectangle const&, Rectangle const&) -> bool = default;
    };
    inline constexpr auto operator+(Rectangle lhs, Coord rhs) { return Rectangle{ lhs.start + rhs, lhs.size, lhs.rounding, lhs.filled }; }
    inline constexpr auto operator+(Coord lhs, Rectangle rhs) { return Rectangle{ lhs + rhs.start, rhs.size, rhs.rounding, rhs.filled }; }
    inline constexpr auto operator-(Rectangle lhs, Coord rhs) { return Rectangle{ lhs.start - rhs, lhs.size, lhs.rounding, lhs.filled }; }
    inline constexpr auto operator-(Coord lhs, Rectangle rhs) { return Rectangle{ lhs - rhs.start, rhs.size, rhs.rounding, rhs.filled }; }

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
        double linePad{};

        Text(Coord::units startx, Coord::units starty, std::string text = "", TextAnchor anchor = TextAnchor::None, bool withBackground = false, double linePad = 0.0)
            : Text(Coord{ startx, starty }, std::move(text), anchor, withBackground, linePad)
        {
        }

        explicit Text(Coord start, std::string text = "", TextAnchor anchor = TextAnchor::None, bool withBackground = false, double linePad = 0.0)
            : c1(start)
            , text(std::move(text))
            , anchor(anchor)
            , withBackground(withBackground)
            , linePad(linePad)
        {
        }

        explicit Text(Coord start, std::string text, double linePad)
            : Text(start, std::move(text), TextAnchor::None, false, linePad)
        {
        }

        Text(Coord::units startx, Coord::units starty, std::string text = "", TextAnchor anchor = TextAnchor::None, bool withBackground = false)
            : Text(Coord{ startx, starty }, std::move(text), anchor, withBackground)
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
    inline auto operator+(Text const& lhs, Coord rhs) { return Text{ lhs.c1 + rhs, lhs.text, lhs.anchor, lhs.withBackground, lhs.linePad }; }
    inline auto operator+(Coord lhs, Text const& rhs) { return Text{ lhs + rhs.c1, rhs.text, rhs.anchor, rhs.withBackground, rhs.linePad }; }
    inline auto operator-(Text const& lhs, Coord rhs) { return Text{ lhs.c1 - rhs, lhs.text, lhs.anchor, lhs.withBackground, lhs.linePad }; }
    inline auto operator-(Coord lhs, Text const& rhs) { return Text{ lhs - rhs.c1, rhs.text, rhs.anchor, rhs.withBackground, rhs.linePad }; }

    struct Image {
        Coord mStart{};
        std::shared_ptr<ImageData> mImage{};
        bool mGreyscale{};

        Image(Coord::units startx, Coord::units starty, std::shared_ptr<ImageData> image, bool greyscale = false)
            : Image{ { startx, starty }, std::move(image), greyscale }
        {
        }

        Image(Coord start, std::shared_ptr<ImageData> image, bool greyscale = false)
            : mStart{ start }
            , mImage{ std::move(image) }
            , mGreyscale{ greyscale }
        {
        }
        friend auto operator==(Image const&, Image const&) -> bool = default;
    };
    inline auto operator+(Image lhs, Coord rhs) { return Image{ lhs.mStart + rhs, lhs.mImage, lhs.mGreyscale }; }
    inline auto operator+(Coord lhs, Image rhs) { return Image{ lhs + rhs.mStart, rhs.mImage, rhs.mGreyscale }; }
    inline auto operator-(Image lhs, Coord rhs) { return Image{ lhs.mStart - rhs, lhs.mImage, lhs.mGreyscale }; }
    inline auto operator-(Coord lhs, Image rhs) { return Image{ lhs - rhs.mStart, rhs.mImage, rhs.mGreyscale }; }

    struct OverrideFont {
        Font font;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideFont const& lhs, Coord rhs) -> OverrideFont;
    inline auto operator+(Coord lhs, OverrideFont const& rhs) -> OverrideFont;
    inline auto operator-(OverrideFont const& lhs, Coord rhs) -> OverrideFont;
    inline auto operator-(Coord lhs, OverrideFont const& rhs) -> OverrideFont;
    inline auto operator==(OverrideFont const& lhs, OverrideFont const& rhs) -> bool;

    struct OverrideTextForeground {
        BrushAndPen brushAndPen;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideTextForeground const& lhs, Coord rhs) -> OverrideTextForeground;
    inline auto operator+(Coord lhs, OverrideTextForeground const& rhs) -> OverrideTextForeground;
    inline auto operator-(OverrideTextForeground const& lhs, Coord rhs) -> OverrideTextForeground;
    inline auto operator-(Coord lhs, OverrideTextForeground const& rhs) -> OverrideTextForeground;
    inline auto operator==(OverrideTextForeground const& lhs, OverrideTextForeground const& rhs) -> bool;

    struct OverrideBrush {
        Brush brush;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideBrush const& lhs, Coord rhs) -> OverrideBrush;
    inline auto operator+(Coord lhs, OverrideBrush const& rhs) -> OverrideBrush;
    inline auto operator-(OverrideBrush const& lhs, Coord rhs) -> OverrideBrush;
    inline auto operator-(Coord lhs, OverrideBrush const& rhs) -> OverrideBrush;
    inline auto operator==(OverrideBrush const& lhs, OverrideBrush const& rhs) -> bool;

    struct OverridePen {
        Pen pen;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverridePen const& lhs, Coord rhs) -> OverridePen;
    inline auto operator+(Coord lhs, OverridePen const& rhs) -> OverridePen;
    inline auto operator-(OverridePen const& lhs, Coord rhs) -> OverridePen;
    inline auto operator-(Coord lhs, OverridePen const& rhs) -> OverridePen;
    inline auto operator==(OverridePen const& lhs, OverridePen const& rhs) -> bool;

    struct OverrideBrushAndPen {
        BrushAndPen brushAndPen;
        std::vector<DrawCommand> commands{};
    };
    inline auto operator+(OverrideBrushAndPen const& lhs, Coord rhs) -> OverrideBrushAndPen;
    inline auto operator+(Coord lhs, OverrideBrushAndPen const& rhs) -> OverrideBrushAndPen;
    inline auto operator-(OverrideBrushAndPen const& lhs, Coord rhs) -> OverrideBrushAndPen;
    inline auto operator-(Coord lhs, OverrideBrushAndPen const& rhs) -> OverrideBrushAndPen;
    inline auto operator==(OverrideBrushAndPen const& lhs, OverrideBrushAndPen const& rhs) -> bool;

    enum class StackAlign {
        Begin,
        End,
        Uniform,
        Justified,
    };
    struct VStack {
        std::vector<DrawCommand> commands{};
        StackAlign align = StackAlign::Begin;
    };
    inline auto operator+(VStack const& lhs, Coord rhs) -> VStack;
    inline auto operator+(Coord lhs, VStack const& rhs) -> VStack;
    inline auto operator-(VStack const& lhs, Coord rhs) -> VStack;
    inline auto operator-(Coord lhs, VStack const& rhs) -> VStack;
    inline auto operator==(VStack const& lhs, VStack const& rhs) -> bool;

    struct HStack {
        std::vector<DrawCommand> commands{};
        StackAlign align = StackAlign::Begin;
    };
    inline auto operator+(HStack const& lhs, Coord rhs) -> HStack;
    inline auto operator+(Coord lhs, HStack const& rhs) -> HStack;
    inline auto operator-(HStack const& lhs, Coord rhs) -> HStack;
    inline auto operator-(Coord lhs, HStack const& rhs) -> HStack;
    inline auto operator==(HStack const& lhs, HStack const& rhs) -> bool;

    struct ZStack {
        std::vector<DrawCommand> commands{};
        StackAlign align = StackAlign::Begin;
    };
    inline auto operator+(ZStack const& lhs, Coord rhs) -> ZStack;
    inline auto operator+(Coord lhs, ZStack const& rhs) -> ZStack;
    inline auto operator-(ZStack const& lhs, Coord rhs) -> ZStack;
    inline auto operator-(Coord lhs, ZStack const& rhs) -> ZStack;
    inline auto operator==(ZStack const& lhs, ZStack const& rhs) -> bool;

    inline auto operator==(OverrideFont const& lhs, OverrideFont const& rhs) -> bool
    {
        return lhs.font == rhs.font
            && lhs.commands == rhs.commands;
    }
    inline auto operator==(OverrideTextForeground const& lhs, OverrideTextForeground const& rhs) -> bool
    {
        return lhs.brushAndPen == rhs.brushAndPen
            && lhs.commands == rhs.commands;
    }
    inline auto operator==(OverrideBrush const& lhs, OverrideBrush const& rhs) -> bool
    {
        return lhs.brush == rhs.brush
            && lhs.commands == rhs.commands;
    }
    inline auto operator==(OverridePen const& lhs, OverridePen const& rhs) -> bool
    {
        return lhs.pen == rhs.pen
            && lhs.commands == rhs.commands;
    }
    inline auto operator==(OverrideBrushAndPen const& lhs, OverrideBrushAndPen const& rhs) -> bool
    {
        return lhs.brushAndPen == rhs.brushAndPen
            && lhs.commands == rhs.commands;
    }

    inline auto operator==(VStack const& lhs, VStack const& rhs) -> bool
    {
        return lhs.commands == rhs.commands
            && lhs.align == rhs.align;
    }
    inline auto operator==(HStack const& lhs, HStack const& rhs) -> bool
    {
        return lhs.commands == rhs.commands
            && lhs.align == rhs.align;
    }
    inline auto operator==(ZStack const& lhs, ZStack const& rhs) -> bool
    {
        return lhs.commands == rhs.commands
            && lhs.align == rhs.align;
    }

#define IMPLEMENT_ADD_SUB_OPERATORS(WHICH)                                    \
    inline auto operator+(WHICH const& lhs, Coord rhs) -> WHICH               \
    {                                                                         \
        return std::visit(overloaded{                                         \
                              [rhs](auto arg) { return WHICH{ arg + rhs }; }, \
                          },                                                  \
            lhs);                                                             \
    }                                                                         \
    inline auto operator+(Coord lhs, WHICH const& rhs) -> WHICH               \
    {                                                                         \
        return std::visit(overloaded{                                         \
                              [lhs](auto arg) { return WHICH{ lhs + arg }; }, \
                          },                                                  \
            rhs);                                                             \
    }                                                                         \
    inline auto operator-(WHICH const& lhs, Coord rhs) -> WHICH               \
    {                                                                         \
        return std::visit(overloaded{                                         \
                              [rhs](auto arg) { return WHICH{ arg - rhs }; }, \
                          },                                                  \
            lhs);                                                             \
    }                                                                         \
    inline auto operator-(Coord lhs, WHICH const& rhs) -> WHICH               \
    {                                                                         \
        return std::visit(overloaded{                                         \
                              [lhs](auto arg) { return WHICH{ lhs - arg }; }, \
                          },                                                  \
            rhs);                                                             \
    }

    IMPLEMENT_ADD_SUB_OPERATORS(DrawItems);
    IMPLEMENT_ADD_SUB_OPERATORS(DrawManipulators);
    IMPLEMENT_ADD_SUB_OPERATORS(DrawStack);
    IMPLEMENT_ADD_SUB_OPERATORS(DrawCommand);

#undef IMPLEMENT_ADD_SUB_OPERATORS

    // Type acts as a tag to find the correct operator+ overload
    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    auto operator+(Range&& lhs, Coord rhs)
    {
        return toDrawCommands(std::views::transform(lhs, [rhs](auto const& arg) { return arg + rhs; }));
    }

    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    auto operator+(Coord lhs, Range&& rhs)
    {
        return toDrawCommands(std::views::transform(rhs, [lhs](auto const& arg) { return lhs + arg; }));
    }

    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    auto operator-(Range&& lhs, Coord rhs)
    {
        return toDrawCommands(std::views::transform(lhs, [rhs](auto const& arg) { return arg - rhs; }));
    }
    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto operator-(Coord lhs, Range&& rhs)
    {
        return toDrawCommands(std::views::transform(rhs, [lhs](auto const& arg) { return lhs - arg; }));
    }

    // Type acts as a tag to find the correct operator+ overload
    struct to_add_helper {
        Coord value;
    };
    template <std::ranges::range R>
        requires std::convertible_to<std::ranges::range_value_t<R>, DrawCommand>
    auto operator+(R&& r, to_add_helper value)
    {
        return toDrawCommands(std::views::transform(r, [value](auto const& arg) { return arg + value.value; }));
    }

    // there must be a better way to do this?
    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto toDrawCommands(Range&& range) -> std::vector<DrawCommand>
    {
        auto result = std::vector<DrawCommand>{};
        std::ranges::copy(range, std::back_inserter(result));
        return result;
    }

    inline auto operator+(OverrideFont const& lhs, Coord rhs) -> OverrideFont { return OverrideFont{ lhs.font, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideFont const& rhs) -> OverrideFont { return OverrideFont{ rhs.font, lhs + rhs.commands }; }
    inline auto operator-(OverrideFont const& lhs, Coord rhs) -> OverrideFont { return OverrideFont{ lhs.font, lhs.commands - rhs }; }
    inline auto operator-(Coord lhs, OverrideFont const& rhs) -> OverrideFont { return OverrideFont{ rhs.font, lhs - rhs.commands }; }
    inline auto operator+(OverrideBrush const& lhs, Coord rhs) -> OverrideBrush { return OverrideBrush{ lhs.brush, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideBrush const& rhs) -> OverrideBrush { return OverrideBrush{ rhs.brush, lhs + rhs.commands }; }
    inline auto operator-(OverrideBrush const& lhs, Coord rhs) -> OverrideBrush { return OverrideBrush{ lhs.brush, lhs.commands - rhs }; }
    inline auto operator-(Coord lhs, OverrideBrush const& rhs) -> OverrideBrush { return OverrideBrush{ rhs.brush, lhs - rhs.commands }; }
    inline auto operator+(OverridePen const& lhs, Coord rhs) -> OverridePen { return OverridePen{ lhs.pen, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverridePen const& rhs) -> OverridePen { return OverridePen{ rhs.pen, lhs + rhs.commands }; }
    inline auto operator-(OverridePen const& lhs, Coord rhs) -> OverridePen { return OverridePen{ lhs.pen, lhs.commands - rhs }; }
    inline auto operator-(Coord lhs, OverridePen const& rhs) -> OverridePen { return OverridePen{ rhs.pen, lhs - rhs.commands }; }
    inline auto operator+(OverrideBrushAndPen const& lhs, Coord rhs) -> OverrideBrushAndPen { return OverrideBrushAndPen{ lhs.brushAndPen, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideBrushAndPen const& rhs) -> OverrideBrushAndPen { return OverrideBrushAndPen{ rhs.brushAndPen, lhs + rhs.commands }; }
    inline auto operator-(OverrideBrushAndPen const& lhs, Coord rhs) -> OverrideBrushAndPen { return OverrideBrushAndPen{ lhs.brushAndPen, lhs.commands - rhs }; }
    inline auto operator-(Coord lhs, OverrideBrushAndPen const& rhs) -> OverrideBrushAndPen { return OverrideBrushAndPen{ rhs.brushAndPen, lhs - rhs.commands }; }
    inline auto operator+(OverrideTextForeground const& lhs, Coord rhs) -> OverrideTextForeground { return OverrideTextForeground{ lhs.brushAndPen, lhs.commands + rhs }; }
    inline auto operator+(Coord lhs, OverrideTextForeground const& rhs) -> OverrideTextForeground { return OverrideTextForeground{ rhs.brushAndPen, lhs + rhs.commands }; }
    inline auto operator-(OverrideTextForeground const& lhs, Coord rhs) -> OverrideTextForeground { return OverrideTextForeground{ lhs.brushAndPen, lhs.commands - rhs }; }
    inline auto operator-(Coord lhs, OverrideTextForeground const& rhs) -> OverrideTextForeground { return OverrideTextForeground{ rhs.brushAndPen, lhs - rhs.commands }; }

    // how do we create draw hierarchies.  With DrawCommand.
    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto withFont(Font font, Range&& commands) -> DrawCommand
    {
        return OverrideFont{ font, toDrawCommands(commands) };
    }
    inline auto withFont(Font font, DrawCommand command) { return withFont(font, std::vector<DrawCommand>{ std::move(command) }); }

    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto withBrush(Brush brush, Range&& commands) -> DrawCommand
    {
        return OverrideBrush{ brush, toDrawCommands(commands) };
    }
    inline auto withBrush(Brush brush, DrawCommand command) { return withBrush(brush, std::vector<DrawCommand>{ std::move(command) }); }

    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto withPen(Pen pen, Range&& commands) -> DrawCommand
    {
        return OverridePen{ pen, toDrawCommands(commands) };
    }
    inline auto withPen(Pen pen, DrawCommand command) { return withPen(pen, std::vector<DrawCommand>{ std::move(command) }); }

    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto withBrushAndPen(BrushAndPen brushAndPen, Range&& commands) -> DrawCommand
    {
        return OverrideBrushAndPen{ brushAndPen, toDrawCommands(commands) };
    }
    inline auto withBrushAndPen(BrushAndPen brushAndPen, DrawCommand command) { return withBrushAndPen(brushAndPen, std::vector<DrawCommand>{ std::move(command) }); }

    template <std::ranges::range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, DrawCommand>
    inline auto withTextForeground(BrushAndPen brushAndPen, Range&& commands) -> DrawCommand
    {
        return OverrideTextForeground{ brushAndPen, toDrawCommands(commands) };
    }
    inline auto withTextForeground(BrushAndPen brushAndPen, DrawCommand command) { return withTextForeground(brushAndPen, std::vector<DrawCommand>{ std::move(command) }); }

    inline auto operator+(VStack const& lhs, Coord rhs) -> VStack
    {
        return VStack{ toDrawCommands(lhs.commands + rhs) };
    }
    inline auto operator+(Coord lhs, VStack const& rhs) -> VStack
    {
        return VStack{ toDrawCommands(lhs + rhs.commands) };
    }
    inline auto operator-(VStack const& lhs, Coord rhs) -> VStack
    {
        return VStack{ toDrawCommands(lhs.commands - rhs) };
    }
    inline auto operator-(Coord lhs, VStack const& rhs) -> VStack
    {
        return VStack{ toDrawCommands(lhs - rhs.commands) };
    }
    inline auto operator+(HStack const& lhs, Coord rhs) -> HStack
    {
        return HStack{ toDrawCommands(lhs.commands + rhs), lhs.align };
    }
    inline auto operator+(Coord lhs, HStack const& rhs) -> HStack
    {
        return HStack{ toDrawCommands(lhs + rhs.commands), rhs.align };
    }
    inline auto operator-(HStack const& lhs, Coord rhs) -> HStack
    {
        return HStack{ toDrawCommands(lhs.commands - rhs), lhs.align };
    }
    inline auto operator-(Coord lhs, HStack const& rhs) -> HStack
    {
        return HStack{ toDrawCommands(lhs - rhs.commands), rhs.align };
    }
    inline auto operator+(ZStack const& lhs, Coord rhs) -> ZStack
    {
        return ZStack{ toDrawCommands(lhs.commands + rhs) };
    }
    inline auto operator+(Coord lhs, ZStack const& rhs) -> ZStack
    {
        return ZStack{ toDrawCommands(lhs + rhs.commands) };
    }
    inline auto operator-(ZStack const& lhs, Coord rhs) -> ZStack
    {
        return ZStack{ toDrawCommands(lhs.commands - rhs) };
    }
    inline auto operator-(Coord lhs, ZStack const& rhs) -> ZStack
    {
        return ZStack{ toDrawCommands(lhs - rhs.commands) };
    }
}

namespace Draw {

    // What follows are a collection of functions to create different common "images" for CalChart.
    namespace Field {
        // Construct commands for the field outline
        auto CreateOutline(CalChart::Coord const& fieldsize) -> std::vector<DrawCommand>;

        // Construct commands for the vertical solid line down from the top at 8 step spacing
        auto CreateVerticalSolidLine(CalChart::Coord const& fieldsize, int step1) -> std::vector<DrawCommand>;

        // Construct commands for the vertical dotted lines in between the Solid lines
        auto CreateVerticalDottedLine(CalChart::Coord const& fieldsize, int step1) -> std::vector<DrawCommand>;

        // Construct commands for the horizontal dotted lines
        auto CreateHorizontalDottedLine(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<DrawCommand>;

        // Draw the hashes
        auto CreateHashes(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<DrawCommand>;
        auto CreateHashTicks(CalChart::Coord const& fieldsize, int mode_HashW, int mode_HashE, int step1) -> std::vector<DrawCommand>;

        auto CreateYardlineLabels(std::vector<std::string> const& yard_text, CalChart::Coord const& fieldsize, int offset, int step1) -> std::vector<DrawCommand>;
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
