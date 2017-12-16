#pragma once
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

#include "animate.h"
#include "json.h"

namespace CalChart {

struct DrawCommand;

class AnimateCommand {
public:
    AnimateCommand(unsigned beats);
    virtual ~AnimateCommand() = default;

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
    virtual DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const Coord& offset) const;

    /*!
     * @brief Manipulates dest so that it contains a JSONElement that
     * could represent this movement in an Online Viewer '.viewer' file.
     * @param start The position at which this movement begins.
     */
    JSONElement toOnlineViewerJSON(const Coord& start) const;
    /*!
     * @brief Manipulates dest so that it contains a JSONElement that
     * could represent this movement in an Online Viewer '.viewer' file.
     * @param dest A reference to the JSONElement which will be transformed
     * into a JSON representation of this movement.
     * @param start The position at which this movement begins.
     */
    virtual void toOnlineViewerJSON(JSONElement& dest, const Coord& start) const = 0;

protected:
    unsigned mNumBeats;
    unsigned mBeat;
};

class AnimateCommandMT : public AnimateCommand {
public:
    AnimateCommandMT(unsigned beats, float direction);
    virtual ~AnimateCommandMT() = default;

    virtual AnimateDir Direction() const;
    virtual float RealDirection() const;

    void toOnlineViewerJSON(JSONElement& dest, const Coord& start) const;

protected:
    AnimateDir dir;
    float realdir;
};

class AnimateCommandMove : public AnimateCommandMT {
public:
    AnimateCommandMove(unsigned beats, Coord movement);
    AnimateCommandMove(unsigned beats, Coord movement, float direction);
    virtual ~AnimateCommandMove() = default;

    virtual bool NextBeat(AnimatePoint& pt);
    virtual bool PrevBeat(AnimatePoint& pt);

    virtual void ApplyForward(AnimatePoint& pt);
    virtual void ApplyBackward(AnimatePoint& pt);

    virtual float MotionDirection() const;
    virtual void ClipBeats(unsigned beats);

    virtual DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const Coord& offset) const;

    void toOnlineViewerJSON(JSONElement& dest, const Coord& start) const;

private:
    Coord mVector;
};

class AnimateCommandRotate : public AnimateCommand {
public:
    AnimateCommandRotate(unsigned beats, Coord cntr, float rad, float ang1,
        float ang2, bool backwards = false);
    virtual ~AnimateCommandRotate() = default;

    virtual bool NextBeat(AnimatePoint& pt);
    virtual bool PrevBeat(AnimatePoint& pt);

    virtual void ApplyForward(AnimatePoint& pt);
    virtual void ApplyBackward(AnimatePoint& pt);

    virtual AnimateDir Direction() const;
    virtual float RealDirection() const;
    virtual void ClipBeats(unsigned beats);

    virtual DrawCommand GenCC_DrawCommand(const AnimatePoint& pt, const Coord& offset) const;

    void toOnlineViewerJSON(JSONElement& dest, const Coord& start) const;

private:
    Coord mOrigin;
    float mR, mAngStart, mAngEnd;
    float mFace;
};
}
