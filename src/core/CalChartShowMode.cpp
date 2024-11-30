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
#include "CalChartConfiguration.h"
#include "CalChartConstants.h"
#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimatives.h"
#include "CalChartFileFormat.h"

#include <algorithm>
#include <cassert>

namespace CalChart {

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

auto ShowMode::Serialize() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
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
    return ShowMode::CreateShowMode({ { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } }, kDefaultYardLines);
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
    bool withDetails) -> std::vector<Draw::DrawCommand>
{
    auto drawCmds = std::vector<Draw::DrawCommand>{};
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
    bool largeOffset) -> std::vector<Draw::DrawCommand>
{
    auto drawCmds = std::vector<Draw::DrawCommand>{};
    auto yard_text = mode.Get_yard_text();
    auto yard_text2 = std::vector<std::string>(yard_text.begin() + (-CalChart::CoordUnits2Int((mode.Offset() - mode.Border1()).x) + (CalChart::kYardTextValues - 1) * 4) / 8, yard_text.end());
    return CalChart::Draw::Field::CreateYardlineLabels(yard_text2, mode.FieldSize(), largeOffset ? kStep8 : 0, kStep1);
}

auto CreateModeDrawCommands(
    Configuration const& config,
    ShowMode const& mode,
    HowToDraw howToDraw) -> std::vector<CalChart::Draw::DrawCommand>
{
    auto result = std::vector<CalChart::Draw::DrawCommand>{};
    auto inBlackAndWhite = howToDraw == HowToDraw::Printing;

    auto fieldPen = inBlackAndWhite
        ? CalChart::Pen{ CalChart::Color::Black() }
        : CalChart::toPen(config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD_DETAIL));
    auto fieldText = inBlackAndWhite
        ? CalChart::BrushAndPen{ CalChart::Color::Black() }
        : config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD_TEXT);
    auto fieldBrush = CalChart::Brush::TransparentBrush();

    auto field = CalChart::CreateFieldLayout(mode, howToDraw == HowToDraw::FieldView || howToDraw == HowToDraw::Printing);

    CalChart::append(result,
        CalChart::Draw::withPen(
            fieldPen,
            CalChart::Draw::withTextForeground(
                fieldText,
                CalChart::Draw::withBrush(
                    fieldBrush,
                    field))));

    if (howToDraw == HowToDraw::Animation) {
        return result;
    }

    auto font = CalChart::Font{ CalChart::Float2CoordUnits(config.Get_YardsSize()) };
    auto brushAndPen = inBlackAndWhite ? CalChart::BrushAndPen{ .brushStyle = CalChart::Brush::Style::Transparent } : config.Get_CalChartBrushAndPen(CalChart::Colors::FIELD);
    auto yardLabels = CalChart::CreateYardlineLayout(mode, howToDraw == HowToDraw::OmniView);
    CalChart::append(result,
        CalChart::Draw::withFont(
            font,
            CalChart::Draw::withBrushAndPen(
                brushAndPen,
                yardLabels)));

    return result;
}

auto CreateModeDrawCommandsWithBorder(
    Configuration const& config,
    ShowMode const& mode,
    HowToDraw howToDraw) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CreateModeDrawCommands(config, mode, howToDraw) + mode.Border1();
}

auto CreateModeDrawCommandsWithBorderOffset(
    Configuration const& config,
    ShowMode const& mode,
    HowToDraw howToDraw) -> std::vector<CalChart::Draw::DrawCommand>
{
    return CreateModeDrawCommandsWithBorder(config, mode, howToDraw) - mode.Offset();
}

auto ShowMode::CreateFieldForPrinting(int left_limit, int right_limit, bool landscape) const -> CalChart::ShowMode
{
    // extend the limit to the next largest 5 yard line
    left_limit = (left_limit / 8) * 8 + (left_limit % 8 ? (left_limit < 0 ? -8 : 8) : 0);
    right_limit = (right_limit / 8) * 8 + (right_limit % 8 ? (right_limit < 0 ? -8 : 8) : 0);

    auto size_x = std::max(CalChart::kFieldStepSizeNorthSouth[landscape], right_limit - left_limit);
    auto size = CalChart::Coord{ CalChart::Int2CoordUnits(size_x), CalChart::Int2CoordUnits(CalChart::kFieldStepSizeEastWest) };

    auto left_edge = -CalChart::kFieldStepSizeSouthEdgeFromCenter[landscape];
    if (left_limit < left_edge) {
        left_edge = left_limit;
    } else if ((left_edge + size.x) < right_limit) {
        left_edge = right_limit - size.x;
    }
    CalChart::Coord off = { CalChart::Int2CoordUnits(-left_edge), CalChart::Int2CoordUnits(CalChart::kFieldStepSizeWestEdgeFromCenter) };

    return CreateShowMode(size, off, { 0, 0 }, { 0, 0 }, CalChart::kFieldStepWestHashFromWestSideline, CalChart::kFieldStepEastHashFromWestSideline, mYardLines);
}

}
