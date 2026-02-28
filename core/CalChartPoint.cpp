/*
 * CalChartPoint.cpp
 * Definition for the point classes
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

#include "CalChartPoint.h"
#include "CalChartConfiguration.h"
#include "CalChartConstants.h"
#include "CalChartFileFormat.h"

#include <cassert>
#include <cmath>
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
    pos.x = static_cast<Coord::units>(reader.Get<int16_t>());
    pos.y = static_cast<Coord::units>(reader.Get<int16_t>());
    return pos;
}

auto WritePositionData(Coord pos)
{
    auto result = std::vector<std::byte>{};
    Parser::Append(result, static_cast<int16_t>(pos.x));
    Parser::Append(result, static_cast<int16_t>(pos.y));
    return result;
}

auto ReadPositionJSON(nlohmann::json const& json)
{
    auto readUnit = [](nlohmann::json const& value) {
        if (!value.is_number()) {
            throw CC_FileException("bad Point JSON: coordinate value must be numeric");
        }
        return CalChart::RoundToCoordUnits(value.get<double>());
    };

    if (!json.is_array() || json.size() != 2) {
        throw CC_FileException("bad Point JSON: position must be [x, y]");
    }
    return Coord{ readUnit(json.at(0)), readUnit(json.at(1)) };
}

auto ReadPositionFromRootJSON(nlohmann::json const& json)
{
    if (json.contains("pos")) {
        return ReadPositionJSON(json.at("pos"));
    }
    throw CC_FileException("bad Point JSON: missing pos");
}

auto ReadPointSymbolJSON(nlohmann::json const& json)
{
    if (!json.is_string()) {
        throw CC_FileException("bad Point JSON: symbol must be a string");
    }
    auto const symbolName = json.get<std::string>();
    auto const symbol = GetSymbolForName(symbolName);
    if (symbol >= MAX_NUM_SYMBOLS) {
        throw CC_FileException("bad Point JSON: invalid symbol name '" + symbolName + "'");
    }
    return symbol;
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

Point::Point(nlohmann::json const& json)
    : mSym(SYMBOL_PLAIN)
{
    try {
        if (!json.is_object()) {
            throw CC_FileException("bad Point JSON: root must be an object");
        }

        mPos = ReadPositionFromRootJSON(json);
        mRef.fill(mPos);

        if (json.contains("ref")) {
            auto const& refs = json.at("ref");
            if (!refs.is_array() || refs.size() > kNumRefPoints) {
                throw CC_FileException("bad Point JSON: ref must be an array of up to 3 points");
            }
            for (auto i = 0U; i < refs.size(); ++i) {
                auto const& oneRef = refs.at(i);
                mRef.at(i) = ReadPositionJSON(oneRef);
            }
        }

        mSym = ReadPointSymbolJSON(json.at("symbol"));
        if (json.contains("flip")) {
            mFlags.set(kPointLabelFlipped, json.at("flip").get<bool>());
        }
        if (json.contains("label_invisible")) {
            mFlags.set(kLabelIsInvisible, json.at("label_invisible").get<bool>());
        }
    } catch (nlohmann::json::exception const& e) {
        throw CC_FileException(std::string("bad Point JSON: ") + e.what());
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

auto Point::toJSON() const -> nlohmann::json
{
    auto ref = nlohmann::json::array();
    // find the last ref point that is different from the main point, and only serialize up to that one
    for (auto i = kNumRefPoints; i > 0; --i) {
        if (GetPos(i) != GetPos(0)) {
            for (auto j = 1; j <= i; ++j) {
                ref.push_back(nlohmann::json::array({ GetPos(j).x, GetPos(j).y }));
            }
            break;
        }
    }

    auto const pos = GetPos();
    auto result = nlohmann::json{ { "pos", nlohmann::json::array({ pos.x, pos.y }) },
        { "symbol", GetNameForSymbol(GetSymbol()) } };
    if (!ref.empty()) {
        result["ref"] = ref;
    }
    if (GetFlip()) {
        result["flip"] = true;
    }
    if (!LabelIsVisible()) {
        result["label_invisible"] = true;
    }
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
    auto CreatePointCircle(CalChart::SYMBOL_TYPE symbol, double dotRatio) -> std::vector<Draw::DrawCommand>
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

        return { Draw::Circle({ 0, 0 }, circ_r, filled) };
    }

    auto CreatePointCross(CalChart::SYMBOL_TYPE symbol, double dotRatio, double pLineRatio, double sLineRatio)
        -> std::vector<Draw::DrawCommand>
    {
        auto const plineoff = static_cast<Coord::units>(CalChart::Float2CoordUnits(dotRatio * pLineRatio) / 2.0);
        auto const slineoff = static_cast<Coord::units>(CalChart::Float2CoordUnits(dotRatio * sLineRatio) / 2.0);

        auto drawCmds = std::vector<Draw::DrawCommand>{};
        switch (symbol) {
        case CalChart::SYMBOL_SL:
        case CalChart::SYMBOL_X:
            drawCmds.push_back(Draw::Line{ -plineoff, plineoff, plineoff, -plineoff });
            break;
        case CalChart::SYMBOL_SOLSL:
        case CalChart::SYMBOL_SOLX:
            drawCmds.push_back(Draw::Line{ -slineoff, slineoff, slineoff, -slineoff });
            break;
        default:
            break;
        }
        switch (symbol) {
        case CalChart::SYMBOL_BKSL:
        case CalChart::SYMBOL_X:
            drawCmds.push_back(Draw::Line{ -plineoff, -plineoff, plineoff, plineoff });
            break;
        case CalChart::SYMBOL_SOLBKSL:
        case CalChart::SYMBOL_SOLX:
            drawCmds.push_back(Draw::Line{ -slineoff, -slineoff, slineoff, slineoff });
            break;
        default:
            break;
        }
        return drawCmds;
    }

    auto CreatePointLabel(CalChart::Point const& point, std::string const& label, double dotRatio)
        -> std::vector<Draw::DrawCommand>
    {
        if (!point.LabelIsVisible()) {
            return {};
        }
        auto const circ_r = CalChart::Float2CoordUnits(dotRatio) / 2.0;
        auto anchor = Draw::Text::TextAnchor::Bottom;
        anchor = anchor | (point.GetFlip() ? Draw::Text::TextAnchor::Left : Draw::Text::TextAnchor::Right);
        return { Draw::Text(-CalChart::Coord(0, circ_r), label, anchor) };
    }

    auto CreatePoint(SYMBOL_TYPE sym, double dotRatio, double pLineRatio, double sLineRatio)
        -> std::vector<Draw::DrawCommand>
    {
        return CalChart::append(
            CreatePointCircle(sym, dotRatio), CreatePointCross(sym, dotRatio, pLineRatio, sLineRatio));
    }

    auto CreatePoint(CalChart::Point const& point, SYMBOL_TYPE sym, std::string const& label, double dotRatio,
        double pLineRatio, double sLineRatio) -> std::vector<Draw::DrawCommand>
    {
        return CalChart::append(
            CreatePoint(sym, dotRatio, pLineRatio, sLineRatio), CreatePointLabel(point, label, dotRatio));
    }
}

auto Point::GetDrawCommands(unsigned ref, std::string const& label, double dotRatio, double pLineRatio,
    double sLineRatio) const -> std::vector<Draw::DrawCommand>
{
    return CreatePoint(*this, GetSymbol(), label, dotRatio, pLineRatio, sLineRatio) + GetPos(ref);
}

auto Point::GetDrawCommands(unsigned ref, std::string const& label, Configuration const& config) const
    -> std::vector<Draw::DrawCommand>
{
    return GetDrawCommands(ref, label, config.Get_DotRatio(), config.Get_PLineRatio(), config.Get_SLineRatio());
}

auto Point::GetDrawCommands(std::string const& label, Configuration const& config) const
    -> std::vector<Draw::DrawCommand>
{
    return GetDrawCommands(label, config.Get_DotRatio(), config.Get_PLineRatio(), config.Get_SLineRatio());
}

auto Point::GetDrawCommands(double dotRatio, double pLineRatio, double sLineRatio) const
    -> std::vector<Draw::DrawCommand>
{
    return CreatePoint(GetSymbol(), dotRatio, pLineRatio, sLineRatio) + GetPos(0);
}

auto Point::GetDrawCommands(Configuration const& config) const -> std::vector<Draw::DrawCommand>
{
    return GetDrawCommands(config.Get_DotRatio(), config.Get_PLineRatio(), config.Get_SLineRatio());
}

}
