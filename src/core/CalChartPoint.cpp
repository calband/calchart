/*
 * CalChartPoint.cpp
 * Definition for the point classes
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

#include "CalChartPoint.h"
#include "CalChartFileFormat.h"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace CalChart {

Point::Point()
    : mSym(SYMBOL_PLAIN)
{
}

Point::Point(Coord const& p, SYMBOL_TYPE sym)
    : mSym(sym)
    , mPos(p)
{
    mRef.fill(mPos);
}

auto ReadPositionData(Reader& reader)
{
    auto pos = Coord{};
    pos.x = static_cast<Coord::units>(reader.Get<uint16_t>());
    pos.y = static_cast<Coord::units>(reader.Get<uint16_t>());
    return pos;
}

auto WritePositionData(Coord pos)
{
    auto result = std::vector<std::byte>{};
    Parser::Append(result, static_cast<uint16_t>(pos.x));
    Parser::Append(result, static_cast<uint16_t>(pos.y));
    return result;
}

// EACH_POINT_DATA    = BigEndianInt8(Size_rest_of_EACH_POINT_DATA) ,
// POSITION_DATA , REF_POSITION_DATA , POINT_SYMBOL_DATA , POINT_LABEL_FLIP ;
// POSITION_DATA      = BigEndianInt16( x ) , BigEndianInt16( y ) ;
// REF_POSITION_DATA  = BigEndianInt8( num ref pts ) , { BigEndianInt8( which
// reference point ) , BigEndianInt16( x ) , BigEndianInt16( y ) }* ;
// POINT_SYMBOL_DATA  = BigEndianInt8( which symbol type ) ;
// POINT_LABEL_FLIP_DATA = BigEndianInt8( label flipped ) ;
Point::Point(Reader reader)
    : mSym(SYMBOL_PLAIN)
{
    mPos = ReadPositionData(reader);
    // set all the reference points
    mRef.fill(mPos);

    auto const num_ref = reader.Get<uint8_t>();
    for (auto i = 0; i < num_ref; ++i) {
        auto const ref = reader.Get<uint8_t>();
        mRef.at(ref - 1) = ReadPositionData(reader);
    }
    mSym = static_cast<SYMBOL_TYPE>(reader.Get<uint8_t>());
    mFlags.set(kPointLabelFlipped, (reader.Get<uint8_t>()) > 0);
    if (reader.size() != 0) {
        throw CC_FileException("bad POS chunk");
    }
}

auto Point::SerializeHelper() const -> std::vector<std::byte>
{
    // how many reference points are we going to write?
    auto pointsToWrite = std::vector<unsigned>{};
    for (auto j = 1; j <= Point::kNumRefPoints; j++) {
        if (GetPos(j) != GetPos(0)) {
            pointsToWrite.push_back(j);
        }
    }
    auto result = std::vector<std::byte>{};
    // Point positions
    // Write block size

    // Write POSITION
    Parser::Append(result, WritePositionData(GetPos()));

    // Write REF_POS
    Parser::Append(result, static_cast<uint8_t>(pointsToWrite.size()));
    if (!pointsToWrite.empty()) {
        for (auto j : pointsToWrite) {
            Parser::Append(result, static_cast<uint8_t>(j));
            Parser::Append(result, WritePositionData(GetPos(j)));
        }
    }

    // Write SYMBOL
    Parser::Append(result, static_cast<uint8_t>(GetSymbol()));

    // Point labels (left or right)
    Parser::Append(result, static_cast<uint8_t>(GetFlip()));

    return result;
}

auto Point::Serialize() const -> std::vector<std::byte>
{
    auto const serializedData = SerializeHelper();
    std::vector<std::byte> result;
    Parser::Append(result, static_cast<uint8_t>(serializedData.size()));
    Parser::Append(result, serializedData);
    return result;
}

void Point::Flip(bool val) { mFlags.set(kPointLabelFlipped, val); };

void Point::SetLabelVisibility(bool isVisible) { mFlags.set(kLabelIsInvisible, !isVisible); }

auto Point::GetPos(unsigned ref) const -> Coord
{
    if (ref == 0) {
        return mPos;
    }
    if (ref > kNumRefPoints) {
        throw std::range_error("GetPos() point out of range");
    }
    return mRef.at(ref - 1);
}

void Point::SetPos(Coord c, unsigned ref)
{
    if (ref == 0) {
        mPos = c;
        return;
    }
    if (ref > kNumRefPoints) {
        throw std::range_error("SetRefPos() point out of range");
    }
    mRef.at(ref - 1) = c;
}

void Point::SetSymbol(SYMBOL_TYPE s) { mSym = s; }

namespace {
    auto CreatePointCircle(CalChart::SYMBOL_TYPE symbol, double dotRatio) -> std::vector<CalChart::DrawCommand>
    {
        auto const filled = [](auto symbol) {
            switch (symbol) {
            case CalChart::SYMBOL_SOL:
            case CalChart::SYMBOL_SOLBKSL:
            case CalChart::SYMBOL_SOLSL:
            case CalChart::SYMBOL_SOLX:
                return true;
                break;
            default:
                return false;
            }
        }(symbol);

        auto const circ_r = CalChart::Float2CoordUnits(dotRatio) / 2.0;

        return { CalChart::Draw::Circle({ 0, 0 }, circ_r, filled) };
    }

    auto CreatePointCross(CalChart::SYMBOL_TYPE symbol, double dotRatio, double pLineRatio, double sLineRatio) -> std::vector<CalChart::DrawCommand>
    {
        auto const circ_r = CalChart::Float2CoordUnits(dotRatio) / 2.0;
        auto const plineoff = circ_r * pLineRatio;
        auto const slineoff = circ_r * sLineRatio;

        auto drawCmds = std::vector<CalChart::DrawCommand>{};
        switch (symbol) {
        case CalChart::SYMBOL_SL:
        case CalChart::SYMBOL_X:
            drawCmds.push_back(CalChart::Draw::Line(-plineoff, plineoff, plineoff, -plineoff));
            break;
        case CalChart::SYMBOL_SOLSL:
        case CalChart::SYMBOL_SOLX:
            drawCmds.push_back(CalChart::Draw::Line(-slineoff, slineoff, slineoff, -slineoff));
            break;
        default:
            break;
        }
        switch (symbol) {
        case CalChart::SYMBOL_BKSL:
        case CalChart::SYMBOL_X:
            drawCmds.push_back(CalChart::Draw::Line(-plineoff, -plineoff, plineoff, plineoff));
            break;
        case CalChart::SYMBOL_SOLBKSL:
        case CalChart::SYMBOL_SOLX:
            drawCmds.push_back(CalChart::Draw::Line(-slineoff, -slineoff, slineoff, slineoff));
            break;
        default:
            break;
        }
        return drawCmds;
    }

    auto CreatePointLabel(CalChart::Point const& point, std::string const& label, double dotRatio) -> std::vector<CalChart::DrawCommand>
    {
        if (!point.LabelIsVisible()) {
            return {};
        }
        auto const circ_r = CalChart::Float2CoordUnits(dotRatio) / 2.0;
        auto anchor = CalChart::Draw::Text::TextAnchor::Bottom;
        anchor = anchor | (point.GetFlip() ? CalChart::Draw::Text::TextAnchor::Left : CalChart::Draw::Text::TextAnchor::Right);
        return { CalChart::Draw::Text(-CalChart::Coord(0, circ_r), label, anchor) };
    }

    auto CreatePoint(CalChart::Point const& point, SYMBOL_TYPE sym, std::string const& label, double dotRatio, double pLineRatio, double sLineRatio) -> std::vector<CalChart::DrawCommand>
    {
        return CalChart::append(
            CalChart::append(
                CreatePointCircle(sym, dotRatio),
                CreatePointCross(sym, dotRatio, pLineRatio, sLineRatio)),
            CreatePointLabel(point, label, dotRatio));
    }
}

auto Point::GetDrawCommands(unsigned ref, std::string const& label, double dotRatio, double pLineRatio, double sLineRatio) const -> std::vector<CalChart::DrawCommand>
{
    return CreatePoint(*this, GetSymbol(), label, dotRatio, pLineRatio, sLineRatio) + GetPos(ref);
}

// Test Suite stuff
struct Point_values {
    std::bitset<Point::kTotalBits> mFlags{};
    SYMBOL_TYPE mSym{ SYMBOL_PLAIN };
    Coord mPos{};
    std::array<Coord, Point::kNumRefPoints> mRef{};
    bool GetFlip{};
    bool Visable{};
};

auto Check_Point(Point const& underTest, Point_values const& values) -> bool
{
    return (underTest.mRef == values.mRef) && (underTest.mFlags == values.mFlags) && (underTest.mSym == values.mSym) && (underTest.mPos == values.mPos) && (underTest.GetFlip() == values.GetFlip) && (underTest.LabelIsVisible() == values.Visable);
}

void Point_UnitTests()
{
    // test some defaults:
    Point_values values{};
    values.mSym = SYMBOL_PLAIN;
    values.Visable = true;

    // test defaults
    Point underTest;
    assert(Check_Point(underTest, values));

    // test flip
    underTest.Flip(false);
    assert(Check_Point(underTest, values));

    values.mFlags = 1;
    values.GetFlip = true;
    underTest.Flip(true);
    assert(Check_Point(underTest, values));

    values.mFlags = 0;
    values.GetFlip = false;
    underTest.Flip(false);
    assert(Check_Point(underTest, values));

    // test visability
    underTest.SetLabelVisibility(true);
    assert(Check_Point(underTest, values));

    values.mFlags = 2;
    values.Visable = false;
    underTest.SetLabelVisibility(false);
    assert(Check_Point(underTest, values));

    values.mFlags = 0;
    values.Visable = true;
    underTest.SetLabelVisibility(true);
    assert(Check_Point(underTest, values));
}
}
