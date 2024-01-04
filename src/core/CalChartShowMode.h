#pragma once
/*
 * CalChartShowMode.h
 * Definitions for the show mode classes
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
#include "CalChartDrawCommand.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace CalChart {

class Reader;
struct Font;
struct BrushAndPen;

static constexpr auto kYardTextValues = 53;
static constexpr auto kSprLineTextValues = 5;

static constexpr auto kFieldStepSizeNorthSouth = std::array{ 96, 160 };
static constexpr auto kFieldStepSizeEastWest = 84;
static constexpr auto kFieldStepSizeSouthEdgeFromCenter = std::array{ 48, 80 };
static constexpr auto kFieldStepSizeWestEdgeFromCenter = 42;
static constexpr auto kFieldStepWestHashFromWestSideline = 32;
static constexpr auto kFieldStepEastHashFromWestSideline = 52;

class ShowMode {
public:
    static auto GetDefaultShowMode() -> ShowMode;

    using YardLinesInfo_t = std::array<std::string, kYardTextValues>;
    static auto GetDefaultYardLines() -> YardLinesInfo_t;

    enum ShowModeArrayValues {
        kwhash,
        kehash,
        kbord1_x,
        kbord1_y,
        kbord2_x,
        kbord2_y,
        koffset_x,
        koffset_y,
        ksize_x,
        ksize_y,
        kShowModeValues
    };

    static auto CreateShowMode(ShowModeData_t const& values, YardLinesInfo_t const& yardlines) -> ShowMode;
    static auto CreateShowMode(Coord size, Coord offset, Coord border1, Coord border2, uint16_t whash, uint16_t ehash, YardLinesInfo_t const& yardlines) -> ShowMode;
    static auto CreateShowMode(CalChart::Reader reader) -> ShowMode;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte>;

    [[nodiscard]] auto GetShowModeData() const -> ShowModeData_t;

    [[nodiscard]] auto Offset() const { return mOffset + mBorder1; }
    [[nodiscard]] auto FieldOffset() const { return -mOffset; }
    [[nodiscard]] auto Size() const { return mSize + mBorder1 + mBorder2; }
    [[nodiscard]] auto FieldSize() const { return mSize; }
    [[nodiscard]] auto MinPosition() const { return -(mOffset + mBorder1); }
    [[nodiscard]] auto MaxPosition() const { return mSize - mOffset + mBorder2; }
    [[nodiscard]] auto Border1() const { return mBorder1; }
    [[nodiscard]] auto ClipPosition(const CalChart::Coord& pos) const -> CalChart::Coord;

    [[nodiscard]] auto HashW() const { return mHashW; }
    [[nodiscard]] auto HashE() const { return mHashE; }

    // Yard Lines
    [[nodiscard]] auto Get_yard_text() const -> YardLinesInfo_t { return mYardLines; }

private:
    ShowMode(CalChart::Coord size,
        CalChart::Coord offset,
        CalChart::Coord border1,
        CalChart::Coord border2,
        uint16_t whash,
        uint16_t ehash,
        YardLinesInfo_t yardlines);

    CalChart::Coord mSize;
    CalChart::Coord mOffset;
    CalChart::Coord mBorder1;
    CalChart::Coord mBorder2;

    uint16_t mHashW, mHashE;

    YardLinesInfo_t mYardLines;

    friend auto operator==(ShowMode const& lhs, ShowMode const& rhs) -> bool;
};

auto CreateFieldLayout(
    ShowMode const& mode,
    bool withDetails) -> std::vector<Draw::DrawCommand>;

auto CreateYardlineLayout(
    ShowMode const& mode,
    bool largeOffset) -> std::vector<Draw::DrawCommand>;

}
