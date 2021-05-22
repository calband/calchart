#pragma once
/*
 * modes.h
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

#include "CalChartCoord.h"

#include <array>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include <boost/serialization/access.hpp>
#include <boost/serialization/array.hpp>

namespace CalChart {

static constexpr auto SPR_YARD_LEFT = 8;
static constexpr auto SPR_YARD_RIGHT = 4;
static constexpr auto SPR_YARD_ABOVE = 2;
static constexpr auto SPR_YARD_BELOW = 1;

static constexpr auto kYardTextValues = 53;
static constexpr auto kSprLineTextValues = 5;

static constexpr int kFieldStepSizeNorthSouth[2] = { 96, 160 };
static constexpr auto kFieldStepSizeEastWest = 84;
static constexpr int kFieldStepSizeSouthEdgeFromCenter[2] = { 48, 80 };
static constexpr auto kFieldStepSizeWestEdgeFromCenter = 42;
static constexpr auto kFieldStepWestHashFromWestSideline = 32;
static constexpr auto kFieldStepEastHashFromWestSideline = 52;

class ShowMode {
public:
    static ShowMode GetDefaultShowMode();

    using YardLinesInfo_t = std::array<std::string, kYardTextValues>;
    static YardLinesInfo_t GetDefaultYardLines();

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
    using ShowModeInfo_t = std::array<long, kShowModeValues>;

    static ShowMode CreateShowMode(ShowModeInfo_t const& values, YardLinesInfo_t const& yardlines);
    static ShowMode CreateShowMode(CalChart::Coord size, CalChart::Coord offset,
        CalChart::Coord border1, CalChart::Coord border2, unsigned short whash,
        unsigned short ehash, YardLinesInfo_t const& yardlines);

    static ShowMode CreateShowMode(std::vector<uint8_t> const&);
    std::vector<uint8_t> Serialize() const;

    ShowModeInfo_t GetShowModeInfo() const;

    auto Offset() const { return mOffset + mBorder1; }
    auto FieldOffset() const { return -mOffset; }
    auto Size() const { return mSize + mBorder1 + mBorder2; }
    auto FieldSize() const { return mSize; }
    auto MinPosition() const { return -(mOffset + mBorder1); }
    auto MaxPosition() const { return mSize - mOffset + mBorder2; }
    auto Border1() const { return mBorder1; }
    CalChart::Coord ClipPosition(const CalChart::Coord& pos) const;

    auto HashW() const { return mHashW; }
    auto HashE() const { return mHashE; }

    // Yard Lines
    YardLinesInfo_t const& Get_yard_text() const;

private:
    ShowMode(CalChart::Coord size,
        CalChart::Coord offset,
        CalChart::Coord border1,
        CalChart::Coord border2,
        unsigned short whash,
        unsigned short ehash,
        YardLinesInfo_t const& yardlines);

    CalChart::Coord mSize;
    CalChart::Coord mOffset;
    CalChart::Coord mBorder1;
    CalChart::Coord mBorder2;

    unsigned short mHashW, mHashE;

    YardLinesInfo_t mYardLines;

    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& mSize;
        ar& mOffset;
        ar& mBorder1;
        ar& mBorder2;
        ar& mHashW;
        ar& mHashE;
        ar& mYardLines;
    }
    friend bool operator==(ShowMode const& lhs, ShowMode const& rhs);
};

void ShowMode_UnitTests();

}
