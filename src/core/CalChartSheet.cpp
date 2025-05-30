/*
 * CalChartSheet.cpp
 * Defintion for calchart sheet class
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

#define _LIBCPP_ENABLE_EXPERIMENTAL 1

#include "CalChartSheet.h"
#include "CalChartConfiguration.h"
#include "CalChartFileFormat.h"
#include "CalChartRanges.h"
#include "CalChartShow.h"
#include "viewer_translate.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>

namespace CalChart {

std::array<std::string, MAX_NUM_SYMBOLS> const contnames = {
    "Plain", "Sol", "Bksl", "Sl", "X", "Solbksl", "Solsl", "Solx"
};

std::array<std::string, MAX_NUM_SYMBOLS> const long_contnames = {
    "Plain", "Solid", "Backslash", "Slash", "Crossed", "Solid Backslash", "Solid Slash", "Solid Crossed"
};

Sheet::Sheet(size_t numPoints)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mBeats(1)
    , mPoints(numPoints)
{
}

Sheet::Sheet(size_t numPoints, std::string name)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mBeats(1)
    , mPoints(numPoints)
    , mName(std::move(name))
{
}

namespace {
    auto are_equal_helper(std::string const& a, std::string const& b) -> bool
    {
        auto p = std::mismatch(a.begin(), a.end(), b.begin(), [](char c1, char c2) {
            return std::tolower(c1) == std::tolower(c2);
        });
        return (p.first == a.end() && p.second == b.end());
    }

    auto are_equal(std::string const& a, std::string const& b) -> bool
    {
        return a.size() <= b.size() ? are_equal_helper(a, b) : are_equal_helper(b, a);
    }

}

auto GetSymbolForName(std::string const& name) -> SYMBOL_TYPE
{
    for (auto [index, symbolName] : CalChart::Ranges::enumerate_view(contnames)) {
        if (are_equal(name, symbolName)) {
            return static_cast<SYMBOL_TYPE>(index);
        }
    }
    // what do we do here?  give larger one for now...
    // This should probably throw
    return MAX_NUM_SYMBOLS;
}

auto GetNameForSymbol(SYMBOL_TYPE which) -> std::string
{
    if (which > MAX_NUM_SYMBOLS) {
        return "";
    }
    return contnames[which];
}

auto GetLongNameForSymbol(SYMBOL_TYPE which) -> std::string
{
    if (which > MAX_NUM_SYMBOLS) {
        return "";
    }
    return long_contnames[which];
}

// -=-=-=-=-=- LEGACY CODE -=-=-=-=-=-
// Recommend that you don't touch this unless you know what you are doing.
namespace {
    void
    CheckInconsistancy(SYMBOL_TYPE symbol, uint8_t cont_index,
        std::map<SYMBOL_TYPE, uint8_t>& continity_for_symbol,
        std::map<uint8_t, SYMBOL_TYPE>& symbol_for_continuity,
        std::string const& sheet_name, uint32_t pointNum)
    {
        // need to check for symbol inconsistency here.
        if (continity_for_symbol.count(symbol) == 0) {
            // we haven't seen this symbol->cont_index yet
            continity_for_symbol[symbol] = cont_index;
        } else {
            if (continity_for_symbol[symbol] != cont_index) {
                std::stringstream buf;
                buf << "Error, symbol inconsistency on sheet \"" << sheet_name << "\".\n";
                buf << "Symbol " << GetNameForSymbol(symbol)
                    << " previously used continuity "
                    << (uint32_t)continity_for_symbol[symbol] << " but point " << pointNum
                    << " on uses continuity " << (uint32_t)cont_index
                    << ", which is used by symbol "
                    << GetNameForSymbol(symbol_for_continuity[cont_index]) << ".\n";
                buf << "Try opening this file on CalChart v3.3.5 or earlier.\n";
                throw CC_FileException(buf.str());
            }
        }
        if (symbol_for_continuity.count(cont_index) == 0) {
            symbol_for_continuity[cont_index] = symbol;
        } else {
            if (symbol_for_continuity[cont_index] != symbol) {
                std::stringstream buf;
                buf << "Error, symbol inconsistency on sheet \"" << sheet_name << "\".\n";
                buf << "Continuity index " << (uint32_t)cont_index
                    << " previously used symbol "
                    << GetNameForSymbol(symbol_for_continuity[cont_index])
                    << "  but point " << pointNum << " on uses symbol "
                    << GetNameForSymbol(symbol) << ".\n";
                buf << "Try opening this file on CalChart v3.3.5 or earlier.\n";
                throw CC_FileException(buf.str());
            }
        }
    }
}

// Constructor for shows 3.3 and ealier.
// intentionally a reference to Reader.
Sheet::Sheet(Version_3_3_and_earlier, size_t numPoints, Reader& reader, ParseErrorHandlers const* correction)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mPoints(numPoints)
{
    // Read in sheet name
    // <INGL_NAME><size><string + 1>
    auto data = reader.ReadCheckIDandFillData(INGL_NAME);
    mName = (const char*)&data[0];

    // read in the duration:
    // <INGL_DURA><4><duration>
    auto chunk = reader.ReadCheckIDandSize(INGL_DURA);
    mBeats = chunk;

    // Point positions
    // <INGL_DURA><size><data>
    data = reader.ReadCheckIDandFillData(INGL_POS);
    if (data.size() != size_t(mPoints.size() * 4)) {
        throw CC_FileException("bad POS chunk");
    }
    {
        auto reader = CalChart::Reader({ data.data(), data.size() });
        for (unsigned i = 0; i < mPoints.size(); ++i) {
            auto x = reader.Get<int16_t>();
            auto y = reader.Get<int16_t>();
            auto c = Coord(x, y);
            for (unsigned j = 0; j <= Point::kNumRefPoints; j++) {
                mPoints[i].SetPos(c, j);
            }
        }
    }

    uint32_t name = reader.Get<uint32_t>();
    // read all the reference points
    while (INGL_REFP == name) {
        auto size = reader.Get<uint32_t>();
        if (size != mPoints.size() * 4 + 2) {
            throw CC_FileException("Bad REFP chunk");
        }
        auto ref = reader.Get<uint16_t>();
        for (unsigned i = 0; i < mPoints.size(); i++) {
            auto x = reader.Get<int16_t>();
            auto y = reader.Get<int16_t>();
            auto c = Coord(x, y);
            mPoints[i].SetPos(c, ref);
        }
        name = reader.Get<uint32_t>();
    }
    // Point symbols
    while (INGL_SYMB == name) {
        std::vector<uint8_t> data = reader.GetVector<uint8_t>();
        if (data.size() != mPoints.size()) {
            throw CC_FileException("Bad SYMB chunk");
        }
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); i++) {
            SetSymbol(i, (SYMBOL_TYPE)(*(d++)));
        }
        name = reader.Get<uint32_t>();
    }
    std::map<SYMBOL_TYPE, uint8_t> continity_for_symbol;
    std::map<uint8_t, SYMBOL_TYPE> symbol_for_continuity;
    bool has_type = false;
    // Point continuity types
    while (INGL_TYPE == name) {
        has_type = true;
        std::vector<uint8_t> data = reader.GetVector<uint8_t>();
        if (data.size() != mPoints.size()) {
            throw CC_FileException("Bad TYPE chunk");
        }
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); i++) {
            CheckInconsistancy(GetSymbol(i), *(d++), continity_for_symbol,
                symbol_for_continuity, mName, i);
        }
        name = reader.Get<uint32_t>();
    }
    // because older calchart files may omit the continuity index, need to check
    // if it isn't used
    if (!has_type) {
        // when a point doesn't have a cont_index, it is assumed to be 0
        for (unsigned i = 0; i < mPoints.size(); i++) {
            CheckInconsistancy(GetSymbol(i), 0, continity_for_symbol,
                symbol_for_continuity, mName, i);
        }
    }
    // Point labels (left or right)
    while (INGL_LABL == name) {
        std::vector<uint8_t> data = reader.GetVector<uint8_t>();
        if (data.size() != mPoints.size()) {
            throw CC_FileException("Bad SYMB chunk");
        }
        uint8_t* d = &data[0];
        for (unsigned i = 0; i < mPoints.size(); i++) {
            if (*(d++)) {
                mPoints.at(i).Flip();
            }
        }
        name = reader.Get<uint32_t>();
    }
    // Continuity text
    while (INGL_CONT == name) {
        std::vector<uint8_t> data = reader.GetVector<uint8_t>();
        if (data.size() < 3) // one byte num + two nils minimum
        {
            throw CC_FileException("Bad cont chunk");
        }
        auto d = (char const*)&data[0];
        if (d[data.size() - 1] != '\0') {
            throw CC_FileException("Bad cont chunk");
        }

        auto text = d + 1;
        size_t num = strlen(text);
        if (data.size() < num + 3) // check for room for text string
        {
            throw CC_FileException("Bad cont chunk");
        }
        std::string namestr(text);
        text = d + 2 + strlen(text);

        auto symbol_index = GetSymbolForName(namestr);
        if (symbol_index == MAX_NUM_SYMBOLS) {
            throw CC_FileException("No viable symbol for name");
        }
        if (continity_for_symbol.count(symbol_index)) {
            // some point is using this symbol, check to see if it points to the same
            // continuity
            if (continity_for_symbol[symbol_index] != (*d)) {
                std::stringstream buf;
                buf << "Error, continuity inconsistency on sheet " << mName << "\n";
                buf << "Continuity index " << (uint32_t)(*d) << " is symbol "
                    << GetNameForSymbol(symbol_index)
                    << " but points using that symbol refer to continuity index "
                    << (uint32_t)continity_for_symbol[symbol_index] << "\n";
                throw CC_FileException(buf.str());
            }
        }
        std::string textstr(text);
        mAnimationContinuity.at(symbol_index) = Continuity{ textstr, correction };

        name = reader.Get<uint32_t>();
    }
}
// -=-=-=-=-=- LEGACY CODE</end> -=-=-=-=-=-

Sheet::Sheet(size_t numPoints, Reader reader, ParseErrorHandlers const* correction)
    : mAnimationContinuity(MAX_NUM_SYMBOLS)
    , mPoints(numPoints)
{
    // construct the parser handlers
    auto parse_INGL_NAME = [](Sheet* sheet, Reader reader) {
        auto str = reader.Get<std::string>();
        if (reader.size() != 0) {
            throw CC_FileException("Description the wrong size", INGL_NAME);
        }
        sheet->mName = str;
    };
    auto parse_INGL_DURA = [](Sheet* sheet, Reader reader) {
        if (reader.size() != 4) {
            throw CC_FileException("Incorrect size", INGL_DURA);
        }
        sheet->mBeats = reader.Get<uint32_t>();
    };
    auto parse_INGL_PNTS = [](Sheet* sheet, Reader reader) {
        for (auto i = 0u; i < sheet->mPoints.size(); ++i) {
            auto this_size = reader.Get<uint8_t>();
            if (this_size > reader.size()) {
                throw CC_FileException("Incorrect size", INGL_PNTS);
            }
            sheet->mPoints[i] = Point(reader.first(this_size));
            reader = reader.subspan(this_size);
        }
        if (reader.size() != 0) {
            throw CC_FileException("Incorrect size", INGL_PNTS);
        }
    };
    auto parse_INGL_ECNT = [correction](Sheet* sheet, Reader reader) {
        if (reader.size() < 2) // one byte num + 1 nil minimum
        {
            throw CC_FileException("Bad cont chunk", INGL_ECNT);
        }
        SYMBOL_TYPE symbol_index = static_cast<SYMBOL_TYPE>(reader.Get<uint8_t>());
        if (symbol_index >= MAX_NUM_SYMBOLS) {
            throw CC_FileException("No viable symbol for name", INGL_ECNT);
        }
        auto text = reader.Get<std::string>();
        sheet->mAnimationContinuity.at(symbol_index) = Continuity{ text, correction };
    };
    auto parse_INGL_CONT = [parse_INGL_ECNT](Sheet* sheet, Reader reader) {
        const std::map<uint32_t, std::function<void(Sheet*, Reader)>>
            parser = {
                { INGL_ECNT, parse_INGL_ECNT },
            };

        auto table = reader.ParseOutLabels();
        for (auto& i : table) {
            auto the_parser = parser.find(std::get<0>(i));
            if (the_parser != parser.end()) {
                the_parser->second(sheet, std::get<1>(i));
            }
        }
    };
    auto parse_INGL_EVCT = [](Sheet* sheet, Reader reader) {
        if (reader.size() < 1) // one byte for symbol
        {
            throw CC_FileException("Bad cont chunk", INGL_EVCT);
        }
        SYMBOL_TYPE symbol_index = static_cast<SYMBOL_TYPE>(reader.Get<uint8_t>());
        if (symbol_index >= MAX_NUM_SYMBOLS) {
            throw CC_FileException("No viable symbol for name", INGL_EVCT);
        }
        sheet->mAnimationContinuity.at(symbol_index) = Continuity{ reader };
    };
    auto parse_INGL_VCNT = [parse_INGL_EVCT](Sheet* sheet, Reader reader) {
        std::map<uint32_t, std::function<void(Sheet*, Reader)>> const
            parser
            = {
                  { INGL_EVCT, parse_INGL_EVCT },
              };

        auto table = reader.ParseOutLabels();
        for (auto& i : table) {
            auto the_parser = parser.find(std::get<0>(i));
            if (the_parser != parser.end()) {
                the_parser->second(sheet, std::get<1>(i));
            }
        }
    };
    auto parse_INGL_PCNT = [](Sheet* sheet, Reader reader) {
        auto print_name = reader.Get<std::string>();
        auto print_cont = reader.Get<std::string>();
        if (reader.size() != 0) {
            throw CC_FileException("Bad Print cont chunk", INGL_PCNT);
        }
        sheet->mPrintableContinuity = PrintContinuity(print_name, print_cont);
    };
    auto parse_INGL_BACK = [](Sheet* sheet, Reader reader) {
        auto num = reader.Get<int32_t>();
        while (num--) {
            auto [image, new_reader] = CreateImageInfo(reader);
            sheet->mBackgroundImages.push_back(image);
            reader = new_reader;
        }
        if (reader.size() != 0) {
            throw CC_FileException("Bad Background chunk", INGL_BACK);
        }
    };
    auto parse_INGL_CURV = [](Sheet* sheet, Reader reader) {
        auto num = reader.Get<int32_t>();
        while (num--) {
            auto [curve, new_reader] = CreateCurve(reader);
            sheet->mCurves.push_back(std::pair<Curve, std::vector<MarcherIndex>>{ curve, {} });
            reader = new_reader;
        }
        if (reader.size() != 0) {
            throw CC_FileException("Bad curve chunk", INGL_BACK);
        }
    };
    auto parse_INGL_CASS = [](Sheet* sheet, Reader reader) {
        auto num = reader.Get<int32_t>();
        for (auto which = 0; which < num; ++which) {
            sheet->mCurves.at(which).second = reader.GetVector<uint32_t>();
        }
        if (reader.size() != 0) {
            throw CC_FileException("Bad Curve Assignment chunk", INGL_BACK);
        }
    };

    std::map<uint32_t, std::function<void(Sheet*, Reader)>> const
        parser
        = {
              { INGL_NAME, parse_INGL_NAME },
              { INGL_DURA, parse_INGL_DURA },
              { INGL_PNTS, parse_INGL_PNTS },
              { INGL_CONT, parse_INGL_CONT },
              { INGL_VCNT, parse_INGL_VCNT },
              { INGL_PCNT, parse_INGL_PCNT },
              { INGL_BACK, parse_INGL_BACK },
              { INGL_CURV, parse_INGL_CURV },
              { INGL_CASS, parse_INGL_CASS },
          };

    auto table = reader.ParseOutLabels();
    for (auto& i : table) {
        auto the_parser = parser.find(std::get<0>(i));
        if (the_parser != parser.end()) {
            the_parser->second(this, std::get<1>(i));
        }
    }
    RepositionCurveMarchers();
}

auto Sheet::SerializeAllPoints() const -> std::vector<std::byte>
{
    // for each of the points, serialize them.  Don't need to wrap in block
    // because it's not specified that way
    std::vector<std::byte> result;
    for (auto&& i : mPoints) {
        Parser::Append(result, i.Serialize());
    }
    return result;
}

auto Sheet::SerializeContinuityData() const -> std::vector<std::byte>
{
    // for each continuity in use, serialize them.
    std::vector<std::byte> result;
    for (auto& current_symbol : k_symbols) {
        if (ContinuityInUse(current_symbol)) {
            std::vector<std::byte> continuity;
            Parser::Append(continuity, static_cast<uint8_t>(current_symbol));
            Parser::Append(continuity, mAnimationContinuity.at(current_symbol).Serialize());
            Parser::Append(result, Parser::Construct_block(INGL_EVCT, continuity));
        }
    }
    return result;
}

auto Sheet::SerializePrintContinuityData() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::AppendAndNullTerminate(
        result, mPrintableContinuity.GetPrintNumber());
    Parser::AppendAndNullTerminate(
        result, mPrintableContinuity.GetOriginalLine());
    return result;
}

auto Sheet::SerializeBackgroundImageInfo() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, static_cast<uint32_t>(mBackgroundImages.size()));
    for (auto&& i : mBackgroundImages) {
        Parser::Append(result, Serialize(i));
    }
    return result;
}

auto Sheet::SerializeCurves() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, static_cast<uint32_t>(mCurves.size()));
    for (auto&& [curve, marchers] : mCurves) {
        Parser::Append(result, curve.Serialize());
    }
    return result;
}

auto Sheet::SerializeCurveAssigments() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, static_cast<uint32_t>(mCurves.size()));
    for (auto&& [curve, marchers] : mCurves) {
        Parser::Append(result, static_cast<uint32_t>(marchers.size()));
        Parser::Append(result, marchers);
    }
    return result;
}

auto Sheet::SerializeSheetData() const -> std::vector<std::byte>
{
    // SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY,
    // PRINT_CONTINUITY ;

    std::vector<std::byte> result;
    // Write NAME
    std::vector<std::byte> tstring;
    Parser::AppendAndNullTerminate(tstring, GetName());
    Parser::Append(
        result, Parser::Construct_block(INGL_NAME, tstring));

    // Write DURATION
    Parser::Append(result, Parser::Construct_block(INGL_DURA, uint32_t{ GetBeats() }));

    // Write ALL_POINTS
    Parser::Append(result, Parser::Construct_block(INGL_PNTS, SerializeAllPoints()));

    // Write Continuity
    Parser::Append(result, Parser::Construct_block(INGL_VCNT, SerializeContinuityData()));

    // Write Continuity
    Parser::Append(result,
        Parser::Construct_block(
            INGL_PCNT, SerializePrintContinuityData()));

    // Write Background
    Parser::Append(result, Parser::Construct_block(INGL_BACK, SerializeBackgroundImageInfo()));

    // Write Curves
    Parser::Append(result, Parser::Construct_block(INGL_CURV, SerializeCurves()));
    Parser::Append(result, Parser::Construct_block(INGL_CASS, SerializeCurveAssigments()));

    return result;
}

// SHEET              = INGL_SHET , BigEndianInt32(DataTill_SHEET_END) ,
// SHEET_DATA , SHEET_END ;
// SHEET_DATA         = NAME , DURATION , ALL_POINTS , VCONTINUITY, [
// PRINT_CONTINUITY ] ;
// SHEET_END          = INGL_END , INGL_SHET ;
auto Sheet::SerializeSheet() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    Parser::Append(result, Parser::Construct_block(INGL_SHET, SerializeSheetData()));
    return result;
}

// Find point at certain coords
auto Sheet::FindMarcher(Coord where, Coord::units searchBound, unsigned ref) const -> std::optional<MarcherIndex>
{
    for (auto i : std::views::iota(0ul, mPoints.size())) {
        Coord c = GetMarcherPosition(i, ref);
        if (((where.x + searchBound) >= c.x) && ((where.x - searchBound) <= c.x) && ((where.y + searchBound) >= c.y) && ((where.y - searchBound) <= c.y)) {
            return i;
        }
    }
    return std::nullopt;
}

auto Sheet::FindCurveControlPoint(Coord where, Coord::units searchBound) const -> std::optional<std::tuple<size_t, size_t>>
{
    for (auto&& [whichCurve, curve] : CalChart::Ranges::enumerate_view(mCurves)) {
        auto&& points = curve.first.GetControlPoints();
        if (auto iter = std::find_if(points.begin(), points.end(), [where, searchBound](auto point) {
                return ((where.x + searchBound) >= point.x) && ((where.x - searchBound) <= point.x) && ((where.y + searchBound) >= point.y) && ((where.y - searchBound) <= point.y);
            });
            iter != points.end()) {
            return std::tuple<size_t, size_t>{ whichCurve, std::distance(points.begin(), iter) };
        }
    }
    return std::nullopt;
}

auto Sheet::FindCurve(Coord where, Coord::units searchBound) const -> std::optional<std::tuple<size_t, size_t, double>>
{
    for (auto&& [whichCurve, curve] : CalChart::Ranges::enumerate_view(mCurves)) {
        auto result = curve.first.LowerControlPointOnLine(where, searchBound);
        if (result.has_value()) {
            return std::tuple<size_t, size_t, double>{ whichCurve, std::get<0>(*result), std::get<1>(*result) };
        }
    }
    return std::nullopt;
}

auto Sheet::GetCurveAssignments() const -> std::vector<std::vector<MarcherIndex>>
{
    return CalChart::Ranges::ToVector<std::vector<MarcherIndex>>(mCurves | std::views::transform([](auto&& curve) { return curve.second; }));
}

void Sheet::SetCurveAssignment(std::vector<std::vector<MarcherIndex>> curveAssignments)
{
    for (auto&& [which, marchers] : CalChart::Ranges::enumerate_view(curveAssignments)) {
        mCurves.at(which).second = marchers;
    }
    RepositionCurveMarchers();
}

auto Sheet::GetCurveAssignmentsWithNewAssignments(size_t whichCurve, std::vector<MarcherIndex> whichMarchers) const -> std::vector<std::vector<MarcherIndex>>
{
    // first get all the marchers assigned to curves
    auto curveAssignments = GetCurveAssignments();
    for (auto& assignment : curveAssignments) {
        auto begin = assignment.begin();
        auto end = assignment.end();
        for (auto marcher : whichMarchers) {
            end = std::remove(begin, end, marcher);
        }
        assignment.erase(end, assignment.end());
    }
    // go through and remove and of the ones we're adding to curves
    curveAssignments.at(whichCurve) = whichMarchers;
    return curveAssignments;
}

void Sheet::UnassignMarchersFromAnyCurve(std::vector<MarcherIndex> marchers)
{
    for (auto marcher : marchers) {
        for (auto& [curve, marchers] : mCurves) {
            marchers.erase(std::remove(marchers.begin(), marchers.end(), marcher), marchers.end());
        }
    }
}

auto Sheet::MakeSelectPointsBySymbol(SYMBOL_TYPE i) const -> SelectionList
{
    SelectionList select;
    std::ranges::for_each(
        std::views::iota(0ul, mPoints.size())
            | std::views::filter([&](MarcherIndex j) { return GetSymbol(j) == i; }),
        [&](MarcherIndex j) { select.insert(j); });
    return select;
}

auto Sheet::NewNumPointsPositions(int num, int columns, Coord new_march_position) const -> std::vector<Point>
{
    std::vector<Point> newpts(mPoints.begin(), mPoints.begin() + std::min<size_t>(mPoints.size(), num));
    auto c = new_march_position;
    auto col = 0;
    auto num_left = num - newpts.size();
    while (num_left--) {
        newpts.emplace_back(c);
        ++col;
        c.x += Int2CoordUnits(2);
        if (col >= columns) {
            c.x = new_march_position.x;
            c.y += Int2CoordUnits(2);
            col = 0;
        }
    }
    return newpts;
}

void Sheet::DeletePoints(SelectionList const& sl)
{
    UnassignMarchersFromAnyCurve({ sl.begin(), sl.end() });
    for (auto iter = sl.rbegin(); iter != sl.rend(); ++iter) {
        mPoints.erase(mPoints.begin() + *iter);
    }
    RepositionCurveMarchers();
}

// modifying curves means reseting all the points
void Sheet::AddCurve(Curve const& curve, size_t index)
{
    mCurves.insert(mCurves.begin() + index, std::pair<Curve, std::vector<MarcherIndex>>{ curve, {} });
}

void Sheet::RemoveCurve(size_t index)
{
    mCurves.erase(mCurves.cbegin() + index);
}

void Sheet::ReplaceCurve(Curve const& curve, size_t index)
{
    mCurves.at(index).first = curve;
    RepositionCurveMarchers();
}

auto Sheet::GetCurve(size_t index) const -> Curve { return mCurves.at(index).first; }
auto Sheet::GetNumberCurves() const -> size_t { return mCurves.size(); }

auto Sheet::RemapPoints(std::vector<MarcherIndex> const& table) const -> std::vector<Point>
{
    if (mPoints.size() != table.size()) {
        throw std::runtime_error("wrong size for Relabel");
    }
    std::vector<Point> newpts(mPoints.size());
    for (size_t i = 0; i < newpts.size(); i++) {
        newpts.at(i) = mPoints.at(table.at(i));
    }
    return newpts;
}

void Sheet::SetContinuity(SYMBOL_TYPE which, Continuity const& new_cont)
{
    mAnimationContinuity.at(which) = new_cont;
}

auto Sheet::ContinuityInUse(SYMBOL_TYPE idx) const -> bool
{
    auto points = std::vector<int>(mPoints.size());
    std::iota(points.begin(), points.end(), 0);
    // is any point using this symbol?
    for (auto& point : points) {
        if (GetSymbol(point) == idx) {
            return true;
        }
    }
    return false;
}

auto Sheet::GetName() const -> std::string { return mName; }

void Sheet::SetName(std::string const& newname) { mName = newname; }

auto Sheet::GetPrintNumber() const -> std::string
{
    return mPrintableContinuity.GetPrintNumber();
}

std::string Sheet::GetRawPrintContinuity() const
{
    return mPrintableContinuity.GetOriginalLine();
}

// Get position of point
auto Sheet::GetMarcherPosition(MarcherIndex i, unsigned ref) const -> Coord
{
    return mPoints[i].GetPos(ref);
}

// Set position of point
void Sheet::SetPosition(Coord val, MarcherIndex i, unsigned ref)
{
    SetPositionHelper(val, i, ref);
    if (ref == 0) {
        UnassignMarchersFromAnyCurve({ i });
        RepositionCurveMarchers();
    }
}

void Sheet::SetPositionHelper(Coord val, MarcherIndex i, unsigned ref)
{
    if (ref == 0) {
        for (auto j = 1; j <= Point::kNumRefPoints; j++) {
            if (mPoints[i].GetPos(j) == mPoints[i].GetPos(0)) {
                mPoints[i].SetPos(val, j);
            }
        }
        mPoints[i].SetPos(val);
    } else {
        mPoints[i].SetPos(val, ref);
    }
}

void Sheet::SetPrintableContinuity(std::string const& name, std::string const& lines)
{
    mPrintableContinuity = PrintContinuity(name, lines);
}

auto Sheet::GetPrintableContinuity() const -> Textline_list
{
    return mPrintableContinuity.GetChunks();
}

auto Sheet::GetMarcher(MarcherIndex i) const -> Point { return mPoints[i]; }

void Sheet::SetSymbol(MarcherIndex i, SYMBOL_TYPE sym)
{
    mPoints[i].SetSymbol(sym);
}

void Sheet::SetMarcherFlip(MarcherIndex i, bool val)
{
    mPoints.at(i).Flip(val);
}

void Sheet::SetMarcherLabelVisibility(MarcherIndex i, bool isVisible)
{
    mPoints.at(i).SetLabelVisibility(isVisible);
}

auto Sheet::GetSymbols() const -> std::vector<SYMBOL_TYPE>
{
    std::vector<SYMBOL_TYPE> result;
    std::transform(mPoints.begin(), mPoints.end(), std::back_inserter(result), [](auto&& i) { return i.GetSymbol(); });
    return result;
}

auto Sheet::toOnlineViewerJSON(unsigned sheetNum, std::vector<std::string> dotLabels, std::map<std::string, std::vector<nlohmann::json>> const& movements) const -> nlohmann::json
{
    nlohmann::json j;
    // TODO; add printed continuities to viewer file manually for now
    auto boilerplate = std::vector<std::string>{
        std::string("(MANUAL) first continuity instruction goes here for SS") + std::to_string(sheetNum),
        std::string("(MANUAL) second instruction"),
        std::string("(MANUAL) third instruction..."),
    };

    std::set<std::string> uniqueDotTypes;
    std::map<std::string, std::string> labelToSymbol;
    std::map<std::string, std::vector<std::string>> continuities;

    for (unsigned i = 0; i < mPoints.size(); i++) {
        auto symbolName = ToOnlineViewer::symbolName(GetSymbol(i));
        uniqueDotTypes.insert(symbolName);
        labelToSymbol[dotLabels[i]] = symbolName;
        continuities[symbolName] = boilerplate;
    }

    j["dot_labels"] = labelToSymbol;
    j["dot_types"] = uniqueDotTypes;
    j["label"] = std::to_string(sheetNum);
    j["beats"] = static_cast<double>(mBeats);
    j["field_type"] = "college";
    j["continuities"] = continuities;

    j["movements"] = movements;
    return j;
}

namespace {
    // Returns a view adaptor that will transform a range of point indices to Draw point commands.
    auto TransformIndexToDrawCommands(CalChart::Sheet const& sheet, std::vector<std::string> const& labels, int ref, CalChart::Configuration const& config)
    {
        return std::views::transform([&sheet, ref, labels, &config](int i) {
            return sheet.GetMarcher(i).GetDrawCommands(ref, labels.at(i), config);
        })
            | std::ranges::views::join;
    }

    // Given a set and a size, return a range that has the numbers not in the set
    auto NegativeIntersection(CalChart::SelectionList const& set, int count)
    {
        return std::views::iota(0, count)
            | std::views::filter([set](int i) {
                  return !set.contains(i);
              });
    }

    // convention is that we have unselected
    auto GetMarcherColors(bool isGhost, bool isRef) -> std::array<Colors, 4>
    {
        if (isGhost) {
            return { Colors::GHOST_POINT, Colors::GHOST_POINT_HLIT, Colors::GHOST_POINT_TEXT, Colors::GHOST_POINT_HLIT_TEXT };
        }
        if (isRef) {
            return { Colors::REF_POINT, Colors::REF_POINT_HILIT, Colors::REF_POINT_TEXT, Colors::REF_POINT_HILIT_TEXT };
        }
        return { Colors::POINT, Colors::POINT_HILIT, Colors::POINT_TEXT, Colors::POINT_HILIT_TEXT };
    }

    auto GenerateSheetMarcherDrawCommands(
        CalChart::Configuration const& config,
        CalChart::SelectionList const& selection_list,
        std::vector<std::string> const& labels,
        CalChart::Sheet const& sheet,
        int ref,
        std::array<Colors, 4> color) -> std::vector<CalChart::Draw::DrawCommand>
    {

        return {
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(std::get<0>(color)),
                CalChart::Draw::withTextForeground(
                    config.Get_CalChartBrushAndPen(std::get<2>(color)),
                    NegativeIntersection(selection_list, labels.size())
                        | TransformIndexToDrawCommands(sheet, labels, ref, config))),
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(std::get<1>(color)),
                CalChart::Draw::withTextForeground(
                    config.Get_CalChartBrushAndPen(std::get<3>(color)),
                    selection_list
                        | TransformIndexToDrawCommands(sheet, labels, ref, config))),
        };
    }

    auto GenerateCurvePoints(std::vector<CalChart::Coord> const& points, Coord::units boxSize) -> std::vector<CalChart::Draw::DrawCommand>
    {
        return CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(points | std::views::transform([boxSize](auto&& point) {
            return CalChart::Draw::Rectangle(point - Coord(boxSize, boxSize) / 2, Coord(boxSize, boxSize));
        }));
    }

    auto GenerateCurve(CalChart::Configuration const& config, CalChart::Curve const& curve, int which) -> std::vector<Draw::DrawCommand>
    {
        auto boxSize = CalChart::Float2CoordUnits(config.Get_ControlPointRatio());
        auto points = curve.GetControlPoints();
        auto drawCmds = std::vector<Draw::DrawCommand>{
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(CalChart::Colors::SHEET_CURVE),
                curve.GetCC_DrawCommand()),
            CalChart::Draw::withBrushAndPen(
                config.Get_CalChartBrushAndPen(CalChart::Colors::SHEET_CURVE_CONTROL_POINT),
                GenerateCurvePoints(points, boxSize)),
        };
        if (points.size()) {
            drawCmds.push_back(CalChart::Draw::Text(points.front(), std::string("C") + std::to_string(which)));
        }
        return drawCmds;
    }

}

auto Sheet::GenerateGhostElements(CalChart::Configuration const& config, SelectionList const& selected, std::vector<std::string> const& marcherLabels) const -> std::vector<CalChart::Draw::DrawCommand>
{
    return GenerateSheetMarcherDrawCommands(config, selected, marcherLabels, *this, 0, GetMarcherColors(true, false));
}

auto Sheet::GenerateSheetElements(CalChart::Configuration const& config, SelectionList const& selected, std::vector<std::string> const& marcherLabels, int referencePoint) const -> std::vector<CalChart::Draw::DrawCommand>
{
    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{};
    if (referencePoint > 0) {
        // if we are editing a ref point other than 0, draw the 0 one in a different color.
        CalChart::append(drawCmds, GenerateSheetMarcherDrawCommands(config, selected, marcherLabels, *this, 0, GetMarcherColors(false, true)));
    }
    CalChart::append(drawCmds, GenerateSheetMarcherDrawCommands(config, selected, marcherLabels, *this, referencePoint, GetMarcherColors(false, false)));

    for (auto&& [which, curve] : CalChart::Ranges::enumerate_view(mCurves)) {
        CalChart::append(drawCmds, GenerateCurve(config, curve.first, which));
    }
    return drawCmds;
}

void Sheet::SetMarchers(std::vector<Point> const& points) { mPoints = points; }

void Sheet::AddBackgroundImage(ImageInfo const& image, size_t where)
{
    auto insert_point = mBackgroundImages.begin() + std::min(where, mBackgroundImages.size());
    mBackgroundImages.insert(insert_point, image);
}

void Sheet::RemoveBackgroundImage(size_t which)
{
    if (which < mBackgroundImages.size()) {
        mBackgroundImages.erase(mBackgroundImages.begin() + which);
    }
}

void Sheet::MoveBackgroundImage(size_t which, int left, int top, int scaled_width, int scaled_height)
{
    if (which < mBackgroundImages.size()) {
        mBackgroundImages.at(which).left = left;
        mBackgroundImages.at(which).top = top;
        mBackgroundImages.at(which).scaledWidth = scaled_width;
        mBackgroundImages.at(which).scaledHeight = scaled_height;
    }
}

namespace {
    // Return a bounding box of the show of where the marchers are.  If they are
    // outside the show, we don't see them.
    // can be done better with algorithms
    auto GetMarcherBoundingBox(std::vector<CalChart::Point> const& pts) -> std::pair<CalChart::Coord, CalChart::Coord>
    {
        CalChart::Coord bounding_box_upper_left{ 10000, 10000 };
        CalChart::Coord bounding_box_low_right{ -10000, -10000 };

        for (auto& i : pts) {
            auto position = i.GetPos();
            bounding_box_upper_left = CalChart::Coord(std::min(bounding_box_upper_left.x, position.x), std::min(bounding_box_upper_left.y, position.y));
            bounding_box_low_right = CalChart::Coord(std::max(bounding_box_low_right.x, position.x), std::max(bounding_box_low_right.y, position.y));
        }

        return { bounding_box_upper_left, bounding_box_low_right };
    }
}

// we want sheets to print in landscape when the width exceeds the height
auto Sheet::ShouldPrintLandscape() const -> bool
{
    auto boundingBox = GetMarcherBoundingBox(GetAllMarchers());
    return (boundingBox.second.x - boundingBox.first.x) > CalChart::Int2CoordUnits(CalChart::kFieldStepSizeNorthSouth[0]);
}

void Sheet::RepositionCurveMarchers()
{
    for (auto&& [curve, marchers] : mCurves) {
        for (auto&& [where, marcher] : CalChart::Ranges::zip_view(curve.GetPointsOnLine(marchers.size()), marchers)) {
            SetPositionHelper(where, marcher);
        }
    }
}

}
