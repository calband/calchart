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

#include "CalChartAngles.h"
#include "CalChartAnimationTypes.h"
#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"
#include <nlohmann/json.hpp>

/**
 * Animate::Command
 * With each part of a continuity we want to be able to determine the following:
 * Where the Marcher's position is at distinct points (Start, End, or any Beat)
 * Which direction they are moving/marching.
 * How many beats is that particular continuity.
 * What step type they are doing.
 *
 * Continuities can be broken down into 3 distinct types:
 *  Still: Marcher not moving, but facing a direction and having a step type
 *  Moving: A vector along which to be moving (indicating how far to move each point) and where to face
 *  Rotate: A point which to rotate, radius, start and end angles
 *
 * Animate::Command is a variant that is one of these objects.
 * We use variant here to have a way to treat these objects like values.
 */

namespace CalChart::Animate {

class CommandStill;
class CommandMove;
class CommandRotate;
using Command = std::variant<CommandStill, CommandMove, CommandRotate>;

using beats_t = unsigned;

struct MarcherInfo {
    CalChart::Coord mPosition{};
    CalChart::Radian mFacingDirection{};
    CalChart::MarchingStyle mStepStyle = CalChart::MarchingStyle::HighStep;
    friend auto operator==(MarcherInfo const& lhs, MarcherInfo const& rhs) -> bool
    {
        return lhs.mPosition == rhs.mPosition && lhs.mFacingDirection.IsEqual(rhs.mFacingDirection) && lhs.mStepStyle == rhs.mStepStyle;
    };
};

template <typename Command>
concept CommandT = requires(Command cmd, beats_t beats) {
    {
        cmd.End()
    } -> std::convertible_to<Coord>;
    {
        cmd.PositionAtBeat(beats)
    } -> std::convertible_to<Coord>;
    {
        cmd.FacingDirectionAtBeat(beats)
    } -> std::convertible_to<CalChart::Degree>;
    {
        cmd.MotionDirectionAtBeat(beats)
    } -> std::convertible_to<CalChart::Degree>;
    {
        cmd.NumBeats()
    } -> std::convertible_to<beats_t>;
    {
        cmd.StepStyle()
    } -> std::convertible_to<MarchingStyle>;
    {
        cmd.GenCC_DrawCommand()
    } -> std::convertible_to<Draw::DrawCommand>;
    {
        cmd.toOnlineViewerJSON()
    } -> std::convertible_to<nlohmann::json>;
    {
        cmd.WithBeats(beats)
    } -> std::convertible_to<Command>;
};

class CommandStill {
public:
    enum class Style {
        MarkTime,
        StandAndPlay,
        Close,
    };
    CommandStill(Coord start, beats_t beats, Style style, CalChart::Degree direction)
        : mStart{ start }
        , mNumBeats{ beats }
        , mDirection{ direction }
        , mStyle{ style }
    {
    }

    [[nodiscard]] auto WithBeats(beats_t beats) const -> CommandStill
    {
        return CommandStill{ mStart, beats, mStyle, mDirection };
    }

    [[nodiscard]] auto End() const -> Coord { return mStart; }
    [[nodiscard]] auto PositionAtBeat([[maybe_unused]] beats_t beat) const -> Coord { return mStart; }
    [[nodiscard]] auto FacingDirectionAtBeat([[maybe_unused]] beats_t beat) const { return mDirection; }
    [[nodiscard]] auto MotionDirectionAtBeat([[maybe_unused]] beats_t beat) const { return mDirection; }

    [[nodiscard]] auto NumBeats() const -> beats_t { return mNumBeats; }
    [[nodiscard]] auto StepStyle() const -> MarchingStyle
    {
        return mStyle == Style::MarkTime ? MarchingStyle::HighStep : MarchingStyle::Close;
    }

    [[nodiscard]] auto GenCC_DrawCommand() const -> Draw::DrawCommand;
    [[nodiscard]] auto toOnlineViewerJSON() const -> nlohmann::json;

    friend auto operator==(CommandStill const&, CommandStill const&) -> bool = default;

private:
    Coord mStart;
    beats_t mNumBeats;
    CalChart::Degree mDirection;
    Style mStyle = Style::MarkTime;
};

class CommandMove {
public:
    CommandMove(Coord start, beats_t beats, Coord movement)
        : CommandMove{ start, beats, movement, CalChart::Degree{ movement.Direction() } }
    {
    }
    CommandMove(Coord start, beats_t beats, Coord movement, CalChart::Degree direction)
        : mStart{ start }
        , mNumBeats{ beats }
        , mMovement{ movement }
        , mDirection{ direction }
    {
    }

    [[nodiscard]] auto WithBeats(beats_t beats) const -> CommandMove
    {
        return CommandMove{ mStart, beats, mMovement, mDirection };
    }

    [[nodiscard]] auto End() const -> Coord { return mStart + mMovement; }

    [[nodiscard]] auto PositionAtBeat(beats_t beat) const -> Coord;
    [[nodiscard]] auto FacingDirectionAtBeat([[maybe_unused]] beats_t beat) const
    {
        //        std::cout << "Asking for move facing at beat " << beat << " dir " << mDirection << " in radian: " << CalChart::Radian{ mDirection } << "\n";
        return mDirection;
    }
    [[nodiscard]] auto MotionDirectionAtBeat([[maybe_unused]] beats_t beat) const { return CalChart::Degree{ mMovement.Direction() }; }

