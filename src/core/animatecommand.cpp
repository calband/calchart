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

DrawCommand AnimateCommand::GenCC_DrawCommand(const Coord& /*pt*/, const Coord& /*offset*/) const
{
    return DrawCommand();
}

bool AnimateCommand::Begin(Coord& pt)
{
    mBeat = 0;
    if (mNumBeats == 0) {
        ApplyForward(pt);
        return false;
    }
    return true;
}

bool AnimateCommand::End(Coord& pt)
{
    mBeat = mNumBeats;
    if (mNumBeats == 0) {
        ApplyBackward(pt);
        return false;
    }
    return true;
}

bool AnimateCommand::NextBeat(Coord&)
{
    ++mBeat;
    if (mBeat >= mNumBeats)
        return false;
    return true;
}

bool AnimateCommand::PrevBeat(Coord&)
{
    if (mBeat == 0)
        return false;
    else {
        --mBeat;
        return true;
    }
}

void AnimateCommand::ApplyForward(Coord&) { mBeat = mNumBeats; }

void AnimateCommand::ApplyBackward(Coord&) { mBeat = 0; }

float AnimateCommand::MotionDirection() const { return RealDirection(); }

void AnimateCommand::ClipBeats(unsigned beats) { mNumBeats = beats; }

AnimateCommandMT::AnimateCommandMT(unsigned beats, float direction)
    : AnimateCommand(beats)
    , dir(AnimGetDirFromAngle(direction))
    , realdir(direction)
{
}

std::unique_ptr<AnimateCommand> AnimateCommandMT::clone() const
{
    return std::make_unique<AnimateCommandMT>(*this);
}

AnimateDir AnimateCommandMT::Direction() const { return dir; }

float AnimateCommandMT::RealDirection() const { return realdir; }

nlohmann::json AnimateCommandMT::toOnlineViewerJSON(const Coord& start) const
{
    nlohmann::json j;

    j["type"] = "mark";
    j["beats"] = static_cast<double>(NumBeats());
    j["facing"] = ToOnlineViewer::angle(RealDirection());
    j["x"] = ToOnlineViewer::xPosition(start.x);
    j["y"] = ToOnlineViewer::yPosition(start.y);
    return j;
}

AnimateCommandMove::AnimateCommandMove(unsigned beats, Coord movement)
    : AnimateCommandMT(beats, movement.Direction())
    , mVector(movement)
{
}

std::unique_ptr<AnimateCommand> AnimateCommandMove::clone() const
{
    return std::make_unique<AnimateCommandMove>(*this);
}

AnimateCommandMove::AnimateCommandMove(unsigned beats, Coord movement,
    float direction)
    : AnimateCommandMT(beats, direction)
    , mVector(movement)
{
}

bool AnimateCommandMove::NextBeat(Coord& pt)
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

bool AnimateCommandMove::PrevBeat(Coord& pt)
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

void AnimateCommandMove::ApplyForward(Coord& pt)
{
    AnimateCommand::ApplyForward(pt);
    pt += mVector;
}

void AnimateCommandMove::ApplyBackward(Coord& pt)
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
AnimateCommandMove::GenCC_DrawCommand(const Coord& pt, const Coord& offset) const
{
    return { pt.x + offset.x, pt.y + offset.y,
        pt.x + mVector.x + offset.x,
        pt.y + mVector.y + offset.y };
}

nlohmann::json AnimateCommandMove::toOnlineViewerJSON(const Coord& start) const
{
    nlohmann::json j;

    j["type"] = "even";
    j["beats"] = static_cast<double>(NumBeats());
    j["beats_per_step"] = static_cast<double>(1);
    j["x1"] = ToOnlineViewer::xPosition(start.x);
    j["y1"] = ToOnlineViewer::yPosition(start.y);
    j["x2"] = ToOnlineViewer::xPosition(start.x + mVector.x);
    j["y2"] = ToOnlineViewer::yPosition(start.y + mVector.y);
    j["facing"] = ToOnlineViewer::angle(MotionDirection());
    return j;
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

std::unique_ptr<AnimateCommand> AnimateCommandRotate::clone() const
{
    return std::make_unique<AnimateCommandRotate>(*this);
}

bool AnimateCommandRotate::NextBeat(Coord& pt)
{
    bool b = AnimateCommand::NextBeat(pt);
    float curr_ang = (mNumBeats ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart)
                                : mAngStart)
        * M_PI / 180.0;
    pt.x = RoundToCoordUnits(mOrigin.x + cos(curr_ang) * mR);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(curr_ang) * mR);
    return b;
}

bool AnimateCommandRotate::PrevBeat(Coord& pt)
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

void AnimateCommandRotate::ApplyForward(Coord& pt)
{
    AnimateCommand::ApplyForward(pt);
    pt.x = RoundToCoordUnits(mOrigin.x + cos(mAngEnd * M_PI / 180.0) * mR);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(mAngEnd * M_PI / 180.0) * mR);
}

void AnimateCommandRotate::ApplyBackward(Coord& pt)
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
AnimateCommandRotate::GenCC_DrawCommand(const Coord& /*pt*/, const Coord& offset) const
{
    float start = (mAngStart < mAngEnd) ? mAngStart : mAngEnd;
    float end = (mAngStart < mAngEnd) ? mAngEnd : mAngStart;
    auto x_start = RoundToCoordUnits(mOrigin.x + cos(start * M_PI / 180.0) * mR) + offset.x;
    auto y_start = RoundToCoordUnits(mOrigin.y - sin(start * M_PI / 180.0) * mR) + offset.y;
    auto x_end = RoundToCoordUnits(mOrigin.x + cos(end * M_PI / 180.0) * mR) + offset.x;
    auto y_end = RoundToCoordUnits(mOrigin.y - sin(end * M_PI / 180.0) * mR) + offset.y;

    return { x_start, y_start, x_end, y_end, mOrigin.x + offset.x, mOrigin.y + offset.y };
}

nlohmann::json AnimateCommandRotate::toOnlineViewerJSON(const Coord& start) const
{
    nlohmann::json j;

    j["type"] = "arc";
    j["start_x"] = ToOnlineViewer::xPosition(start.x);
    j["start_y"] = ToOnlineViewer::yPosition(start.y);
    j["center_x"] = ToOnlineViewer::xPosition(mOrigin.x);
    j["center_y"] = ToOnlineViewer::yPosition(mOrigin.y);
    j["angle"] = -(mAngEnd - mAngStart);
    j["beats"] = static_cast<double>(NumBeats());
    j["beats_per_step"] = static_cast<double>(1);
    j["facing_offset"] = -mFace + 90;
    return j;
}
}
