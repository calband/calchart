#pragma once
/*
 * CalChartDrawingLayout.h
 * Utilities for Drawing Layout
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

#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawingGetMinSize.h"
#include "CalChartRanges.h"
#include <wx/dc.h>

namespace wxCalChart::Draw {

// The Layout functions take an array of StackSizes and calculate the points where they should be placed.
// See CalChartDrawCommand.h for the intent of the different alignments.  This file contains the details
// of how to do it with the wxWidgets drawing system.

template <std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, StackSize>
auto HLayout(Range const& sizes, DrawSurface surface, CalChart::Draw::StackAlign align) -> std::vector<wxPoint>
{
    if (sizes.empty()) {
        return {};
    }
    auto const minSize = details::HMinSize(sizes);
    auto firstSize = std::visit(CalChart::overloaded{
                                    [](wxSize size) { return size; },
                                    [](auto&& tabStop) { return tabStop.tabFunc({}); },
                                },
        sizes.front());
    auto [start, between] = [size = static_cast<int>(sizes.size()), availableSize = surface.size, minSize, firstSize](auto align) -> std::tuple<int, int> {
        switch (align) {
        case CalChart::Draw::StackAlign::Begin:
            return { 0, 0 };
        case CalChart::Draw::StackAlign::End:
            return { (availableSize - minSize).x, 0 };
        case CalChart::Draw::StackAlign::Uniform:
            if (size > 1) {
                auto space = (availableSize - minSize).x / size;
                return { space / 2, space };
            }
            return { (availableSize - firstSize).x / 2, 0 };
        case CalChart::Draw::StackAlign::Justified:
            if (size > 1) {
                auto space = (availableSize - minSize).x / (size - 1);
                return { 0, space };
            }
            return { (availableSize - firstSize).x / 2, 0 };
        }
    }(align);

    auto offsets = std::vector<int>{};
    std::exclusive_scan(
        sizes.begin(),
        sizes.end(),
        std::back_inserter(offsets),
        start,
        [between](auto&& acc, auto&& input) {
            return std::visit(CalChart::overloaded{
                                  [acc, between](wxSize size) { return acc + between + size.x; },
                                  [acc](auto&& tabStop) { return tabStop.tabFunc(wxSize{ acc, 0 }).x; },
                              },
                input);
        });
    auto results = CalChart::Ranges::ToVector<wxPoint>(
        offsets
        | std::views::transform([](auto&& input) { return wxPoint{ input, 0 }; })
        | std::views::transform([offset = surface.origin](auto&& point) { return point + offset; }));
    return results;
}

template <std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, StackSize>
auto VLayout(Range const& sizes, DrawSurface surface, CalChart::Draw::StackAlign align) -> std::vector<wxPoint>
{
    if (sizes.empty()) {
        return {};
    }
    auto const minSize = details::VMinSize(sizes);
    auto firstSize = std::visit(CalChart::overloaded{
                                    [](wxSize size) { return size; },
                                    [](auto&& tabStop) { return tabStop.tabFunc({}); },
                                },
        sizes.front());
    auto [start, between] = [size = static_cast<int>(sizes.size()), availableSize = surface.size, minSize, firstSize](auto align) -> std::tuple<int, int> {
        switch (align) {
        case CalChart::Draw::StackAlign::Begin:
            return { 0, 0 };
        case CalChart::Draw::StackAlign::End:
            return { (availableSize - minSize).y, 0 };
        case CalChart::Draw::StackAlign::Uniform:
            if (size > 1) {
                auto space = (availableSize - minSize).y / size;
                return { space / 2, space };
            }
            return { (availableSize - firstSize).y / 2, 0 };
        case CalChart::Draw::StackAlign::Justified:
            if (size > 1) {
                auto space = (availableSize - minSize).y / (size - 1);
                return { 0, space };
            }
            return { (availableSize - firstSize).y / 2, 0 };
        }
    }(align);

    auto offsets = std::vector<int>{};
    std::exclusive_scan(
        sizes.begin(),
        sizes.end(),
        std::back_inserter(offsets),
        start,
        [between](auto&& acc, auto&& input) {
            return std::visit(CalChart::overloaded{
                                  [acc, between](wxSize size) { return acc + between + size.y; },
                                  [acc](auto&& tabStop) { return tabStop.tabFunc(wxSize{ acc, 0 }).y; },
                              },
                input);
        });
    auto results = CalChart::Ranges::ToVector<wxPoint>(
        offsets
        | std::views::transform([](auto&& input) { return wxPoint{ 0, input }; })
        | std::views::transform([offset = surface.origin](auto&& point) { return point + offset; }));
    return results;
}

template <std::ranges::range Range>
    requires std::convertible_to<std::ranges::range_value_t<Range>, StackSize>
auto ZLayout(Range const& sizes, DrawSurface surface, [[maybe_unused]] CalChart::Draw::StackAlign align) -> std::vector<wxPoint>
{
    return std::vector<wxPoint>{ sizes.size(), surface.origin };
}

}