    [[nodiscard]] auto NumBeats() const -> beats_t { return mNumBeats; }
    [[nodiscard]] auto StepStyle() const -> MarchingStyle { return MarchingStyle::HighStep; }

    [[nodiscard]] auto GenCC_DrawCommand() const -> Draw::DrawCommand;
    [[nodiscard]] auto toOnlineViewerJSON() const -> nlohmann::json;

    friend auto operator==(CommandMove const&, CommandMove const&) -> bool = default;

private:
    Coord mStart;
    beats_t mNumBeats;
    Coord mMovement;
    CalChart::Degree mDirection;
};

class CommandRotate {
public:
    CommandRotate(
        beats_t beats,
        Coord cntr,
        float radius,
        CalChart::Degree ang1,
        CalChart::Degree ang2,
        bool backwards = false);
    CommandRotate(
        Coord start,
        beats_t beats,
        Coord cntr,
        float radius,
        CalChart::Degree ang1,
        CalChart::Degree ang2,
        CalChart::Degree face)
        : mStart{ start }
        , mNumBeats{ beats }
        , mOrigin{ cntr }
        , mRadius{ radius }
        , mAngStart{ ang1 }
        , mAngEnd{ ang2 }
        , mFace{ face }
    {
    }

    [[nodiscard]] auto WithBeats(beats_t beats) const -> CommandRotate
    {
        return CommandRotate{ mStart, beats, mOrigin, mRadius, mAngStart, mAngEnd, mFace };
    }

    [[nodiscard]] auto End() const -> Coord;
    [[nodiscard]] auto PositionAtBeat([[maybe_unused]] beats_t beat) const -> Coord;
    [[nodiscard]] auto FacingDirectionAtBeat(beats_t beat) const -> CalChart::Degree;
    [[nodiscard]] auto MotionDirectionAtBeat(beats_t beat) const { return FacingDirectionAtBeat(beat); }

    [[nodiscard]] auto NumBeats() const -> beats_t { return mNumBeats; }
    [[nodiscard]] auto StepStyle() const -> MarchingStyle { return MarchingStyle::HighStep; }

    [[nodiscard]] auto GenCC_DrawCommand() const -> Draw::DrawCommand;
    [[nodiscard]] auto toOnlineViewerJSON() const -> nlohmann::json;

    friend auto operator==(CommandRotate const&, CommandRotate const&) -> bool = default;

private:
    Coord mStart;
    beats_t mNumBeats;
    Coord mOrigin;
    float mRadius;
    CalChart::Degree mAngStart;
    CalChart::Degree mAngEnd;
    CalChart::Degree mFace;
};

static_assert(CommandT<CommandStill>);
static_assert(CommandT<CommandMove>);
static_assert(CommandT<CommandRotate>);

using Command = std::variant<CommandStill, CommandMove, CommandRotate>;

inline auto End(Command const& cmd) -> Coord
{
    return std::visit([](auto arg) { return arg.End(); }, cmd);
}

inline auto PositionAtBeat(Command const& cmd, beats_t beats) -> Coord
{
    return std::visit([beats](auto arg) { return arg.PositionAtBeat(beats); }, cmd);
}

inline auto FacingDirectionAtBeat(Command const& cmd, beats_t beats) -> CalChart::Degree
{
    return std::visit([beats](auto arg) { return arg.FacingDirectionAtBeat(beats); }, cmd);
}

inline auto MotionDirectionAtBeat(Command const& cmd, beats_t beats) -> CalChart::Degree
{
    return std::visit([beats](auto arg) { return arg.MotionDirectionAtBeat(beats); }, cmd);
}

inline auto NumBeats(Command const& cmd) -> beats_t
{
    return std::visit([](auto arg) { return arg.NumBeats(); }, cmd);
}

inline auto StepStyle(Command const& cmd) -> MarchingStyle
{
    return std::visit([](auto arg) { return arg.StepStyle(); }, cmd);
}

inline auto GenCC_DrawCommand(Command const& cmd) -> Draw::DrawCommand
{
    return std::visit([](auto arg) { return arg.GenCC_DrawCommand(); }, cmd);
}

inline auto toOnlineViewerJSON(Command const& cmd) -> nlohmann::json
{
    return std::visit([](auto arg) { return arg.toOnlineViewerJSON(); }, cmd);
}

inline auto WithBeats(Command const& cmd, beats_t beats) -> Command
{
    return std::visit([beats](auto arg) { return Command{ arg.WithBeats(beats) }; }, cmd);
}

class Commands {
public:
    explicit Commands(std::vector<Command> const& commands);
    [[nodiscard]] auto TotalBeats() const -> beats_t;
    // which command, beat for position/step and which command,beat for facing
    [[nodiscard]] auto BeatToCommandOffsetAndBeat(beats_t beat) const -> std::pair<std::pair<size_t, beats_t>, std::pair<size_t, beats_t>>;
    [[nodiscard]] auto MarcherInfoAtBeat(beats_t beat) const -> MarcherInfo;
    [[nodiscard]] auto GeneratePathToDraw(Coord::units endRadius) const -> std::vector<Draw::DrawCommand>;
    [[nodiscard]] auto toOnlineViewerJSON() const -> std::vector<nlohmann::json>;

private:
    std::vector<Command> mCommands;
    std::vector<beats_t> mRunningBeatCount;
};

}
