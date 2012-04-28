/*
 * animate.h
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

#ifndef _ANIMATECOMMAND_H_
#define _ANIMATECOMMAND_H_

#include "animate.h"

class AnimateCommand
{
public:
	AnimateCommand(unsigned beats);
	virtual ~AnimateCommand();

// returns false if end of command
	virtual bool Begin(AnimatePoint& pt);
	virtual bool End(AnimatePoint& pt);
	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

// go through all beats at once
	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction() const = 0;
	virtual float RealDirection() const = 0;
	virtual float MotionDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual unsigned NumBeats() const { return mNumBeats; }

	// What style to display
	virtual MarchingStyle StepStyle() { return STYLE_HighStep; }

	// when we want to have the path drawn:
	virtual void DrawCommand(wxDC& dc, const AnimatePoint& pt, const CC_coord& offset) const {}

protected:
	unsigned mNumBeats;
	unsigned mBeat;
};

class AnimateCommandMT : public AnimateCommand
{
public:
	AnimateCommandMT(unsigned beats, float direction);
	virtual ~AnimateCommandMT() {}

	virtual AnimateDir Direction() const;
	virtual float RealDirection() const;
protected:
	AnimateDir dir;
	float realdir;
};

class AnimateCommandMove : public AnimateCommandMT
{
public:
	AnimateCommandMove(unsigned beats, CC_coord movement);
	AnimateCommandMove(unsigned beats, CC_coord movement, float direction);
	virtual ~AnimateCommandMove() {}

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual float MotionDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual void DrawCommand(wxDC& dc, const AnimatePoint& pt, const CC_coord& offset) const;

private:
	CC_coord mVector;
};

class AnimateCommandRotate : public AnimateCommand
{
public:
	AnimateCommandRotate(unsigned beats, CC_coord cntr, float rad,
		float ang1, float ang2, bool backwards = false);
	virtual ~AnimateCommandRotate() {}

	virtual bool NextBeat(AnimatePoint& pt);
	virtual bool PrevBeat(AnimatePoint& pt);

	virtual void ApplyForward(AnimatePoint& pt);
	virtual void ApplyBackward(AnimatePoint& pt);

	virtual AnimateDir Direction() const;
	virtual float RealDirection() const;
	virtual void ClipBeats(unsigned beats);

	virtual void DrawCommand(wxDC& dc, const AnimatePoint& pt, const CC_coord& offset) const;

private:
	CC_coord mOrigin;
	float mR, mAngStart, mAngEnd;
	float mFace;
};

#endif //_ANIMATECOMMAND_H_
