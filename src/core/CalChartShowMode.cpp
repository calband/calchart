/*
 * modes.cpp
 * Handle show mode classes
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

#include "CalChartShowMode.h"
#include "CalChartFileFormat.h"

#include <algorithm>
#include <cassert>

namespace CalChart {

auto ShowMode::GetDefaultYardLines() -> ShowMode::YardLinesInfo_t
{
    static auto kYardTextDefaults = std::array<std::string, kYardTextValues>{
        "N", "M", "L", "K", "J", "I", "H", "G", "F", "E", "D", "C", "B", "A", "-10", "-5",
        "0", "5", "10", "15", "20", "25", "30", "35", "40", "45", "50",
        "45", "40", "35", "30", "25", "20", "15", "10", "5", "0",
        "-5", "-10", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N"
    };

    return kYardTextDefaults;
}

ShowMode::ShowMode(CalChart::Coord size,
    CalChart::Coord offset,
    CalChart::Coord border1,
    CalChart::Coord border2,
    uint16_t whash,
    uint16_t ehash,
    YardLinesInfo_t yardlines)
    : mSize(size)
    , mOffset(offset)
    , mBorder1(border1)
    , mBorder2(border2)
    , mHashW(whash)
    , mHashE(ehash)
    , mYardLines(std::move(yardlines))
{
}

auto ShowMode::ClipPosition(const CalChart::Coord& pos) const -> CalChart::Coord
{
    auto min = MinPosition();
    auto max = MaxPosition();

    CalChart::Coord clipped;
    if (pos.x < min.x) {
        clipped.x = min.x;
    } else if (pos.x > max.x) {
        clipped.x = max.x;
    } else {
        clipped.x = pos.x;
    }
    if (pos.y < min.y) {
        clipped.y = min.y;
    } else if (pos.y > max.y) {
        clipped.y = max.y;
    } else {
        clipped.y = pos.y;
    }
    return clipped;
}

auto ShowMode::GetShowModeData() const -> ShowModeData_t
{
    return { {
        mHashW,
        mHashE,
        CoordUnits2Int(mBorder1.x),
        CoordUnits2Int(mBorder1.y),
        CoordUnits2Int(mBorder2.x),
        CoordUnits2Int(mBorder2.y),
        CoordUnits2Int(-mOffset.x),
        CoordUnits2Int(-mOffset.y),
        CoordUnits2Int(mSize.x),
        CoordUnits2Int(mSize.y),
    } };
}

auto ShowMode::CreateShowMode(ShowModeData_t const& values, YardLinesInfo_t const& yardlines) -> ShowMode
{
    auto whash = static_cast<uint16_t>(values[kwhash]);
    auto ehash = static_cast<uint16_t>(values[kehash]);
    auto bord1 = CalChart::Coord{ Int2CoordUnits(values[kbord1_x]), Int2CoordUnits(values[kbord1_y]) };
    auto bord2 = CalChart::Coord{ Int2CoordUnits(values[kbord2_x]), Int2CoordUnits(values[kbord2_y]) };
    auto offset = CalChart::Coord{ Int2CoordUnits(-values[koffset_x]), Int2CoordUnits(-values[koffset_y]) };
    auto size = CalChart::Coord{ Int2CoordUnits(values[ksize_x]), Int2CoordUnits(values[ksize_y]) };
    return {
        size,
        offset,
        bord1,
        bord2,
        whash,
        ehash,
        yardlines
    };
}

auto ShowMode::CreateShowMode(Coord size, Coord offset, Coord border1, Coord border2, uint16_t whash, uint16_t ehash, YardLinesInfo_t const& yardlines) -> ShowMode
{
    return {
        size,
        offset,
        border1,
        border2,
        whash,
        ehash,
        yardlines
    };
}

auto ShowMode::CreateShowMode(CalChart::Reader reader) -> ShowMode
{
    ShowModeData_t values;
    for (auto& i : values) {
        if (reader.size() < 4) {
            throw std::runtime_error("Error, size of ShowMode is not correct");
        }
        i = reader.Get<uint32_t>();
    }
    YardLinesInfo_t yardlines;
    for (auto& i : yardlines) {
        if (reader.size() < 1) {
            throw std::runtime_error("Error, yardtext does not have enough for a null terminator");
        }
        i = reader.Get<std::string>();
    }
    if (reader.size() != 0) {
        throw std::runtime_error("Error, size of ShowMode is not correct");
    }
    return ShowMode::CreateShowMode(values, yardlines);
}

auto ShowMode::Serialize() const -> std::vector<uint8_t>
{
    std::vector<uint8_t> result;
    for (uint32_t const i : GetShowModeData()) {
        Parser::Append(result, i);
    }
    for (auto&& i : Get_yard_text()) {
        Parser::AppendAndNullTerminate(result, i);
    }
    return result;
}

auto ShowMode::GetDefaultShowMode() -> ShowMode
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
    return ShowMode::CreateShowMode({ { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } }, GetDefaultYardLines());
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
}

auto operator==(ShowMode const& lhs, ShowMode const& rhs) -> bool
{
    return lhs.mSize == rhs.mSize
        && lhs.mOffset == rhs.mOffset
        && lhs.mBorder1 == rhs.mBorder1
        && lhs.mBorder2 == rhs.mBorder2
        && lhs.mHashW == rhs.mHashW
        && lhs.mHashE == rhs.mHashE
        && lhs.mYardLines == rhs.mYardLines;
}

static constexpr auto kStep8 = CalChart::Int2CoordUnits(8);
static constexpr auto kStep1 = CalChart::Int2CoordUnits(1);

auto CreateFieldLayout(
    ShowMode const& mode,
    bool withDetails) -> std::vector<CalChart::DrawCommand>
{
    auto drawCmds = std::vector<CalChart::DrawCommand>{};
    append(drawCmds, CalChart::Draw::Field::CreateOutline(mode.FieldSize()));
    append(drawCmds, CalChart::Draw::Field::CreateVerticalSolidLine(mode.FieldSize(), kStep1));

    if (withDetails) {
        append(drawCmds, CalChart::Draw::Field::CreateVerticalDottedLine(mode.FieldSize(), kStep1));
        append(drawCmds, CalChart::Draw::Field::CreateHorizontalDottedLine(mode.FieldSize(), mode.HashW(), mode.HashE(), kStep1));
    }

    if (mode.HashW() != static_cast<uint16_t>(-1)) {
        append(drawCmds, CalChart::Draw::Field::CreateHashes(mode.FieldSize(), mode.HashW(), mode.HashE(), kStep1));
        if (withDetails) {
            append(drawCmds, CalChart::Draw::Field::CreateHashTicks(mode.FieldSize(), mode.HashW(), mode.HashE(), kStep1));
        }
    }
    return drawCmds;
}

auto CreateYardlineLayout(
    ShowMode const& mode,
    bool largeOffset) -> std::vector<CalChart::DrawCommand>
{
    auto drawCmds = std::vector<CalChart::DrawCommand>{};
    auto yard_text = mode.Get_yard_text();
    auto yard_text2 = std::vector<std::string>(yard_text.begin() + (-CalChart::CoordUnits2Int((mode.Offset() - mode.Border1()).x) + (CalChart::kYardTextValues - 1) * 4) / 8, yard_text.end());
    return CalChart::Draw::Field::CreateYardlineLabels(yard_text2, mode.FieldSize(), largeOffset ? kStep8 : 0, kStep1);
}

}
