/*
 * cc_point.cpp
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

#include "cc_point.h"

#include "confgr.h"
#include <wx/wx.h>
#include <assert.h>

CC_point::CC_point() :
mFlags(0),
mSym(SYMBOL_PLAIN),
mCont(0)
{
}

CC_point::CC_point(unsigned char c, const CC_coord& p) :
mFlags(0),
mSym(SYMBOL_PLAIN),
mCont(c),
mPos(p)
{
	for (unsigned j = 0; j < CC_point::kNumRefPoints; j++)
	{
		mRef[j] = mPos;
	}
}

bool
CC_point::GetFlip() const
{
	return (bool)(mFlags & kPointLabel);
}

void
CC_point::Flip(bool val)
{
	if (val) mFlags |= kPointLabel;
	else mFlags &= ~kPointLabel;
};

void
CC_point::FlipToggle()
{
	Flip(GetFlip() ? false:true);
}

CC_coord
CC_point::GetPos() const
{
	return mPos;
}

void
CC_point::SetPos(const CC_coord& c)
{
	mPos = c;
}

CC_coord
CC_point::GetRefPos(unsigned which) const
{
	return mRef[which];
}

void
CC_point::SetRefPos(const CC_coord& c, unsigned which)
{
	mRef[which] = c;
}

SYMBOL_TYPE
CC_point::GetSymbol() const
{
	return mSym;
}

void
CC_point::SetSymbol(SYMBOL_TYPE s)
{
	mSym = s;
}

unsigned char
CC_point::GetCont() const
{
	return mCont;
}

void
CC_point::SetCont(unsigned char c)
{
	mCont = c;
}


void
DrawPoint(const CC_point& point, wxDC& dc, unsigned reference, const CC_coord& origin, const wxBrush *fillBrush, const wxString& label)
{
	float circ_r = Float2Coord(GetConfiguration_DotRatio());
	float offset = circ_r / 2;
	float plineoff = offset * GetConfiguration_PLineRatio();
	float slineoff = offset * GetConfiguration_SLineRatio();
	float textoff = offset * 1.25;

	long x = ((reference) ? point.GetRefPos(reference-1).x : point.GetPos().x) + origin.x;
	long y = ((reference) ? point.GetRefPos(reference-1).y : point.GetPos().y) + origin.y;
	switch (point.GetSymbol())
	{
		case SYMBOL_SOL:
		case SYMBOL_SOLBKSL:
		case SYMBOL_SOLSL:
		case SYMBOL_SOLX:
			dc.SetBrush(*fillBrush);
			break;
		default:
			dc.SetBrush(*wxTRANSPARENT_BRUSH);
	}
	dc.DrawEllipse(x - offset, y - offset, circ_r, circ_r);
	switch (point.GetSymbol())
	{
		case SYMBOL_SL:
		case SYMBOL_X:
			dc.DrawLine(x - plineoff, y + plineoff,
						x + plineoff, y - plineoff);
			break;
		case SYMBOL_SOLSL:
		case SYMBOL_SOLX:
			dc.DrawLine(x - slineoff, y + slineoff,
						x + slineoff, y - slineoff);
			break;
		default:
			break;
	}
	switch (point.GetSymbol())
	{
		case SYMBOL_BKSL:
		case SYMBOL_X:
			dc.DrawLine(x - plineoff, y - plineoff,
						x + plineoff, y + plineoff);
			break;
		case SYMBOL_SOLBKSL:
		case SYMBOL_SOLX:
			dc.DrawLine(x - slineoff, y - slineoff,
						x + slineoff, y + slineoff);
			break;
		default:
			break;
	}
	wxCoord textw, texth, textd;
	dc.GetTextExtent(label, &textw, &texth, &textd);
	dc.DrawText(label,
				point.GetFlip() ? x : (x - textw),
				y - textoff - texth + textd);
}

// Test Suite stuff
struct CC_point_values
{
	unsigned short mFlags;
	SYMBOL_TYPE mSym;
	unsigned char mCont;
	CC_coord mPos;
	CC_coord mRef[CC_point::kNumRefPoints];
	bool GetFlip;
};

bool Check_CC_point(const CC_point& underTest, const CC_point_values& values)
{
	bool running_value = true;
	for (unsigned i = 0; i < CC_point::kNumRefPoints; ++i)
		running_value = running_value && (underTest.mRef[i] == values.mRef[i]);
	return running_value
		&& (underTest.mFlags == values.mFlags)
		&& (underTest.mSym == values.mSym)
		&& (underTest.mCont == values.mCont)
		&& (underTest.mPos == values.mPos)
		&& (underTest.GetFlip() == values.GetFlip)
		;
}

void CC_point_UnitTests()
{
	// test some defaults:
	CC_point_values values;
	values.mFlags = 0;
	values.mSym = SYMBOL_PLAIN;
	values.mCont = 0;
	values.mPos = CC_coord();
	for (unsigned i = 0; i < CC_point::kNumRefPoints; ++i)
		values.mRef[i] = CC_coord();
	values.GetFlip = false;

	// test defaults
	CC_point underTest;
	assert(Check_CC_point(underTest, values));

	// test flip
	underTest.Flip(false);
	assert(Check_CC_point(underTest, values));

	values.mFlags = 1;
	values.GetFlip = true;
	underTest.Flip(true);
	assert(Check_CC_point(underTest, values));

	values.mFlags = 0;
	values.GetFlip = false;
	underTest.Flip(false);
	assert(Check_CC_point(underTest, values));

	// test flip toggle
	values.mFlags = 1;
	values.GetFlip = true;
	underTest.FlipToggle();
	assert(Check_CC_point(underTest, values));

	values.mFlags = 0;
	values.GetFlip = false;
	underTest.FlipToggle();
	assert(Check_CC_point(underTest, values));

}
