#pragma once
/*
 * CalChartConstants.h
 * Common constants and enums for CalChart.
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

#include "CalChartAngles.h"
#include "CalChartUtils.h"
#include <array>
#include <string>
#include <vector>

namespace CalChart {

enum class Colors {
    FIELD,
    FIELD_DETAIL,
    FIELD_TEXT,
    POINT,
    POINT_TEXT,
    POINT_HILIT,
    POINT_HILIT_TEXT,
    REF_POINT,
    REF_POINT_TEXT,
    REF_POINT_HILIT,
    REF_POINT_HILIT_TEXT,
    GHOST_POINT,
    GHOST_POINT_TEXT,
    GHOST_POINT_HLIT,
    GHOST_POINT_HLIT_TEXT,
    POINT_ANIM_FRONT,
    POINT_ANIM_BACK,
    POINT_ANIM_SIDE,
    POINT_ANIM_HILIT_FRONT,
    POINT_ANIM_HILIT_BACK,
    POINT_ANIM_HILIT_SIDE,
    POINT_ANIM_COLLISION,
    POINT_ANIM_COLLISION_WARNING,
    SHAPES,
    PATHS,
    SHEET_CURVE,
    SHEET_CURVE_CONTROL_POINT,
    DRAW_CURVE,
    DRAW_CURVE_CONTROL_POINT,
    NUM
};
using ColorsIterator = CalChart::Iterator<Colors, Colors::FIELD, Colors::DRAW_CURVE_CONTROL_POINT>;

using ColorInfoDefault_t = std::tuple<std::string const, std::string const, int const>;
static const std::array<ColorInfoDefault_t, toUType(Colors::NUM)> ColorInfoDefaults = {
    ColorInfoDefault_t{ "FIELD", "FOREST GREEN", 1 },
    ColorInfoDefault_t{ "FIELD DETAIL", "WHITE", 1 },
    ColorInfoDefault_t{ "FIELD TEXT", "BLACK", 1 },
    ColorInfoDefault_t{ "POINT", "WHITE", 1 },
    ColorInfoDefault_t{ "POINT TEXT", "BLACK", 1 },
    ColorInfoDefault_t{ "HILIT POINT", "YELLOW", 1 },
    ColorInfoDefault_t{ "HILIT POINT TEXT", "YELLOW", 1 },
    ColorInfoDefault_t{ "REF POINT", "PURPLE", 1 },
    ColorInfoDefault_t{ "REF POINT TEXT", "BLACK", 1 },
    ColorInfoDefault_t{ "HILIT REF POINT", "PURPLE", 1 },
    ColorInfoDefault_t{ "HILIT REF POINT TEXT", "BLACK", 1 },
    ColorInfoDefault_t{ "GHOST POINT", "BLUE", 1 },
    ColorInfoDefault_t{ "GHOST POINT TEXT", "NAVY", 1 },
    ColorInfoDefault_t{ "HLIT GHOST POINT", "PURPLE", 1 },
    ColorInfoDefault_t{ "HLIT GHOST POINT TEXT", "PLUM", 1 },
    ColorInfoDefault_t{ "ANIM FRONT", "WHITE", 1 },
    ColorInfoDefault_t{ "ANIM BACK", "YELLOW", 1 },
    ColorInfoDefault_t{ "ANIM SIDE", "SKY BLUE", 1 },
    ColorInfoDefault_t{ "HILIT ANIM FRONT", "RED", 1 },
    ColorInfoDefault_t{ "HILIT ANIM BACK", "RED", 1 },
    ColorInfoDefault_t{ "HILIT ANIM SIDE", "RED", 1 },
    ColorInfoDefault_t{ "ANIM COLLISION", "PURPLE", 1 },
    ColorInfoDefault_t{ "ANIM COLLISION WARNING", "CORAL", 1 },
    ColorInfoDefault_t{ "SHAPES", "ORANGE", 2 },
    ColorInfoDefault_t{ "CONTINUITY PATHS", "RED", 1 },
    ColorInfoDefault_t{ "CURVE", "BLUE", 2 },
    ColorInfoDefault_t{ "CURVE CONTROL POINTS", "YELLOW", 1 },
    ColorInfoDefault_t{ "DRAW CURVE", "RED", 2 },
    ColorInfoDefault_t{ "DRAW CURVE CONTROL POINTS", "RED", 1 },
};

enum class ContinuityCellColors {
    PROC,
    VALUE,
    FUNCTION,
    DIRECTION,
    STEPTYPE,
    POINT,
    UNSET,
    OUTLINE,
    SELECTED,
    NUM,
};
using ContinuityCellColorsIterator = CalChart::Iterator<ContinuityCellColors, ContinuityCellColors::PROC, ContinuityCellColors::SELECTED>;

static const std::array<ColorInfoDefault_t, toUType(ContinuityCellColors::NUM)> ContCellColorInfoDefaults = {
    ColorInfoDefault_t{ "CONT CELL PROCEDURE", "LIME GREEN", 1 },
    ColorInfoDefault_t{ "CONT CELL VALUE", "YELLOW", 1 },
    ColorInfoDefault_t{ "CONT CELL FUNCTION", "SLATE BLUE", 1 },
    ColorInfoDefault_t{ "CONT CELL DIRECTION", "MEDIUM ORCHID", 1 },
    ColorInfoDefault_t{ "CONT CELL STEPTYPE", "SKY BLUE", 1 },
    ColorInfoDefault_t{ "CONT CELL POINT", "GOLD", 1 },
    ColorInfoDefault_t{ "CONT CELL UNSET", "WHITE", 1 },
    ColorInfoDefault_t{ "CONT CELL OUTLINE", "BLACK", 1 },
    ColorInfoDefault_t{ "CONT CELL SELECTED", "PINK", 3 },
};

enum class ShowModes {
    STANDARD,
    FULL_FIELD,
    TUNNEL,
    OLD_FIELD,
    PRO_FIELD,
    NUM
};

static constexpr auto kShowModeValues = 10;
// Specialize on show mode
static const std::array<std::string const, kShowModeValues> ShowModeKeys = {
    "whash",
    "ehash",
    "bord1_x",
    "bord1_y",
    "bord2_x",
    "bord2_y",
    "size_x",
    "size_y",
    "offset_x",
    "offset_y",
};

using ShowModeData_t = std::array<long, kShowModeValues>;
using ShowModeInfo_t = std::tuple<std::string const, ShowModeData_t>;

// What values mean:
// whash ehash (steps from west sideline)
// left top right bottom (border in steps)
// x y w h (region of the field to use, in steps)
static const std::array<ShowModeInfo_t, toUType(ShowModes::NUM)> kShowModeDefaultValues = {
    ShowModeInfo_t{ "Standard", { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } },
    ShowModeInfo_t{ "Full Field", { 32, 52, 8, 8, 8, 8, -96, -42, 192, 84 } },
    ShowModeInfo_t{ "Tunnel", { 32, 52, 8, 8, 8, 8, 16, -42, 192, 84 } },
    ShowModeInfo_t{ "Old Field", { 28, 52, 8, 8, 8, 8, -80, -42, 160, 84 } },
    ShowModeInfo_t{ "Pro Field", { 36, 48, 8, 8, 8, 8, -80, -42, 160, 84 } },
};

static constexpr auto kYardTextValues = 53;
using YardLinesInfo_t = std::array<std::string, kYardTextValues>;

static const YardLinesInfo_t kDefaultYardLines = {
    "N", "M", "L", "K", "J", "I", "H", "G", "F", "E", "D", "C", "B", "A", "-10", "-5",
    "0", "5", "10", "15", "20", "25", "30", "35", "40", "45", "50",
    "45", "40", "35", "30", "25", "20", "15", "10", "5", "0",
    "-5", "-10", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N"
};

enum class PSFONT {
    SYMBOL,
    NORM,
    BOLD,
    ITAL,
    BOLDITAL
};

enum SYMBOL_TYPE {
    SYMBOL_PLAIN = 0,
    SYMBOL_SOL,
    SYMBOL_BKSL,
    SYMBOL_SL,
    SYMBOL_X,
    SYMBOL_SOLBKSL,
    SYMBOL_SOLSL,
    SYMBOL_SOLX,
    MAX_NUM_SYMBOLS
};

static const auto k_symbols = std::array<SYMBOL_TYPE, MAX_NUM_SYMBOLS>{
    SYMBOL_PLAIN,
    SYMBOL_SOL,
    SYMBOL_BKSL,
    SYMBOL_SL,
    SYMBOL_X,
    SYMBOL_SOLBKSL,
    SYMBOL_SOLSL,
    SYMBOL_SOLX
};

constexpr auto kNumberPalettes = 4; // arbitrary, could go more, but 4 is a good starting point

const std::array<std::string, kNumberPalettes> kPaletteColorDefault = {
    "FOREST GREEN",
    "GREY",
    "GREY",
    "GREY",
};

const std::array<std::string, kNumberPalettes> kPaletteNameDefault = {
    "Default",
    "[Unset]",
    "[Unset]",
    "[Unset]",
};

auto GetNameForSymbol(SYMBOL_TYPE which) -> std::string;
auto GetLongNameForSymbol(SYMBOL_TYPE which) -> std::string;
auto GetSymbolForName(std::string const& name) -> SYMBOL_TYPE;

namespace details {
    template <std::size_t index, typename Array, std::size_t... I>
    auto at_offset(const Array& a, std::index_sequence<I...>)
    {
        return std::array{ std::get<index>(a[I])... };
    }
    template <int Which, typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
    auto vector_at_offset(std::array<T, N> const& a)
    {
        auto data = at_offset<Which>(a, Indices{});
        return std::vector(data.begin(), data.end());
    }
}

inline auto GetShowModeNames() -> std::vector<std::string>
{
    static auto result = details::vector_at_offset<0>(kShowModeDefaultValues);
    return result;
}

inline auto GetColorNames() -> std::vector<std::string>
{
    static auto result = details::vector_at_offset<0>(CalChart::ColorInfoDefaults);
    return result;
}

inline auto GetDefaultColors() -> std::vector<std::string>
{
    static auto result = details::vector_at_offset<1>(CalChart::ColorInfoDefaults);
    return result;
}

inline auto GetDefaultPenWidth() -> std::vector<int>
{
    static auto result = details::vector_at_offset<2>(CalChart::ColorInfoDefaults);
    return result;
}

inline auto GetContCellColorNames() -> std::vector<std::string>
{
    static auto result = details::vector_at_offset<0>(CalChart::ContCellColorInfoDefaults);
    return result;
}

inline auto GetContCellDefaultColors() -> std::vector<std::string>
{
    static auto result = details::vector_at_offset<1>(CalChart::ContCellColorInfoDefaults);
    return result;
}

inline auto GetContCellDefaultPenWidth() -> std::vector<int>
{
    static auto result = details::vector_at_offset<2>(CalChart::ContCellColorInfoDefaults);
    return result;
}

inline constexpr auto kViewPoint_x_1 = 0.0;
inline constexpr auto kViewPoint_y_1 = -60.0;
inline constexpr auto kViewPoint_z_1 = 20.0;
inline constexpr auto kViewAngle_1 = CalChart::pi / 2.0;
inline constexpr auto kViewAngle_z_1 = -CalChart::pi / 8.0;

inline constexpr auto kViewPoint_x_2 = 0.0;
inline constexpr auto kViewPoint_y_2 = -16.0;
inline constexpr auto kViewPoint_z_2 = 2.5;
inline constexpr auto kViewAngle_2 = CalChart::pi / 2.0;
inline constexpr auto kViewAngle_z_2 = CalChart::Radian{};

inline constexpr auto kViewPoint_x_3 = 60.0;
inline constexpr auto kViewPoint_y_3 = -55.0;
inline constexpr auto kViewPoint_z_3 = 20.0;
inline constexpr auto kViewAngle_3 = 11.0 * CalChart::pi / 16.0;
inline constexpr auto kViewAngle_z_3 = -CalChart::pi / 8.0;

}