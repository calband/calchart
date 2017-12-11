#pragma once
/*
 * cc_point.h
 * Definitions for the point classes
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

#include "cc_types.h"
#include "cc_coord.h"

#include <bitset>
#include <vector>

namespace CalChart {

class Point {
public:
    static constexpr auto kNumRefPoints = 3;
    Point();
    Point(const Coord& pos);

    Point(const std::vector<uint8_t>& serialized_data);
    std::vector<uint8_t> Serialize() const;

    auto GetFlip() const { return mFlags.test(kPointLabelFlipped); }
    void Flip(bool val = true);

    auto LabelIsVisible() const { return !mFlags.test(kLabelIsInvisible); }
    void SetLabelVisibility(bool isVisible);

    auto GetSymbol() const { return mSym; }
    void SetSymbol(SYMBOL_TYPE sym);

    // reference points 0 is the point, refs are [1, kNumRefPoints]
    Coord GetPos(unsigned ref = 0) const;
    void SetPos(const Coord& c, unsigned ref = 0);

private:
    enum { kPointLabelFlipped,
        kLabelIsInvisible,
        kTotalBits };

    std::bitset<kTotalBits> mFlags;
    // by having both a sym type and cont index, we can have several
    // points share the same symbol but have different continuities.
    SYMBOL_TYPE mSym;
    Coord mPos;
    Coord mRef[kNumRefPoints];

    friend struct Point_values;
    friend bool Check_Point(const Point&, const struct Point_values&);
};

void Point_UnitTests();
}
