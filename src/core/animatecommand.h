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
#include <nlohmann/json.hpp>

namespace CalChart {

struct DrawCommand;

class AnimateCommand {
public:
    AnimateCommand(unsigned beats);
    virtual ~AnimateCommand() = default;

    virtual std::unique_ptr<AnimateCommand> clone() const = 0;

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
    virtual MarchingStyle StepStyle() { return STYLE_HighStep; }

    // when we want to have the path drawn:
    virtual DrawCommand GenCC_DrawCommand(const Coord& pt, const Coord& offset) const;

    /*!
     * @brief json  that represent this movement in an Online Viewer '.viewer' file.
     * @param start The position at which this movement begins.
     */
    virtual nlohmann::json toOnlineViewerJSON(const Coord& start) const = 0;

protected:
    unsigned mNumBeats;
    unsigned mBeat;
};

class AnimateCommandMT : public AnimateCommand {
public:
    AnimateCommandMT(unsigned beats, float direction);
    virtual ~AnimateCommandMT() = default;

    std::unique_ptr<AnimateCommand> clone() const override;

    AnimateDir Direction() const override;
    float RealDirection() const override;

    nlohmann::json toOnlineViewerJSON(const Coord& start) const override;

protected:
    AnimateDir dir;
    float realdir;
};

class AnimateCommandMove : public AnimateCommandMT {
public:
    AnimateCommandMove(unsigned beats, Coord movement);
    AnimateCommandMove(unsigned beats, Coord movement, float direction);
    virtual ~AnimateCommandMove() = default;

    std::unique_ptr<AnimateCommand> clone() const override;

    bool NextBeat(Coord& pt) override;
    bool PrevBeat(Coord& pt) override;

    void ApplyForward(Coord& pt) override;
    void ApplyBackward(Coord& pt) override;

    float MotionDirection() const override;
    void ClipBeats(unsigned beats) override;

    DrawCommand GenCC_DrawCommand(const Coord& pt, const Coord& offset) const override;

    nlohmann::json toOnlineViewerJSON(const Coord& start) const override;

private:
    Coord mVector;
};

class AnimateCommandRotate : public AnimateCommand {
public:
    AnimateCommandRotate(unsigned beats, Coord cntr, float rad, float ang1,
        float ang2, bool backwards = false);
    virtual ~AnimateCommandRotate() = default;

    std::unique_ptr<AnimateCommand> clone() const override;

    bool NextBeat(Coord& pt) override;
    bool PrevBeat(Coord& pt) override;

    void ApplyForward(Coord& pt) override;
    void ApplyBackward(Coord& pt) override;

    AnimateDir Direction() const override;
    float RealDirection() const override;
    void ClipBeats(unsigned beats) override;

    DrawCommand GenCC_DrawCommand(const Coord& pt, const Coord& offset) const override;

    nlohmann::json toOnlineViewerJSON(const Coord& start) const override;

private:
    Coord mOrigin;
    float mR, mAngStart, mAngEnd;
    float mFace;
};
}
