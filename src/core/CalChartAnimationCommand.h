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

#include "CalChartAngles.h"
#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"
#include "CalChartUtils.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace CalChart {

class AnimationCommand {
public:
    explicit AnimationCommand(unsigned beats);
    virtual ~AnimationCommand() = default;
    AnimationCommand(AnimationCommand const&) = default;
    auto operator=(AnimationCommand const&) -> AnimationCommand& = default;
    AnimationCommand(AnimationCommand&&) = default;
    auto operator=(AnimationCommand&&) -> AnimationCommand& = default;

    [[nodiscard]] virtual auto clone() const -> std::unique_ptr<AnimationCommand> = 0;

    // returns false if end of command
    virtual auto Begin(Coord& pt) -> bool;
    virtual auto End(Coord& pt) -> bool;
    virtual auto NextBeat(Coord& pt) -> bool;
    virtual auto PrevBeat(Coord& pt) -> bool;

    // go through all beats at once
    virtual void ApplyForward(Coord& pt);
    virtual void ApplyBackward(Coord& pt);

    [[nodiscard]] virtual auto FacingDirection() const -> CalChart::Degree = 0;
    [[nodiscard]] virtual auto MotionDirection() const -> CalChart::Degree;
    virtual void ClipBeats(unsigned beats);

    [[nodiscard]] virtual auto NumBeats() const -> unsigned { return mNumBeats; }

    // What style to display
    [[nodiscard]] virtual auto StepStyle() const -> MarchingStyle { return MarchingStyle::HighStep; }

    // when we want to have the path drawn:
    [[nodiscard]] virtual auto GenCC_DrawCommand(Coord pt) const -> Draw::DrawCommand;

    /*!
     * @brief json  that represent this movement in an Online Viewer '.viewer' file.
     * @param start The position at which this movement begins.
     */
    [[nodiscard]] virtual auto toOnlineViewerJSON(Coord start) const -> nlohmann::json = 0;

protected:
    friend auto operator==(AnimationCommand const& lhs, AnimationCommand const& rhs) -> bool;
    [[nodiscard]] virtual auto is_equal(AnimationCommand const& other) const -> bool
    {
        return mNumBeats == other.mNumBeats && mBeat == other.mBeat;
    }

    unsigned mNumBeats;
    unsigned mBeat;
};

inline auto operator==(AnimationCommand const& lhs, AnimationCommand const& rhs) -> bool
{
    return (typeid(lhs) == typeid(rhs)) && lhs.is_equal(rhs);
}

class AnimationCommandStand : public AnimationCommand {
    using super = AnimationCommand;

public:
    AnimationCommandStand(unsigned beats, CalChart::Degree direction);
    ~AnimationCommandStand() override = default;
    AnimationCommandStand(AnimationCommandStand const&) = default;
    auto operator=(AnimationCommandStand const&) -> AnimationCommandStand& = default;
    AnimationCommandStand(AnimationCommandStand&&) = default;
    auto operator=(AnimationCommandStand&&) -> AnimationCommandStand& = default;

    [[nodiscard]] auto clone() const -> std::unique_ptr<AnimationCommand> override;
    [[nodiscard]] auto FacingDirection() const -> CalChart::Degree override;
    [[nodiscard]] auto toOnlineViewerJSON(Coord start) const -> nlohmann::json override;
    [[nodiscard]] auto StepStyle() const -> MarchingStyle override { return MarchingStyle::Close; }

protected:
    [[nodiscard]] auto is_equal(AnimationCommand const& other) const -> bool override
    {
        auto const* ptr = dynamic_cast<AnimationCommandStand const*>(&other);
        if (ptr == nullptr) {
            return false;
        }
        return super::is_equal(other) && dir.IsEqual(ptr->dir);
    }

    CalChart::Degree dir;
};

class AnimationCommandMT : public AnimationCommand {
    using super = AnimationCommand;

public:
    AnimationCommandMT(unsigned beats, CalChart::Degree direction);
    ~AnimationCommandMT() override = default;
    AnimationCommandMT(AnimationCommandMT const&) = default;
    auto operator=(AnimationCommandMT const&) -> AnimationCommandMT& = default;
    AnimationCommandMT(AnimationCommandMT&&) = default;
    auto operator=(AnimationCommandMT&&) -> AnimationCommandMT& = default;

