#pragma once
/*
 * CalChartPoint.h
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

/**
 * CalChartPoint
 *  A point in CalChart is a marcher on the field. It maintains the reference points positions, and it's positions,
 *  as well as Symbol and some visualization information (like Flipped).
 *
 *  In general we should try to keep point simple, and instead have the visualization data and other concepts
 *  maintained in the larger container, such as instrument or name, which should be per show.  Symbol should
 *  probably be in per show, but as such the serialization code would make this difficult to restructure, so it will
 *  likely need to continue to be in Point.
 */

#include "CalChartConstants.h"
#include "CalChartCoord.h"
#include "CalChartTypes.h"

#include <array>
#include <bitset>
#include <vector>

namespace CalChart {

class Reader;

class Point {
public:
    static constexpr auto kNumRefPoints = 3;
    Point();
    Point(Coord const& pos);

    Point(Reader);
    std::vector<uint8_t> Serialize() const;

    auto GetFlip() const { return mFlags.test(kPointLabelFlipped); }
    void Flip(bool val = true);

    auto LabelIsVisible() const { return !mFlags.test(kLabelIsInvisible); }
    void SetLabelVisibility(bool isVisible);

    // reference points 0 is the point, refs are [1, kNumRefPoints]
    Coord GetPos(unsigned ref = 0) const;
    void SetPos(Coord c, unsigned ref = 0);

private:
    enum { kPointLabelFlipped,
        kLabelIsInvisible,
        kTotalBits };

    std::bitset<kTotalBits> mFlags;
    SYMBOL_TYPE mSym;
    Coord mPos;
    std::array<Coord, kNumRefPoints> mRef;

    auto GetSymbol() const { return mSym; }
    void SetSymbol(SYMBOL_TYPE sym);

    friend class Sheet;

    friend struct Point_values;
    friend bool Check_Point(const Point&, const struct Point_values&);
};

void Point_UnitTests();
}
