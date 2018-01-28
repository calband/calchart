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

#define _USE_MATH_DEFINES
#include <cmath>

#include "animatecommand.h"
#include "cc_drawcommand.h"
#include "viewer_translate.h"

namespace CalChart {

AnimateCommand::AnimateCommand(unsigned beats)
    : mNumBeats(beats)
    , mBeat(0)
{
}

DrawCommand AnimateCommand::GenCC_DrawCommand(const AnimatePoint& pt, const Coord& offset) const
{
    return DrawCommand();
}

bool AnimateCommand::Begin(AnimatePoint& pt)
{
    mBeat = 0;
    if (mNumBeats == 0) {
        ApplyForward(pt);
        return false;
    }
    return true;
}

bool AnimateCommand::End(AnimatePoint& pt)
{
    mBeat = mNumBeats;
    if (mNumBeats == 0) {
        ApplyBackward(pt);
        return false;
    }
    return true;
}

bool AnimateCommand::NextBeat(AnimatePoint&)
{
    ++mBeat;
    if (mBeat >= mNumBeats)
        return false;
    return true;
}

bool AnimateCommand::PrevBeat(AnimatePoint&)
{
    if (mBeat == 0)
        return false;
    else {
        --mBeat;
        return true;
    }
}

void AnimateCommand::ApplyForward(AnimatePoint&) { mBeat = mNumBeats; }

void AnimateCommand::ApplyBackward(AnimatePoint&) { mBeat = 0; }

float AnimateCommand::MotionDirection() const { return RealDirection(); }

void AnimateCommand::ClipBeats(unsigned beats) { mNumBeats = beats; }

JSONElement AnimateCommand::toOnlineViewerJSON(const Coord& start) const
{
    JSONElement newViewerObject = JSONElement::makeNull();
    toOnlineViewerJSON(newViewerObject, start);
    return newViewerObject;
}

AnimateCommandMT::AnimateCommandMT(unsigned beats, float direction)
    : AnimateCommand(beats)
    , dir(AnimGetDirFromAngle(direction))
    , realdir(direction)
{
}

AnimateDir AnimateCommandMT::Direction() const { return dir; }

float AnimateCommandMT::RealDirection() const { return realdir; }

void AnimateCommandMT::toOnlineViewerJSON(JSONElement& dest, const Coord& start) const
{
    JSONDataObjectAccessor moveAccessor = dest = JSONElement::makeObject();

    moveAccessor["type"] = "mark";
    moveAccessor["beats"] = NumBeats();
    moveAccessor["facing"] = ToOnlineViewer::angle(RealDirection());
    moveAccessor["x"] = ToOnlineViewer::xPosition(start.x);
    moveAccessor["y"] = ToOnlineViewer::yPosition(start.y);
}

AnimateCommandMove::AnimateCommandMove(unsigned beats, Coord movement)
    : AnimateCommandMT(beats, movement.Direction())
    , mVector(movement)
{
}

AnimateCommandMove::AnimateCommandMove(unsigned beats, Coord movement,
    float direction)
    : AnimateCommandMT(beats, direction)
    , mVector(movement)
{
}

bool AnimateCommandMove::NextBeat(AnimatePoint& pt)
{
    bool b = AnimateCommand::NextBeat(pt);
    pt.x += (mNumBeats)
        ? ((long)mBeat * mVector.x / (short)mNumBeats) - ((long)(mBeat - 1) * mVector.x / (short)mNumBeats)
        : 0;
    pt.y += (mNumBeats)
        ? ((long)mBeat * mVector.y / (short)mNumBeats) - ((long)(mBeat - 1) * mVector.y / (short)mNumBeats)
        : 0;
    return b;
}

bool AnimateCommandMove::PrevBeat(AnimatePoint& pt)
{
    if (AnimateCommand::PrevBeat(pt)) {
        pt.x += mNumBeats
            ? ((long)mBeat * mVector.x / (short)mNumBeats) - ((long)(mBeat + 1) * mVector.x / (short)mNumBeats)
            : 0;
        pt.y += mNumBeats
            ? ((long)mBeat * mVector.y / (short)mNumBeats) - ((long)(mBeat + 1) * mVector.y / (short)mNumBeats)
            : 0;
        return true;
    } else {
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

DrawCommand
AnimateCommandMove::GenCC_DrawCommand(const AnimatePoint& pt, const Coord& offset) const
{
    return { pt.x + offset.x, pt.y + offset.y,
        pt.x + mVector.x + offset.x,
        pt.y + mVector.y + offset.y };
}

void AnimateCommandMove::toOnlineViewerJSON(JSONElement& dest, const Coord& start) const
{
    JSONDataObjectAccessor moveAccessor = dest = JSONElement::makeObject();

    moveAccessor["type"] = "even";
    moveAccessor["beats"] = NumBeats();
    moveAccessor["beats_per_step"] = 1;
    moveAccessor["x1"] = ToOnlineViewer::xPosition(start.x);
    moveAccessor["y1"] = ToOnlineViewer::yPosition(start.y);
    moveAccessor["x2"] = ToOnlineViewer::xPosition(start.x + mVector.x);
    moveAccessor["y2"] = ToOnlineViewer::yPosition(start.y + mVector.y);
    moveAccessor["facing"] = ToOnlineViewer::angle(MotionDirection());
}

AnimateCommandRotate::AnimateCommandRotate(unsigned beats, Coord cntr,
    float rad, float ang1, float ang2,
    bool backwards)
    : AnimateCommand(beats)
    , mOrigin(cntr)
    , mR(rad)
    , mAngStart(ang1)
    , mAngEnd(ang2)
{
    if (backwards)
        mFace = -90;
    else
        mFace = 90;
}

bool AnimateCommandRotate::NextBeat(AnimatePoint& pt)
{
    bool b = AnimateCommand::NextBeat(pt);
    float curr_ang = (mNumBeats ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart)
                                : mAngStart)
        * M_PI / 180.0;
    pt.x = RoundToCoordUnits(mOrigin.x + cos(curr_ang) * mR);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(curr_ang) * mR);
    return b;
}

bool AnimateCommandRotate::PrevBeat(AnimatePoint& pt)
{
    if (AnimateCommand::PrevBeat(pt)) {
        float curr_ang = (mNumBeats ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart)
                                    : mAngStart)
            * M_PI / 180.0;
        pt.x = RoundToCoordUnits(mOrigin.x + cos(curr_ang) * mR);
        pt.y = RoundToCoordUnits(mOrigin.y - sin(curr_ang) * mR);
        return true;
    } else {
        return false;
    }
}