    [[nodiscard]] auto clone() const -> std::unique_ptr<AnimationCommand> override;
    [[nodiscard]] auto FacingDirection() const -> CalChart::Degree override;
    [[nodiscard]] auto toOnlineViewerJSON(Coord start) const -> nlohmann::json override;

protected:
    [[nodiscard]] auto is_equal(AnimationCommand const& other) const -> bool override
    {
        auto const* ptr = dynamic_cast<AnimationCommandMT const*>(&other);
        if (ptr == nullptr) {
            return false;
        }
        return super::is_equal(other) && dir.IsEqual(ptr->dir);
    }

    CalChart::Degree dir;
};

class AnimationCommandMove : public AnimationCommandMT {
    using super = AnimationCommandMT;

public:
    AnimationCommandMove(unsigned beats, Coord movement);
    AnimationCommandMove(unsigned beats, Coord movement, CalChart::Degree direction);
    ~AnimationCommandMove() override = default;
    AnimationCommandMove(AnimationCommandMove const&) = default;
    auto operator=(AnimationCommandMove const&) -> AnimationCommandMove& = default;
    AnimationCommandMove(AnimationCommandMove&&) = default;
    auto operator=(AnimationCommandMove&&) -> AnimationCommandMove& = default;

    [[nodiscard]] auto clone() const -> std::unique_ptr<AnimationCommand> override;

    auto NextBeat(Coord& pt) -> bool override;
    auto PrevBeat(Coord& pt) -> bool override;

    void ApplyForward(Coord& pt) override;
    void ApplyBackward(Coord& pt) override;

    [[nodiscard]] auto MotionDirection() const -> CalChart::Degree override;
    void ClipBeats(unsigned beats) override;

    [[nodiscard]] auto GenCC_DrawCommand(Coord pt) const -> Draw::DrawCommand override;

    [[nodiscard]] auto toOnlineViewerJSON(Coord start) const -> nlohmann::json override;

private:
    [[nodiscard]] auto is_equal(AnimationCommand const& other) const -> bool override
    {
        auto const* ptr = dynamic_cast<AnimationCommandMove const*>(&other);
        if (ptr == nullptr) {
            return false;
        }
        return super::is_equal(other) && mVector == ptr->mVector;
    }

    Coord mVector;
};

class AnimationCommandRotate : public AnimationCommand {
    using super = AnimationCommand;

public:
    AnimationCommandRotate(
        unsigned beats,
        Coord cntr,
        float radius,
        CalChart::Degree ang1,
        CalChart::Degree ang2,
        bool backwards = false);
    ~AnimationCommandRotate() override = default;
    AnimationCommandRotate(AnimationCommandRotate const&) = default;
    auto operator=(AnimationCommandRotate const&) -> AnimationCommandRotate& = default;
    AnimationCommandRotate(AnimationCommandRotate&&) = default;
    auto operator=(AnimationCommandRotate&&) -> AnimationCommandRotate& = default;

    [[nodiscard]] auto clone() const -> std::unique_ptr<AnimationCommand> override;

    auto NextBeat(Coord& pt) -> bool override;
    auto PrevBeat(Coord& pt) -> bool override;

    void ApplyForward(Coord& pt) override;
    void ApplyBackward(Coord& pt) override;

    [[nodiscard]] auto FacingDirection() const -> CalChart::Degree override;
    void ClipBeats(unsigned beats) override;

    [[nodiscard]] auto GenCC_DrawCommand(Coord pt) const -> Draw::DrawCommand override;

    [[nodiscard]] auto toOnlineViewerJSON(Coord start) const -> nlohmann::json override;

private:
    [[nodiscard]] auto is_equal(AnimationCommand const& other) const -> bool override
    {
        auto const* ptr = dynamic_cast<AnimationCommandRotate const*>(&other);
        if (ptr == nullptr) {
            return false;
        }
        return super::is_equal(other)
            && mOrigin == ptr->mOrigin
            && IS_EQUAL(mRadius, ptr->mRadius)
            && mAngStart.IsEqual(ptr->mAngStart)
            && mAngEnd.IsEqual(ptr->mAngEnd)
            && mFace.IsEqual(ptr->mFace);
    }

    Coord mOrigin;
    float mRadius;
    CalChart::Degree mAngStart;
    CalChart::Degree mAngEnd;
    CalChart::Degree mFace;
};

}
