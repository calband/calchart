#pragma once
/*
 * CalChartPrintContinuityLayout.h
 * Details about how the print continuity should be laid out.
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

#include "CalChartCoroutine.h"
#include "CalChartDrawCommand.h"

#include <coroutine>
#include <iostream>
#include <ranges>
#include <string>
#include <variant>
#include <vector>

// Print Continuity in CalChart is manually entered text that explains the continuity for a sheet.
// The rendering pipeline for displaying is:
// ┌──────┐  ┌───────────────────────┐  ┌─┐  ┌────────────┐
// │string│─▶│PrintContinuity::VStack│─▶│+│─▶│DrawCommands│
// └──────┘  └───────────────────────┘  └─┘  └────────────┘
//                                       ▲
//                                       │
//                                ┌─────────────┐
//                                │Draw & Config│
//                                │   Context   │
//                                └─────────────┘
// This allows for easy testing and composition.
//
// Continuity is MarkDown-ish with the following special characters:
/* This is the format text line:
 * normal ascii text possibly containing the following codes:
 * \bs \be \is \ie for bold start, bold end, italics start, italics end
 * \po plainman : A
 * \pb backslashman : C
 * \ps slashman : D
 * \px xman : E
 * \so solidman : B
 * \sb solidbackslashman :F
 * \ss solidslashman : G
 * \sx solidxman : H
 * a line may begin with these symbols in order: <>~
 * < don't print continuity on individual sheets
 * > don't print continuity on master sheet
 * ~ center this line
 * also, there are three tab stops set for standard continuity format
 */
//
// A string of Print Continuity can be converted to a Vertical Stack (VStack) of lines to render.
// A line has flags: Centered, Individual, Master.
// Each Line can be is a Horizontal Stack (HStack) of items.  The items have attributes.  The
// items and attributes are:
// Symbol (enum): Tabbed
// Text: Tabbed, Bolded, Italicized
// Then when we have a VStack it can be converted to CalChart::Draw::DrawCommand that can be rendered.
// We require a context of how things are drawn in order to calculate the DrawCommand layout as
// the size of symbols relies on the size of the font, which is not know till we render.

namespace CalChart::PrintContinuityLayout {

struct VStack;
[[nodiscard]] auto Parse(std::string_view input) -> VStack;

struct Context {
    explicit Context(int fontSize, bool landscape, int linePad, int symbolSize, CalChart::Coord symbolMiddle, double pLineRatio, double sLineRatio)
        : plain{ fontSize, CalChart::Font::Family::Modern }
        , bold{ fontSize, CalChart::Font::Family::Modern, CalChart::Font::Style::Normal, CalChart::Font::Weight::Bold }
        , italics{ fontSize, CalChart::Font::Family::Modern, CalChart::Font::Style::Italic }
        , bolditalics{ fontSize, CalChart::Font::Family::Modern, CalChart::Font::Style::Italic, CalChart::Font::Weight::Bold }
        , landscape{ landscape }
        , linePad{ linePad }
        , symbolSize{ symbolSize }
        , symbolMiddle{ symbolMiddle }
        , pLineRatio{ pLineRatio }
        , sLineRatio{ sLineRatio }
    {
    }

    CalChart::Font plain{};
    CalChart::Font bold{};
    CalChart::Font italics{};
    CalChart::Font bolditalics{};
    bool landscape{};
    int linePad{};
    int symbolSize;
    CalChart::Coord symbolMiddle;
    double pLineRatio;
    double sLineRatio;
};
[[nodiscard]] auto ToDrawCommand(VStack const& input, Context const& context) -> Draw::DrawCommand;

// These are details of the VStack.
enum class FontType {
    Plain,
    Bold,
    Italics,
    BoldItalics,
};

enum class Symbol {
    Plain,
    Backslash,
    Slash,
    Cross,
    Solid,
    SolidBackslash,
    SolidSlash,
    SolidCross,
};

struct Tab {
    friend auto operator==(Tab const&, Tab const&) -> bool = default;
};

struct TextItem {
    std::string string;
    FontType fontType = FontType::Plain;
    friend auto operator==(TextItem const&, TextItem const&) -> bool = default;
};

struct Item {
    std::variant<Symbol, Tab, TextItem> data;
    friend auto operator==(Item const&, Item const&) -> bool = default;
};

struct HStack {
    std::vector<Item> items;
    bool center{};
    bool on_main{ true };
    bool on_sheet{ true };
    friend auto operator==(HStack const&, HStack const&) -> bool = default;
};

struct VStack {
    std::vector<HStack> lines;
};

static inline auto operator==(VStack const& lhs, VStack const& rhs) -> bool
{
    return lhs.lines == rhs.lines;
}
}