void AnimateCommandRotate::ApplyForward(AnimatePoint& pt)
{
    AnimateCommand::ApplyForward(pt);
    pt.x = RoundToCoordUnits(mOrigin.x + cos(mAngEnd * M_PI / 180.0) * mR);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(mAngEnd * M_PI / 180.0) * mR);
}

void AnimateCommandRotate::ApplyBackward(AnimatePoint& pt)
{
    AnimateCommand::ApplyBackward(pt);
    pt.x = RoundToCoordUnits(mOrigin.x + cos(mAngStart * M_PI / 180.0) * mR);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(mAngStart * M_PI / 180.0) * mR);
}

AnimateDir AnimateCommandRotate::Direction() const
{
    return AnimGetDirFromAngle(RealDirection());
}

float AnimateCommandRotate::RealDirection() const
{
    float curr_ang = mNumBeats
        ? (mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart
        : mAngStart;
    if (mAngEnd > mAngStart) {
        return curr_ang + mFace;
    } else {
        return curr_ang - mFace;
    }
}

void AnimateCommandRotate::ClipBeats(unsigned beats)
{
    AnimateCommand::ClipBeats(beats);
}

DrawCommand
AnimateCommandRotate::GenCC_DrawCommand(const AnimatePoint& pt, const Coord& offset) const
{
    float start = (mAngStart < mAngEnd) ? mAngStart : mAngEnd;
    float end = (mAngStart < mAngEnd) ? mAngEnd : mAngStart;
    auto x_start = RoundToCoordUnits(mOrigin.x + cos(start * M_PI / 180.0) * mR) + offset.x;
    auto y_start = RoundToCoordUnits(mOrigin.y - sin(start * M_PI / 180.0) * mR) + offset.y;
    auto x_end = RoundToCoordUnits(mOrigin.x + cos(end * M_PI / 180.0) * mR) + offset.x;
    auto y_end = RoundToCoordUnits(mOrigin.y - sin(end * M_PI / 180.0) * mR) + offset.y;

    return { x_start, y_start, x_end, y_end, mOrigin.x + offset.x, mOrigin.y + offset.y };
}

void AnimateCommandRotate::toOnlineViewerJSON(JSONElement& dest, const Coord& start) const
{
    JSONDataObjectAccessor moveAccessor = dest = JSONElement::makeObject();

    moveAccessor["type"] = "arc";
    moveAccessor["start_x"] = ToOnlineViewer::xPosition(start.x);
    moveAccessor["start_y"] = ToOnlineViewer::yPosition(start.y);
    moveAccessor["center_x"] = ToOnlineViewer::xPosition(mOrigin.x);
    moveAccessor["center_y"] = ToOnlineViewer::yPosition(mOrigin.y);
    moveAccessor["angle"] = -(mAngEnd - mAngStart);
    moveAccessor["beats"] = NumBeats();
    moveAccessor["beats_per_step"] = 1;
    moveAccessor["facing_offset"] = -mFace + 90;
}
}
