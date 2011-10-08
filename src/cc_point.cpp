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

#include <confgr.h>
#include <wx/wx.h>
#include <assert.h>

// Test Suite stuff
struct CC_point_values
{
	unsigned short flags;
	SYMBOL_TYPE sym;
	unsigned char cont;
	CC_coord pos;
	CC_coord ref[CC_point::kNumRefPoints];
	bool GetFlip;
};

void CC_point::Draw(wxDC& dc, unsigned reference, const CC_coord& origin, const wxBrush *fillBrush, const wxString& label)
{
	float circ_r = Float2Coord(GetConfiguration_DotRatio());
	float offset = circ_r / 2;
	float plineoff = offset * GetConfiguration_PLineRatio();
	float slineoff = offset * GetConfiguration_SLineRatio();
	float textoff = offset * 1.25;

	long x = ((reference) ? ref[reference-1].x : pos.x) + origin.x;
	long y = ((reference) ? ref[reference-1].y : pos.y) + origin.y;
	switch (sym)
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
	switch (sym)
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
	switch (sym)
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
				GetFlip() ? x : (x - textw),
				y - textoff - texth + textd);
}

bool Check_CC_point(const CC_point& underTest, const CC_point_values& values)
{
	bool running_value = true;
	for (unsigned i = 0; i < CC_point::kNumRefPoints; ++i)
		running_value = running_value && (underTest.ref[i] == values.ref[i]);
	return running_value
		&& (underTest.flags == values.flags)
		&& (underTest.sym == values.sym)
		&& (underTest.cont == values.cont)
		&& (underTest.pos == values.pos)
		&& (underTest.GetFlip() == values.GetFlip)
		;
}

void CC_point_UnitTests()
{
	// test some defaults:
	CC_point_values values;
	values.flags = 0;
	values.sym = SYMBOL_PLAIN;
	values.cont = 0;
	values.pos = CC_coord();
	for (unsigned i = 0; i < CC_point::kNumRefPoints; ++i)
		values.ref[i] = CC_coord();
	values.GetFlip = false;

	// test defaults
	CC_point underTest;
	assert(Check_CC_point(underTest, values));

	// test flip
	underTest.Flip(false);
	assert(Check_CC_point(underTest, values));

	values.flags = 1;
	values.GetFlip = true;
	underTest.Flip(true);
	assert(Check_CC_point(underTest, values));

	values.flags = 0;
	values.GetFlip = false;
	underTest.Flip(false);
	assert(Check_CC_point(underTest, values));

	// test flip toggle
	values.flags = 1;
	values.GetFlip = true;
	underTest.FlipToggle();
	assert(Check_CC_point(underTest, values));

	values.flags = 0;
	values.GetFlip = false;
	underTest.FlipToggle();
	assert(Check_CC_point(underTest, values));

}
