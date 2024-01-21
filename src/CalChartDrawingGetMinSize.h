#pragma once
/*
 * CalChartDrawingGetMinSize.h
 * Utilities for GetMinSize
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

#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include <algorithm>

// GetMinSize: Given an array of Draw Command sizes (tabs or actual size), this calculates
// the minimum area needed to place.  This is a necessary step in laying out "Stacks" --
// first calculate how much the minimum size of the commands to draw would be to determine
// how they would be laid out, then lay them out.

namespace wxCalChart::Draw {

// Tabs are effectively ways of specifying that the layout should proceed at the next stop,
// the "ceiling" function of the layout.  In order to support this what we need to do is have
// a way to describe each of the items in a layout as either having a size or a "ceiling" function.
// We do this with a std::varient, either "raw" size, or something that given a size, returns a
// size that effectively is the next layout point.

struct TabStop {
    std::function<wxSize(wxSize)> tabFunc;
};

using StackSize = std::variant<wxSize, TabStop>;

// Context contains the details on how to draw so layout can occur.
struct Context {
    wxDC& dc;
    DrawSurface surface;
};

inline auto GetMinSizes(Context context, std::vector<CalChart::Draw::DrawCommand> const& commands) -> std::vector<StackSize>;

namespace details {
    inline auto VMinSize(std::vector<StackSize> const& sizes) -> wxSize
    {
        using std::max;
        return std::accumulate(sizes.begin(), sizes.end(), wxSize{}, [](auto&& acc, auto&& input) {
            return std::visit(
                CalChart::overloaded{
                    [acc](wxSize size) {
                        return wxSize{ max(acc.x, size.x), acc.y + size.y };
                    },
                    [acc](auto&& tabStop) {
                        return tabStop.tabFunc(acc);
                    },
                },
                input);
        });
    }

    inline auto HMinSize(std::vector<StackSize> const& sizes) -> wxSize
    {
        using std::max;
        return std::accumulate(sizes.begin(), sizes.end(), wxSize{}, [](auto&& acc, auto&& input) {
            return std::visit(
                CalChart::overloaded{
                    [acc](wxSize size) {
                        return wxSize{ acc.x + size.x, max(acc.y, size.y) };
                    },
                    [acc](auto&& tabStop) {
                        return tabStop.tabFunc(acc);
                    },
                },
                input);
        });
    }

    inline auto ZMinSize(std::vector<StackSize> const& sizes) -> wxSize
    {
        using std::max;
        return std::accumulate(sizes.begin(), sizes.end(), wxSize{}, [](auto&& acc, auto&& input) {
            return std::visit(
                CalChart::overloaded{
                    [acc](wxSize size) {
                        return max(acc, size);
                    },
                    [acc](auto&& tabStop) {
                        return tabStop.tabFunc(acc);
                    },
                },
                input);
        });
    }

    inline auto GetMinSize([[maybe_unused]] Context context, [[maybe_unused]] CalChart::Draw::Ignore const& ignore) -> StackSize { return wxSize{}; }

    inline auto GetMinSize(Context context, [[maybe_unused]] CalChart::Draw::Tab const& tab) -> StackSize
    {
        auto spaceSize = context.dc.GetTextExtent(" ").x;
        return TabStop{
            [spaceSize, calc = tab.tabsToStop](wxSize size) {
                auto x = calc(size.x / spaceSize) * spaceSize;
                auto result = wxSize{ x, size.y };
                return result;
            }
        };
    }

    inline auto GetMinSize([[maybe_unused]] Context context, CalChart::Draw::Line const& line) -> StackSize
    {
        using std::max;
        return make_wxSize(max(line.c1, line.c2));
    }

    inline auto GetMinSize([[maybe_unused]] Context context, CalChart::Draw::Arc const& arc) -> StackSize
    {
        using std::max;
        return make_wxSize(max(arc.c1, arc.c2));
    }

    inline auto GetMinSize([[maybe_unused]] Context context, CalChart::Draw::Ellipse const& ellipse) -> StackSize
    {
        using std::max;
        return make_wxSize(max(ellipse.c1, ellipse.c2));
    }

    inline auto GetMinSize([[maybe_unused]] Context context, CalChart::Draw::Circle const& circle) -> StackSize
    {
        return make_wxSize(circle.c1 + CalChart::Coord(circle.radius, circle.radius));
    }

    inline auto GetMinSize([[maybe_unused]] Context context, CalChart::Draw::Rectangle const& rectangle) -> StackSize
    {
        using std::max;
        return make_wxSize(rectangle.size);
    }

    inline auto GetMinSize(Context context, CalChart::Draw::Text const& text) -> StackSize
    {
        int width{};
        int height{};
        context.dc.GetTextExtent(text.text, &width, &height);
        auto metrics = context.dc.GetFontMetrics();
        return wxSize(width, metrics.height + text.linePad);
    }

    inline auto GetMinSize(Context context, CalChart::Draw::DrawManipulators const& cmd) -> StackSize;

    inline auto GetMinSize(Context context, CalChart::Draw::DrawStack const& cmd) -> StackSize;

    inline auto GetMinSize(Context context, CalChart::Draw::DrawCommand const& cmd) -> StackSize;

    inline auto GetMinSize(Context context, CalChart::Draw::DrawItems const& cmd) -> StackSize
    {
        return std::visit([context](auto&& arg) { return GetMinSize(context, arg); }, cmd);
    }

    inline auto GetMinSize(Context context, CalChart::Draw::DrawManipulators const& cmd) -> StackSize
    {
        return std::visit([context](auto&& arg) {
            return ZMinSize(GetMinSizes(context, arg.commands));
        },
            cmd);
    }

    inline auto GetMinSize(Context context, CalChart::Draw::DrawStack const& cmd) -> StackSize
    {
        return std::visit([context](auto&& arg) {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, CalChart::Draw::VStack>) {
                auto minSize = VMinSize(GetMinSizes(context, arg.commands));
                if (arg.align == CalChart::Draw::StackAlign::Begin
                    || arg.align == CalChart::Draw::StackAlign::End) {
                    return minSize;
                }
                minSize.y = context.surface.size.y;
                return minSize;
            } else if constexpr (std::is_same_v<T, CalChart::Draw::HStack>) {
                auto minSize = HMinSize(GetMinSizes(context, arg.commands));
                if (arg.align == CalChart::Draw::StackAlign::Begin
                    || arg.align == CalChart::Draw::StackAlign::End) {
                    return minSize;
                }
                minSize.x = context.surface.size.x;
                return minSize;
            } else if constexpr (std::is_same_v<T, CalChart::Draw::ZStack>) {
                return ZMinSize(GetMinSizes(context, arg.commands));
            }
        },
            cmd);
    }

    inline auto GetMinSize(Context context, CalChart::Draw::DrawCommand const& cmd) -> StackSize
    {
        return std::visit([context](auto&& arg) { return GetMinSize(context, arg); }, cmd);
    }
}

inline auto GetMinSizes(Context context, std::vector<CalChart::Draw::DrawCommand> const& commands) -> std::vector<StackSize>
{
    auto result = std::vector<StackSize>{};
    result.reserve(commands.size());
    std::transform(commands.cbegin(), commands.cend(), std::back_inserter(result), [context](auto const& command) {
        return details::GetMinSize(context, command);
    });
    return result;
}

}
