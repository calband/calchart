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

#ifndef _CC_POINT_H_
#define _CC_POINT_H_

#include "cc_types.h"
#include "cc_coord.h"

class wxDC;
class wxString;
class wxBrush;

class CC_point
{
public:
	static const unsigned kNumRefPoints = 3;
	CC_point()
		:flags(0), sym(SYMBOL_PLAIN), cont(0) {}

	inline bool GetFlip() const { return (bool)(flags & kPointLabel); }
	inline void Flip(bool val = true)
	{
		if (val) flags |= kPointLabel;
		else flags &= ~kPointLabel;
	};
	inline void FlipToggle() { Flip(GetFlip() ? false:true); }

	unsigned short flags;
	// by having both a sym type and cont index, we can have several
	// points share the same symbol but have different continuities.
	SYMBOL_TYPE sym;
	unsigned char cont;
	CC_coord pos;
	CC_coord ref[kNumRefPoints];

	// Draw the point
	void Draw(wxDC& dc, unsigned reference, const CC_coord& origin, const wxBrush *fillBrush, const wxString& label);
	
private:
	static const unsigned kPointLabel = 1;

friend bool Check_CC_point(const CC_point&, const struct CC_point_values&);
};

void CC_point_UnitTests();

#endif // _CC_POINT_H_
