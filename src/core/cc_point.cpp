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

#include <assert.h>
#include <stdexcept>

CC_point::CC_point() :
mSym(SYMBOL_PLAIN),
mContinuityIndex(0)
{
}

CC_point::CC_point(uint8_t c, const CC_coord& p) :
mSym(SYMBOL_PLAIN),
mContinuityIndex(c),
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
	return mFlags.test(kPointLabelFlipped);
}

void
CC_point::Flip(bool val)
{
	mFlags.set(kPointLabelFlipped, val);
};

void
CC_point::FlipToggle()
{
	mFlags.flip(kPointLabelFlipped);
}

CC_coord
CC_point::GetPos(unsigned ref) const
{
	if (ref == 0)
	{
		return mPos;
	}
	if (ref > kNumRefPoints)
	{
		throw std::range_error("GetPos() point out of range");
	}
	return mRef[ref-1];
}

void
CC_point::SetPos(const CC_coord& c, unsigned ref)
{
	if (ref == 0)
	{
		mPos = c;
		return;
	}
	if (ref > kNumRefPoints)
	{
		throw std::range_error("SetRefPos() point out of range");
	}
	mRef[ref-1] = c;
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

uint8_t
CC_point::GetContinuityIndex() const
{
	return mContinuityIndex;
}

void
CC_point::SetContinuityIndex(uint8_t c)
{
	mContinuityIndex = c;
}


// Test Suite stuff
struct CC_point_values
{
	std::bitset<CC_point::kTotalBits> mFlags;
	SYMBOL_TYPE mSym;
	uint8_t mContinuityIndex;
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
		&& (underTest.mContinuityIndex == values.mContinuityIndex)
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
	values.mContinuityIndex = 0;
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