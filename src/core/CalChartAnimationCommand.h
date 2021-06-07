#pragma once
/*
 * CalChartAnimationCommand.h
 * Classes for the Animation Commands
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
 * Animation Commands
 * Continuities can be broken down into 3 distinct types:
 *  MarkTime: A direction to be facing
 *  Moving: A vector along which to be moving (indicating how far to move each point)
 *  Rotate: A point which to rotate, radius, start and end angles
 * AnimationCommand is an object that represents a particular part of a continuity.  When we decompose
 * continuities into these parts, we can then "transform" a point from a starting position to the end of the
 * Animation by "stepping" it along each AnimationCommand
 */

#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include <nlohmann/json.hpp>

namespace CalChart {

struct DrawCommand;

class AnimationCommand {
public:
    AnimationCommand(unsigned beats);
    virtual ~AnimationCommand() = default;

    virtual std::unique_ptr<AnimationCommand> clone() const = 0;

    // returns false if end of command
    virtual bool Begin(Coord& pt);
    virtual bool End(Coord& pt);
    virtual bool NextBeat(Coord& pt);
    virtual bool PrevBeat(Coord& pt);

    // go through all beats at once
    virtual void ApplyForward(Coord& pt);
    virtual void ApplyBackward(Coord& pt);

    virtual AnimateDir Direction() const = 0;
    virtual float RealDirection() const = 0;
    virtual float MotionDirection() const;
    virtual void ClipBeats(unsigned beats);

    virtual unsigned NumBeats() const { return mNumBeats; }

    // What style to display
    virtual MarchingStyle StepStyle() { return MarchingStyle::HighStep; }

    // when we want to have the path drawn:
    virtual DrawCommand GenCC_DrawCommand(Coord pt, Coord offset) const;

    /*!
     * @brief json  that represent this movement in an Online Viewer '.viewer' file.
     * @param start The position at which this movement begins.
     */
    virtual nlohmann::json toOnlineViewerJSON(Coord start) const = 0;

protected:
    unsigned mNumBeats;
    unsigned mBeat;
};

class AnimationCommandMT : public AnimationCommand {
public:
    AnimationCommandMT(unsigned beats, float direction);
    AnimationCommandMT(unsigned beats, AnimateDir direction);
    virtual ~AnimationCommandMT() = default;

    std::unique_ptr<AnimationCommand> clone() const override;

    AnimateDir Direction() const override;
    float RealDirection() const override;

    nlohmann::json toOnlineViewerJSON(Coord start) const override;

protected:
    AnimateDir dir;
    float realdir;
};

class AnimationCommandMove : public AnimationCommandMT {
public:
    AnimationCommandMove(unsigned beats, Coord movement);
    AnimationCommandMove(unsigned beats, Coord movement, float direction);
    virtual ~AnimationCommandMove() = default;

    std::unique_ptr<AnimationCommand> clone() const override;

    bool NextBeat(Coord& pt) override;
    bool PrevBeat(Coord& pt) override;

    void ApplyForward(Coord& pt) override;
    void ApplyBackward(Coord& pt) override;

    float MotionDirection() const override;
    void ClipBeats(unsigned beats) override;

    DrawCommand GenCC_DrawCommand(Coord pt, Coord offset) const override;

    nlohmann::json toOnlineViewerJSON(Coord start) const override;

private:
    Coord mVector;
};

class AnimationCommandRotate : public AnimationCommand {
public:
    AnimationCommandRotate(unsigned beats, Coord cntr, float rad, float ang1,
        float ang2, bool backwards = false);
    virtual ~AnimationCommandRotate() = default;

    std::unique_ptr<AnimationCommand> clone() const override;

    bool NextBeat(Coord& pt) override;
    bool PrevBeat(Coord& pt) override;

    void ApplyForward(Coord& pt) override;
    void ApplyBackward(Coord& pt) override;

    AnimateDir Direction() const override;
    float RealDirection() const override;
    void ClipBeats(unsigned beats) override;

    DrawCommand GenCC_DrawCommand(Coord pt, Coord offset) const override;

    nlohmann::json toOnlineViewerJSON(Coord start) const override;

private:
    Coord mOrigin;
    float mR, mAngStart, mAngEnd;
    float mFace;
};
}
