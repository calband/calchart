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

Point::Point(Coord const& p)
    : mSym(SYMBOL_PLAIN)
    , mPos(p)
{
    mRef.fill(mPos);
}

// EACH_POINT_DATA    = BigEndianInt8(Size_rest_of_EACH_POINT_DATA) ,
// POSITION_DATA , REF_POSITION_DATA , POINT_SYMBOL_DATA , POINT_LABEL_FLIP ;
// POSITION_DATA      = BigEndianInt16( x ) , BigEndianInt16( y ) ;
// REF_POSITION_DATA  = BigEndianInt8( num ref pts ) , { BigEndianInt8( which
// reference point ) , BigEndianInt16( x ) , BigEndianInt16( y ) }* ;
// POINT_SYMBOL_DATA  = BigEndianInt8( which symbol type ) ;
// POINT_LABEL_FLIP_DATA = BigEndianInt8( label flipped ) ;
Point::Point(std::vector<uint8_t> const& serialized_data)
    : mSym(SYMBOL_PLAIN)
{
    const uint8_t* d = &serialized_data[0];
    {
        mPos.x = get_big_word(d);
        d += 2;
        mPos.y = get_big_word(d);
        d += 2;
    }
    // set all the reference points
    for (unsigned j = 0; j < Point::kNumRefPoints; j++) {
        mRef[j] = mPos;
    }

    uint8_t num_ref = *d;
    ++d;
    for (auto i = 0; i < num_ref; ++i) {
        uint8_t ref = *d;
        ++d;
        mRef[ref - 1].x = get_big_word(d);
        d += 2;
        mRef[ref - 1].y = get_big_word(d);
        d += 2;
    }
    mSym = static_cast<SYMBOL_TYPE>(*d);
    ++d;
    mFlags.set(kPointLabelFlipped, (*d) > 0);
    ++d;
    if (static_cast<size_t>(std::distance(&serialized_data[0], d)) != serialized_data.size()) {
        throw CC_FileException("bad POS chunk");
    }
}

std::vector<uint8_t> Point::Serialize() const
{
    // how many reference points are we going to write?
    uint8_t num_ref_pts = 0;
    for (auto j = 1; j <= Point::kNumRefPoints; j++) {
        if (GetPos(j) != GetPos(0)) {
            ++num_ref_pts;
        }
    }
    std::vector<uint8_t> result;
    // Point positions
    // Write block size

    // Write POSITION
    Parser::Append(result, uint16_t(GetPos().x));
    Parser::Append(result, uint16_t(GetPos().y));

    // Write REF_POS
    Parser::Append(result, uint8_t(num_ref_pts));
    if (num_ref_pts) {
        for (auto j = 1; j <= Point::kNumRefPoints; j++) {
            if (GetPos(j) != GetPos(0)) {
                Parser::Append(result, uint8_t(j));
                Parser::Append(result, uint16_t(GetPos(j).x));
                Parser::Append(result, uint16_t(GetPos(j).y));
            }
        }
    }

    // Write SYMBOL
    Parser::Append(result, uint8_t(GetSymbol()));

    // Point labels (left or right)
    Parser::Append(result, uint8_t(GetFlip()));
    result.insert(result.begin(), uint8_t(result.size()));

    return result;
}

void Point::Flip(bool val) { mFlags.set(kPointLabelFlipped, val); };

void Point::SetLabelVisibility(bool isVisible) { mFlags.set(kLabelIsInvisible, !isVisible); }

Coord Point::GetPos(unsigned ref) const
{
    if (ref == 0) {
        return mPos;
    }
    if (ref > kNumRefPoints) {
        throw std::range_error("GetPos() point out of range");
    }
    return mRef[ref - 1];
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
    mRef[ref - 1] = c;
}

void Point::SetSymbol(SYMBOL_TYPE s) { mSym = s; }

// Test Suite stuff
struct Point_values {
    std::bitset<Point::kTotalBits> mFlags;
    SYMBOL_TYPE mSym;
    Coord mPos;
    Coord mRef[Point::kNumRefPoints];
    bool GetFlip;
    bool Visable;
};

bool Check_Point(Point const& underTest, Point_values const& values)
{
    bool running_value = true;
    for (unsigned i = 0; i < Point::kNumRefPoints; ++i)
        running_value = running_value && (underTest.mRef[i] == values.mRef[i]);
    return running_value && (underTest.mFlags == values.mFlags) && (underTest.mSym == values.mSym) && (underTest.mPos == values.mPos) && (underTest.GetFlip() == values.GetFlip) && (underTest.LabelIsVisible() == values.Visable);
}

void Point_UnitTests()
{
    // test some defaults:
    Point_values values;
    values.mFlags = 0;
    values.mSym = SYMBOL_PLAIN;
    values.mPos = Coord();
    for (unsigned i = 0; i < Point::kNumRefPoints; ++i)
        values.mRef[i] = Coord();
    values.GetFlip = false;
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
