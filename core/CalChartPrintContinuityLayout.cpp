/*
 * CalChartPrintContinuityLayout.cpp
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

#include "CalChartPrintContinuityLayout.h"
#include "CalChartCoroutine.h"
#include "CalChartPoint.h"
#include "CalChartRanges.h"
#include "CalChartUtils.h"

#include <algorithm>
#include <coroutine>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

namespace CalChart::PrintContinuityLayout {

auto operator<<(std::ostream& os, FontType fontType) -> std::ostream&
{
    switch (fontType) {
    case FontType::Plain:
        return os << "Plain";
    case FontType::Bold:
        return os << "Bold";
    case FontType::Italics:
        return os << "Italics";
    case FontType::BoldItalics:
        return os << "BoldItalics";
    }
    return os << "BoldItalics";
}

auto operator<<(std::ostream& os, Item item) -> std::ostream&
{
    return std::visit(CalChart::overloaded{
                          [&os](Symbol) -> std::ostream& {
                              return os << "Symbol";
                          },
                          [&os](Tab) -> std::ostream& {
                              return os << "Tab";
                          },
                          [&os](TextItem text) -> std::ostream& {
                              return os << "font:" << text.fontType << " " << text.string;
                          },
                      },
        item.data);
}

namespace {
    static inline auto& whichFont(FontType fontType, Context const& context)
    {
        switch (fontType) {
        case FontType::Plain:
            return context.plain;
        case FontType::Bold:
            return context.bold;
        case FontType::Italics:
            return context.italics;
        case FontType::BoldItalics:
            return context.bolditalics;
        }
        return context.bolditalics;
    }

    inline auto withBold(FontType fontType, bool bold)
    {
        if (bold) {
            if (fontType == FontType::Plain) {
                return FontType::Bold;
            }
            if (fontType == FontType::Italics) {
                return FontType::BoldItalics;
            }
        } else {
            if (fontType == FontType::Bold) {
                return FontType::Plain;
            }
            if (fontType == FontType::BoldItalics) {
                return FontType::Italics;
            }
        }
        return fontType;
    }

    auto withItalic(FontType fontType, bool italic)
    {
        if (italic) {
            if (fontType == FontType::Plain) {
                return FontType::Italics;
            }
            if (fontType == FontType::Bold) {
                return FontType::BoldItalics;
            }
        } else {
            if (fontType == FontType::Italics) {
                return FontType::Plain;
            }
            if (fontType == FontType::BoldItalics) {
                return FontType::Bold;
            }
        }
        return fontType;
    }

    auto splitStringView(std::string_view input, std::string_view delimiters) -> std::vector<std::string_view>
    {
        std::vector<std::string_view> substrings;

        size_t startPos = 0;
        size_t newPos = input.find_first_of(delimiters);

        while (newPos != std::string_view::npos) {
            substrings.push_back(input.substr(startPos, newPos - startPos));
            startPos = newPos + 1; // Move startPos after the delimiter
            newPos = input.find_first_of(delimiters, startPos);
        }

        if (startPos <= input.length()) {
            substrings.push_back(input.substr(startPos));
        }

        return substrings;
    }

    // Using optionals here make it easy to say that it's not a Person
    auto isPersonMark(std::string_view line) -> std::optional<Symbol>
    {
        if (line.size() < 3) {
            return {};
        }
        if ((tolower(line.at(1)) != 'p') && (tolower(line.at(1)) != 's')) {
            return {};
        }
        if (tolower(line.at(1)) == 'p') {
            switch (tolower(line.at(2))) {
            case 'o':
                return Symbol::Plain;
            case 'b':
                return Symbol::Backslash;
            case 's':
                return Symbol::Slash;
            case 'x':
                return Symbol::Cross;
            default:
                // code not recognized
                throw std::runtime_error("Print continuity not recognized");
            }
        }
        switch (tolower(line.at(2))) {
        case 'o':
            return Symbol::Solid;
        case 'b':
            return Symbol::SolidBackslash;
        case 's':
            return Symbol::SolidSlash;
        case 'x':
            return Symbol::SolidCross;
        default:
            // code not recognized
            throw std::runtime_error("Print continuity not recognized");
        }
    }

    // Using optionals here make it easy to say that it's not a font change.
    auto isBoldItalics(std::string_view line, FontType fontType) -> std::optional<FontType>
    {
        if (line.size() < 3) {
            return {};
        }
        if ((tolower(line.at(1)) != 'b') && (tolower(line.at(1)) != 'i')) {
            return {};
        }
        if (tolower(line.at(1)) == 'b') {
            switch (tolower(line.at(2))) {
            case 's':
                return withBold(fontType, true);
            case 'e':
                return withBold(fontType, false);
            default:
                // code not recognized
                throw std::runtime_error("Print continuity not recognized");
            }
        }
        switch (tolower(line.at(2))) {
        case 's':
            return withItalic(fontType, true);
        case 'e':
            return withItalic(fontType, false);
        default:
            // code not recognized
            throw std::runtime_error("Print continuity not recognized");
        }
    }

    // Given a string, parse into items.  Using Coroutines makes this easier to understand --
    // each time an item is found (ie, we reach a new deliminator), co_yield it.  State is
    // easy to maintain because they are just stack variables.  Nice!
    auto GenerateItems(std::string_view input) -> Coroutine::Generator<Item>
    {
        using std::operator""sv;
        auto delimiters = "\\\t"sv;

        auto fontType = FontType::Plain;

        size_t newPos = input.find_first_of(delimiters);
        while (newPos != std::string_view::npos) {
            // found something, return the previous item.
            co_yield Item{ TextItem{ std::string{ input.substr(0, newPos) }, fontType } };
            input = input.substr(newPos);

            size_t scanStart = 0;

            // examine what's at the newPos
            if (input.at(0) == '\t') {
                co_yield Item{ Tab{} };
                input = input.substr(1);
            }
            if (!input.empty() && input.at(0) == '\\') {
                auto symbol = isPersonMark(input);
                auto boldItalics = isBoldItalics(input, fontType);
                if (symbol) {
                    co_yield Item{ *symbol };
                    input = input.substr(3);
                } else if (boldItalics) {
                    fontType = *boldItalics;
                    input = input.substr(3);
                } else {
                    // we skip the first if it is an escaped character.
                    scanStart = 1;
                }
            }

            newPos = input.find_first_of(delimiters, scanStart);
        }

        co_yield Item{ TextItem{ std::string{ input }, fontType } };
    }

    auto ParseHStack(std::string_view input) -> HStack
    {
        auto onSheet = true;
        auto onMain = true;
        auto centered = false;
        // peel off the '<>~'
        if (!input.empty() && input.at(0) == '<') {
            onSheet = false;
            input = input.substr(1);
        }
        if (!input.empty() && input.at(0) == '>') {
            onMain = false;
            input = input.substr(1);
        }
        if (!input.empty() && input.at(0) == '~') {
            centered = true;
            input = input.substr(1);
        }
        auto items = Ranges::ToVector<Item>(GenerateItems(input));
        return { items, centered, onMain, onSheet };
    }

    auto stripNewline(std::string_view input) -> std::string_view
    {
        size_t pos = input.find_last_of('\n');
        if (pos != std::string_view::npos && pos == input.size() - 1) {
            return input.substr(0, pos);
        }
        return input;
    }
}

auto Parse(std::string_view input) -> VStack
{
    // strip off any trailing newlines
    return { Ranges::ToVector<HStack>(splitStringView(stripNewline(input), "\n")
        | std::views::transform(ParseHStack)) };
}

namespace {
    inline auto toSYMBOL_TYPE(Symbol symbol) -> SYMBOL_TYPE
    {
        switch (symbol) {
        case Symbol::Plain:
            return SYMBOL_PLAIN;
        case Symbol::Backslash:
            return SYMBOL_BKSL;
        case Symbol::Slash:
            return SYMBOL_SL;
        case Symbol::Cross:
            return SYMBOL_X;
        case Symbol::Solid:
            return SYMBOL_SOL;
        case Symbol::SolidBackslash:
            return SYMBOL_SOLBKSL;
        case Symbol::SolidSlash:
            return SYMBOL_SOLSL;
        case Symbol::SolidCross:
            return SYMBOL_SOLX;
        }
        return SYMBOL_SOLX;
    }

    auto DrawContSymbol(double size, double pLineRatio, double sLineRatio, CalChart::SYMBOL_TYPE symbol) -> CalChart::Draw::ZStack
    {
        auto const offset = CalChart::Coord{ static_cast<CalChart::Coord::units>(size / 2.0), static_cast<CalChart::Coord::units>(size / 2.0) };
        auto const dotRatio = CoordUnits2Float(size);
        return { CalChart::Point(offset, symbol).GetDrawCommands(dotRatio, pLineRatio, sLineRatio) };
    }

    // given a tuple of item, coord, generate the draw commands
    auto CreateDrawCommand(Item const& item, Context const& context) -> Draw::DrawCommand
    {
        return std::visit(
            CalChart::overloaded{
                [context](Symbol i) -> Draw::DrawCommand {
                    return DrawContSymbol(context.symbolSize, context.pLineRatio, context.sLineRatio, toSYMBOL_TYPE(i)) + CalChart::Coord{ 0, context.symbolMiddle.y };
                },
                [context]([[maybe_unused]] Tab i) -> Draw::DrawCommand {
                    return Draw::Tab{
                        context.landscape ? [](int i) { 
                            if (i < 10) return 10;
                            if (i < 28) return 28;
                            return (((i - 28) / 8) + 1) * 8 + 28; }
                                          : [](int i) { 
                            if (i < 6) return 6;
                            if (i < 20) return 20;
                            if (i < 28) return 28;
                            return (((i - 28) / 6) + 1) * 6 + 28; }
                    };
                },
                [context](TextItem const& i) -> Draw::DrawCommand {
                    return Draw::Text({}, i.string, context.linePad);
                },
            },
            item.data);
    }

    // We want to minimize the number of font changes needed, so what we do is we first split the line
    // at the points where there are font changes.
    // Then with each group, we create a FontOverride object.
    template <typename Iterator>
    auto GenerateDrawCommands(Iterator begin, Iterator end, CalChart::Draw::DrawCommand cmd, Context const& context) -> std::vector<CalChart::Draw::DrawCommand>
    {
        auto result = CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
            std::ranges::subrange(begin, end) | std::views::transform([context](auto&& item) {
                return CreateDrawCommand(item, context);
            }));
        result.push_back(cmd);
        return result;
    }

    template <typename Iterator>
    auto GenerateOverrideFont(Iterator begin, Iterator end, CalChart::Draw::DrawCommand cmd, Context const& context, FontType fontType) -> CalChart::Draw::DrawCommand
    {
        auto result = CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
            std::ranges::subrange(begin, end) | std::views::transform([context](auto&& item) {
                return CreateDrawCommand(item, context);
            }));
        result.push_back(cmd);
        return Draw::OverrideFont{
            whichFont(fontType, context),
            { Draw::HStack{ result } }
        };
    }

    template <typename Iterator>
    auto GenerateOverrideFont(Iterator begin, Iterator end, Context const& context, FontType fontType) -> CalChart::Draw::DrawCommand
    {
        return Draw::OverrideFont{
            whichFont(fontType, context),
            { Draw::HStack{
                CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
                    std::ranges::subrange(begin, end) | std::views::transform([context](auto&& item) {
                        return CreateDrawCommand(item, context);
                    })) } }
        };
    }

    auto GetWhichFont(Item const& item, FontType fontType) -> FontType
    {
        return std::visit(CalChart::overloaded{
                              [](TextItem const& item) {
                                  return item.fontType;
                              },
                              [fontType](auto&&) {
                                  return fontType;
                              },
                          },
            item.data);
    }

    // how we split an array into
    template <typename Iterator>
    auto SplitUpSequence(Iterator begin, Iterator end, FontType currentFont) -> std::vector<std::pair<std::pair<Iterator, Iterator>, FontType>>
    {
        auto currentBegin = begin;
        auto results = std::vector<std::pair<std::pair<Iterator, Iterator>, FontType>>{};
        while (begin != end) {
            auto nextFont = GetWhichFont(*begin, currentFont);
            if (nextFont != currentFont) {
                results.push_back({ std::pair<Iterator, Iterator>{ currentBegin, begin }, currentFont });
                currentBegin = begin;
                currentFont = nextFont;
            }
            ++begin;
        }
        results.push_back({ { currentBegin, end }, currentFont });
        return results;
    }

    // Given an HStack, create the draw commands
    auto ToHDrawCommands(HStack const& input, Context const& context) -> CalChart::Draw::DrawCommand
    {
        // we first split the sequence up by groups that have the same font, starting with Plain
        auto chunks = SplitUpSequence(input.items.begin(), input.items.end(), FontType::Plain);
        if (chunks.empty()) {
            return {};
        }
        // if we have at least one chunk, then that will be in plain.  So for everything but the first, reverse and create text blocks.
        auto rchunks = chunks | std::views::drop(1) | std::views::reverse;

        auto lastone = std::accumulate(rchunks.begin(), rchunks.end(), CalChart::Draw::DrawCommand{ CalChart::Draw::Ignore{} }, [context](auto&& acc, auto&& chunk) {
            auto [range, fontType] = chunk;
            return GenerateOverrideFont(std::get<0>(range), std::get<1>(range), acc, context, fontType);
        });
        auto [range, fontType] = *(chunks.begin());
        return CalChart::Draw::HStack{
            GenerateDrawCommands(std::get<0>(range), std::get<1>(range), lastone, context),
            Draw::StackAlign::Begin
        };
    }

    // If the input is centered, wrap the commands in a HStack centering command.
    auto ToHDrawCommandFirstCenter(HStack const& input, Context const& context) -> CalChart::Draw::DrawCommand
    {
        if (input.center) {
            return CalChart::Draw::HStack{
                { ToHDrawCommands(input, context) },
                Draw::StackAlign::Justified
            };
        }
        return ToHDrawCommands(input, context);
    }
}

auto ToDrawCommand(VStack const& input, Context const& context) -> CalChart::Draw::DrawCommand
{
    return CalChart::Draw::VStack{
        CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
            input.lines
            | std::views::transform([context](auto&& hstack) {
                  return ToHDrawCommandFirstCenter(hstack, context);
              }))
    };
}
}