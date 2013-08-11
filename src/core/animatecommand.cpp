/*
 * animate.cpp
 * Classes for animating shows
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

#include "animatecommand.h"
#define _USE_MATH_DEFINES
#include <cmath>


// nothing version
AnimateDraw::AnimateDraw() :
mType(Ignore)
{
}

// Line version
AnimateDraw::AnimateDraw(int startx, int starty, int endx, int endy) :
mType(Line),
x1(startx),
y1(starty),
x2(endx),
y2(endy)
{
}

// Arc version
AnimateDraw::AnimateDraw(int startx, int starty, int endx, int endy, int centerx, int centery) :
mType(Arc),
x1(startx),
y1(starty),
x2(endx),
y2(endy),
xc(centerx),
yc(centery)
{
}

AnimateCommand::AnimateCommand(unsigned beats)
: mNumBeats(beats), mBeat(0)
{
}

AnimateCommand::~AnimateCommand()
{
}

bool AnimateCommand::Begin(AnimatePoint& pt)
{
	mBeat = 0;
	if (mNumBeats == 0)
	{
		ApplyForward(pt);
		return false;
	}
	return true;
}


bool AnimateCommand::End(AnimatePoint& pt)
{
	mBeat = mNumBeats;
	if (mNumBeats == 0)
	{
		ApplyBackward(pt);
		return false;
	}
	return true;
}


bool AnimateCommand::NextBeat(AnimatePoint&)
{
	++mBeat;
	if (mBeat >= mNumBeats) return false;
	return true;
}


bool AnimateCommand::PrevBeat(AnimatePoint&)
{
	if (mBeat == 0) return false;
	else
	{
		--mBeat;
		return true;
	}
}


void AnimateCommand::ApplyForward(AnimatePoint&)
{
	mBeat = mNumBeats;
}


void AnimateCommand::ApplyBackward(AnimatePoint&)
{
	mBeat = 0;
}


float AnimateCommand::MotionDirection() const
{
	return RealDirection();
}


void AnimateCommand::ClipBeats(unsigned beats)
{
	mNumBeats = beats;
}


AnimateCommandMT::AnimateCommandMT(unsigned beats, float direction)
: AnimateCommand(beats), dir(AnimGetDirFromAngle(direction)), realdir(direction)
{
}


AnimateDir AnimateCommandMT::Direction() const { return dir; }

float AnimateCommandMT::RealDirection() const { return realdir; }

AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement)
: AnimateCommandMT(beats, movement.Direction()), mVector(movement)
{
}


AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement, float direction)
: AnimateCommandMT(beats, direction), mVector(movement)
{
}


bool AnimateCommandMove::NextBeat(AnimatePoint& pt)
{
	bool b = AnimateCommand::NextBeat(pt);
	pt.x += (mNumBeats) ?
		((long)mBeat * mVector.x / (short)mNumBeats) -
		((long)(mBeat-1) * mVector.x / (short)mNumBeats) : 0;
	pt.y += (mNumBeats) ?
		((long)mBeat * mVector.y / (short)mNumBeats) -
		((long)(mBeat-1) * mVector.y / (short)mNumBeats) : 0;
	return b;
}


bool AnimateCommandMove::PrevBeat(AnimatePoint& pt)
{
	if (AnimateCommand::PrevBeat(pt))
	{
		pt.x += mNumBeats ?
			((long)mBeat * mVector.x / (short)mNumBeats) -
			((long)(mBeat+1) * mVector.x / (short)mNumBeats) : 0;
		pt.y += mNumBeats ?
			((long)mBeat * mVector.y / (short)mNumBeats) -
			((long)(mBeat+1) * mVector.y / (short)mNumBeats) : 0;
		return true;
	}
	else
	{
		return false;
	}
}


void AnimateCommandMove::ApplyForward(AnimatePoint& pt)
{
	AnimateCommand::ApplyForward(pt);
	pt += mVector;
}


void AnimateCommandMove::ApplyBackward(AnimatePoint& pt)
{
	AnimateCommand::ApplyBackward(pt);
	pt -= mVector;
}


float AnimateCommandMove::MotionDirection() const
{
	return mVector.Direction();
}


void AnimateCommandMove::ClipBeats(unsigned beats)
{
	AnimateCommand::ClipBeats(beats);
}


AnimateDraw AnimateCommandMove::GenAnimateDraw(const AnimatePoint& pt, const CC_coord& offset) const
{
	return AnimateDraw(pt.x + offset.x, pt.y + offset.y, pt.x + mVector.x + offset.x, pt.y + mVector.y + offset.y);
}


AnimateCommandRotate::AnimateCommandRotate(unsigned beats, CC_coord cntr,
float rad, float ang1, float ang2,
bool backwards)
: AnimateCommand(beats), mOrigin(cntr), mR(rad), mAngStart(ang1), mAngEnd(ang2)
{
	if (backwards) mFace = -90;
	else mFace = 90;
}


bool AnimateCommandRotate::NextBeat(AnimatePoint& pt)
{
	bool b = AnimateCommand::NextBeat(pt);
	float curr_ang = (mNumBeats ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart) : mAngStart)
		* M_PI / 180.0;
	pt.x = RoundToCoord(mOrigin.x + cos(curr_ang)*mR);
	pt.y = RoundToCoord(mOrigin.y - sin(curr_ang)*mR);
	return b;
}


bool AnimateCommandRotate::PrevBeat(AnimatePoint& pt)
{
	if (AnimateCommand::PrevBeat(pt))
	{
		float curr_ang = (mNumBeats ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart) : mAngStart)
			* M_PI / 180.0;
		pt.x = RoundToCoord(mOrigin.x + cos(curr_ang)*mR);
		pt.y = RoundToCoord(mOrigin.y - sin(curr_ang)*mR);
		return true;
	}
	else
	{
		return false;
	}
}


void AnimateCommandRotate::ApplyForward(AnimatePoint& pt)
{
	AnimateCommand::ApplyForward(pt);
	pt.x = RoundToCoord(mOrigin.x + cos(mAngEnd*M_PI/180.0)*mR);
	pt.y = RoundToCoord(mOrigin.y - sin(mAngEnd*M_PI/180.0)*mR);
}


void AnimateCommandRotate::ApplyBackward(AnimatePoint& pt)
{
	AnimateCommand::ApplyBackward(pt);
	pt.x = RoundToCoord(mOrigin.x + cos(mAngStart*M_PI/180.0)*mR);
	pt.y = RoundToCoord(mOrigin.y - sin(mAngStart*M_PI/180.0)*mR);
}


AnimateDir AnimateCommandRotate::Direction() const
{
	return AnimGetDirFromAngle(RealDirection());
}


float AnimateCommandRotate::RealDirection() const
{
	float curr_ang = mNumBeats ? (mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart : mAngStart;
	if (mAngEnd > mAngStart)
	{
		return curr_ang + mFace;
	}
	else
	{
		return curr_ang - mFace;
	}
}


void AnimateCommandRotate::ClipBeats(unsigned beats)
{
	AnimateCommand::ClipBeats(beats);
}


AnimateDraw AnimateCommandRotate::GenAnimateDraw(const AnimatePoint& pt, const CC_coord& offset) const
{
	float start = (mAngStart < mAngEnd) ? mAngStart : mAngEnd;
	float end = (mAngStart < mAngEnd) ? mAngEnd : mAngStart;
	auto x_start = RoundToCoord(mOrigin.x + cos(start*M_PI/180.0)*mR) + offset.x;
	auto y_start = RoundToCoord(mOrigin.y - sin(start*M_PI/180.0)*mR) + offset.y;
	auto x_end = RoundToCoord(mOrigin.x + cos(end*M_PI/180.0)*mR) + offset.x;
	auto y_end = RoundToCoord(mOrigin.y - sin(end*M_PI/180.0)*mR) + offset.y;

	return AnimateDraw(x_start, y_start, x_end, y_end, mOrigin.x + offset.x, mOrigin.y + offset.y);
}

