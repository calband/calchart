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

#include "CalChartAnimationCommand.h"
#include "CalChartAngles.h"
#include "CalChartAnimation.h"
#include "CalChartCoord.h"
#include "CalChartDrawCommand.h"
#include "CalChartRanges.h"
#include "viewer_translate.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace CalChart {

namespace {
    auto ToAnimateStillStyle(AnimationCommandStill::Style style) -> Animate::CommandStill::Style
    {
        switch (style) {
        case AnimationCommandStill::Style::MarkTime:
            return Animate::CommandStill::Style::MarkTime;
        case AnimationCommandStill::Style::StandAndPlay:
            return Animate::CommandStill::Style::StandAndPlay;
        case AnimationCommandStill::Style::Close:
            return Animate::CommandStill::Style::Close;
        }
    }
}

AnimationCommand::AnimationCommand(unsigned beats)
    : mNumBeats(beats)
    , mBeat(0)
{
}

auto AnimationCommand::GenCC_DrawCommand(Coord /*pt*/) const -> Draw::DrawCommand
{
    return Draw::Ignore{};
}

auto AnimationCommand::Begin(Coord& pt) -> bool
{
    mBeat = 0;
    if (mNumBeats == 0) {
        ApplyForward(pt);
        return false;
    }
    return true;
}

auto AnimationCommand::End(Coord& pt) -> bool
{
    mBeat = mNumBeats;
    if (mNumBeats == 0) {
        ApplyBackward(pt);
        return false;
    }
    return true;
}

auto AnimationCommand::NextBeat([[maybe_unused]] Coord& pt) -> bool
{
    ++mBeat;
    return mBeat < mNumBeats;
}

auto AnimationCommand::PrevBeat([[maybe_unused]] Coord& pt) -> bool
{
    if (mBeat == 0) {
        return false;
    }
    --mBeat;
    return true;
}

void AnimationCommand::ApplyForward([[maybe_unused]] Coord& pt) { mBeat = mNumBeats; }

void AnimationCommand::ApplyBackward([[maybe_unused]] Coord& pt) { mBeat = 0; }

auto AnimationCommand::MotionDirection() const -> CalChart::Degree { return FacingDirection(); }

void AnimationCommand::ClipBeats(unsigned beats) { mNumBeats = beats; }

AnimationCommandStill::AnimationCommandStill(Style style, unsigned beats, CalChart::Degree direction)
    : AnimationCommand(beats)
    , dir(direction)
    , mStyle{ style }
{
}

auto AnimationCommandStill::clone() const -> std::unique_ptr<AnimationCommand>
{
    return std::make_unique<AnimationCommandStill>(*this);
}

auto AnimationCommandStill::FacingDirection() const -> CalChart::Degree { return dir; }

auto AnimationCommandStill::toOnlineViewerJSON(Coord start) const -> nlohmann::json
{
    nlohmann::json j;

    j["type"] = [&]() {
        switch (mStyle) {
        case Style::MarkTime:
            return "mark";
        case Style::StandAndPlay:
            return "stand";
        case Style::Close:
            return "close";
        }
    }();
    j["beats"] = static_cast<double>(NumBeats());
    j["facing"] = ToOnlineViewer::angle(FacingDirection());
    j["x"] = ToOnlineViewer::xPosition(start.x);
    j["y"] = ToOnlineViewer::yPosition(start.y);
    return j;
}

auto AnimationCommandStill::ToAnimateCommand(Coord start) const -> Animate::Command
{
    return Animate::CommandStill{ start, mNumBeats, ToAnimateStillStyle(mStyle), FacingDirection() };
}

AnimationCommandMove::AnimationCommandMove(unsigned beats, Coord movement)
    : AnimationCommand(beats)
    , dir{ movement.Direction() }
    , mVector(movement)
{
}

auto AnimationCommandMove::clone() const -> std::unique_ptr<AnimationCommand>
{
    return std::make_unique<AnimationCommandMove>(*this);
}

AnimationCommandMove::AnimationCommandMove(unsigned beats, Coord movement, CalChart::Degree direction)
    : AnimationCommand(beats)
    , dir{ direction }
    , mVector(movement)
{
}

auto AnimationCommandMove::NextBeat(Coord& pt) -> bool
{
    auto b = AnimationCommand::NextBeat(pt);
    if (mNumBeats == 0) {
        return b;
    }
    pt.x += static_cast<Coord::units>((mNumBeats)
            ? ((long)mBeat * mVector.x / (short)mNumBeats) - ((long)(mBeat - 1) * mVector.x / (short)mNumBeats)
            : 0);
    pt.y += static_cast<Coord::units>((mNumBeats)
            ? ((long)mBeat * mVector.y / (short)mNumBeats) - ((long)(mBeat - 1) * mVector.y / (short)mNumBeats)
            : 0);
    return b;
}

auto AnimationCommandMove::PrevBeat(Coord& pt) -> bool
{
    if (AnimationCommand::PrevBeat(pt)) {
        pt.x += static_cast<Coord::units>(mNumBeats
                ? ((long)mBeat * mVector.x / (short)mNumBeats) - ((long)(mBeat + 1) * mVector.x / (short)mNumBeats)
                : 0);
        pt.y += static_cast<Coord::units>(mNumBeats
                ? ((long)mBeat * mVector.y / (short)mNumBeats) - ((long)(mBeat + 1) * mVector.y / (short)mNumBeats)
                : 0);
        return true;
    }
    return false;
}

void AnimationCommandMove::ApplyForward(Coord& pt)
{
    AnimationCommand::ApplyForward(pt);
    pt += mVector;
}

void AnimationCommandMove::ApplyBackward(Coord& pt)
{
    AnimationCommand::ApplyBackward(pt);
    pt -= mVector;
}

auto AnimationCommandMove::FacingDirection() const -> CalChart::Degree { return dir; }

auto AnimationCommandMove::MotionDirection() const -> CalChart::Degree
{
    return CalChart::Degree{ mVector.Direction() };
}

void AnimationCommandMove::ClipBeats(unsigned beats)
{
    AnimationCommand::ClipBeats(beats);
}

auto AnimationCommandMove::GenCC_DrawCommand(Coord pt) const -> Draw::DrawCommand
{
    return Draw::Line{ pt, pt + mVector };
}

auto AnimationCommandMove::toOnlineViewerJSON(Coord start) const -> nlohmann::json
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

auto AnimationCommandMove::ToAnimateCommand(Coord start) const -> Animate::Command
{
    return Animate::CommandMove{ start, NumBeats(), mVector, FacingDirection() };
}

AnimationCommandRotate::AnimationCommandRotate(
    unsigned beats,
    Coord cntr,
    float radius,
    CalChart::Degree ang1,
    CalChart::Degree ang2,
    bool backwards)
    : AnimationCommand(beats)
    , mOrigin(cntr)
    , mRadius(radius)
    , mAngStart(ang1)
    , mAngEnd(ang2)
    , mFace(backwards ? -90 : 90)
{
}

auto AnimationCommandRotate::clone() const -> std::unique_ptr<AnimationCommand>
{
    return std::make_unique<AnimationCommandRotate>(*this);
}

auto AnimationCommandRotate::NextBeat(Coord& pt) -> bool
{
    auto b = AnimationCommand::NextBeat(pt);
    auto curr_ang = mNumBeats > 0 ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart) : mAngStart;
    pt.x = RoundToCoordUnits(mOrigin.x + cos(curr_ang) * mRadius);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(curr_ang) * mRadius);
    return b;
}

auto AnimationCommandRotate::PrevBeat(Coord& pt) -> bool
{
    if (AnimationCommand::PrevBeat(pt)) {
        auto curr_ang = mNumBeats > 0 ? ((mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart) : mAngStart;
        pt.x = RoundToCoordUnits(mOrigin.x + cos(curr_ang) * mRadius);
        pt.y = RoundToCoordUnits(mOrigin.y - sin(curr_ang) * mRadius);
        return true;
    }
    return false;
}

void AnimationCommandRotate::ApplyForward(Coord& pt)
{
    AnimationCommand::ApplyForward(pt);
    pt.x = RoundToCoordUnits(mOrigin.x + cos(mAngEnd) * mRadius);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(mAngEnd) * mRadius);
}

void AnimationCommandRotate::ApplyBackward(Coord& pt)
{
    AnimationCommand::ApplyBackward(pt);
    pt.x = RoundToCoordUnits(mOrigin.x + cos(mAngStart) * mRadius);
    pt.y = RoundToCoordUnits(mOrigin.y - sin(mAngStart) * mRadius);
}

auto AnimationCommandRotate::FacingDirection() const -> CalChart::Degree
{
    auto curr_ang = mNumBeats > 0 ? (mAngEnd - mAngStart) * mBeat / mNumBeats + mAngStart : mAngStart;
    if (mAngEnd > mAngStart) {
        return curr_ang + mFace;
    }
    return curr_ang - mFace;
}

void AnimationCommandRotate::ClipBeats(unsigned beats)
{
    AnimationCommand::ClipBeats(beats);
}

auto AnimationCommandRotate::GenCC_DrawCommand([[maybe_unused]] Coord pt) const -> Draw::DrawCommand
{
    auto start = (mAngStart < mAngEnd) ? mAngStart : mAngEnd;
    auto end = (mAngStart < mAngEnd) ? mAngEnd : mAngStart;
    auto x_start = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.x + cos(start) * mRadius));
    auto y_start = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.y - sin(start) * mRadius));
    auto x_end = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.x + cos(end) * mRadius));
    auto y_end = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.y - sin(end) * mRadius));

    return Draw::Arc{
        { x_start, y_start },
        { x_end, y_end },
        mOrigin
    };
}

auto AnimationCommandRotate::toOnlineViewerJSON(Coord start) const -> nlohmann::json
{
    nlohmann::json j;

    j["type"] = "arc";
    j["start_x"] = ToOnlineViewer::xPosition(start.x);
    j["start_y"] = ToOnlineViewer::yPosition(start.y);
    j["center_x"] = ToOnlineViewer::xPosition(mOrigin.x);
    j["center_y"] = ToOnlineViewer::yPosition(mOrigin.y);
    j["angle"] = (-(mAngEnd - mAngStart)).getValue();
    j["beats"] = static_cast<double>(NumBeats());
    j["beats_per_step"] = static_cast<double>(1);
    j["facing_offset"] = (-mFace + CalChart::Degree{ 90 }).getValue();
    return j;
}

auto AnimationCommandRotate::ToAnimateCommand(Coord start) const -> Animate::Command
{
    return Animate::CommandRotate{ start, mNumBeats, mOrigin, mRadius, mAngStart, mAngEnd, mFace };
}

}

namespace CalChart::Animate {

auto CommandStill::GenCC_DrawCommand() const -> Draw::DrawCommand
{
    return Draw::Ignore{};
}

auto CommandStill::toOnlineViewerJSON() const -> nlohmann::json
{
    auto start = Start();

    nlohmann::json j;
    j["type"] = [&]() {
        switch (mStyle) {
        case Style::MarkTime:
            return "mark";
        case Style::StandAndPlay:
            return "stand";
        case Style::Close:
            return "close";
        }
    }();
    j["beats"] = static_cast<double>(NumBeats());
    j["facing"] = ToOnlineViewer::angle(FacingDirectionAtBeat(0));
    j["x"] = ToOnlineViewer::xPosition(start.x);
    j["y"] = ToOnlineViewer::yPosition(start.y);
    return j;
}

auto CommandMove::PositionAtBeat(unsigned beat) -> Coord
{
    auto start = Start();
    auto numBeats = NumBeats();

    if (numBeats == 0) {
        return start;
    }
    return start + (mMovement * beat) / numBeats;
}

auto CommandMove::GenCC_DrawCommand() const -> Draw::DrawCommand
{
    auto start = Start();
    return Draw::Line{ start, start + mMovement };
}

auto CommandMove::toOnlineViewerJSON() const -> nlohmann::json
{
    auto start = Start();
    nlohmann::json j;

    j["type"] = "even";
    j["beats"] = static_cast<double>(NumBeats());
    j["beats_per_step"] = static_cast<double>(1);
    j["x1"] = ToOnlineViewer::xPosition(start.x);
    j["y1"] = ToOnlineViewer::yPosition(start.y);
    j["x2"] = ToOnlineViewer::xPosition(start.x + mMovement.x);
    j["y2"] = ToOnlineViewer::yPosition(start.y + mMovement.y);
    j["facing"] = ToOnlineViewer::angle(MotionDirectionAtBeat(0));
    return j;
}

CommandRotate::CommandRotate(
    unsigned beats,
    Coord cntr,
    float radius,
    CalChart::Degree ang1,
    CalChart::Degree ang2,
    bool backwards)
    : CommandRotate{
        Coord{ RoundToCoordUnits(cntr.x + cos(ang1) * radius), RoundToCoordUnits(cntr.y - sin(ang1) * radius) },
        beats,
        cntr,
        radius,
        ang1,
        ang2,
        backwards ? CalChart::Degree{ -90 } : CalChart::Degree{ 90 }
    }
{
}

auto CommandRotate::End() const -> Coord
{
    auto start = Coord{};
    start.x += RoundToCoordUnits(mOrigin.x + cos(mAngEnd) * mRadius);
    start.y += RoundToCoordUnits(mOrigin.y - sin(mAngEnd) * mRadius);
    return start;
}

auto CommandRotate::PositionAtBeat(unsigned beat) -> Coord
{
    auto numBeats = NumBeats();
    auto curr_ang = numBeats > 0 ? ((mAngEnd - mAngStart) * beat / numBeats + mAngStart) : mAngStart;
    return Coord{ RoundToCoordUnits(mOrigin.x + cos(curr_ang) * mRadius), RoundToCoordUnits(mOrigin.y - sin(curr_ang) * mRadius) };
}

auto CommandRotate::FacingDirectionAtBeat(unsigned beat) const -> CalChart::Degree
{
    auto numBeats = NumBeats();
    auto curr_ang = numBeats > 0 ? (mAngEnd - mAngStart) * beat / numBeats + mAngStart : mAngStart;
    if (mAngEnd > mAngStart) {
        return curr_ang + mFace;
    }
    return curr_ang - mFace;
}

auto CommandRotate::GenCC_DrawCommand() const -> Draw::DrawCommand
{
    auto start = (mAngStart < mAngEnd) ? mAngStart : mAngEnd;
    auto end = (mAngStart < mAngEnd) ? mAngEnd : mAngStart;
    auto x_start = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.x + cos(start) * mRadius));
    auto y_start = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.y - sin(start) * mRadius));
    auto x_end = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.x + cos(end) * mRadius));
    auto y_end = static_cast<CalChart::Coord::units>(RoundToCoordUnits(mOrigin.y - sin(end) * mRadius));

    return Draw::Arc{
        { x_start, y_start },
        { x_end, y_end },
        mOrigin
    };
}

auto CommandRotate::toOnlineViewerJSON() const -> nlohmann::json
{
    auto start = Start();
    nlohmann::json j;
    j["type"] = "arc";
    j["start_x"] = ToOnlineViewer::xPosition(start.x);
    j["start_y"] = ToOnlineViewer::yPosition(start.y);
    j["center_x"] = ToOnlineViewer::xPosition(mOrigin.x);
    j["center_y"] = ToOnlineViewer::yPosition(mOrigin.y);
    j["angle"] = (-(mAngEnd - mAngStart)).getValue();
    j["beats"] = static_cast<double>(NumBeats());
    j["beats_per_step"] = static_cast<double>(1);
    j["facing_offset"] = (-mFace + CalChart::Degree{ 90 }).getValue();
    return j;
}

namespace {
    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Command>)
    auto GetBeatsPerCont(Range&& range)
    {
        return range | std::views::transform([](auto cmd) { return NumBeats(cmd); });
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Command>)
    auto GetRunningBeats(Range&& range)
    {
        auto allBeats = CalChart::Ranges::ToVector<beats_t>(GetBeatsPerCont(range));
        auto running = std::vector<beats_t>(allBeats.size());
        std::inclusive_scan(allBeats.begin(), allBeats.end(), running.begin());
        return running;
    }
}

Commands::Commands(std::vector<Command> const& commands)
    : mCommands(commands)
    , mRunningBeatCount{ GetRunningBeats(commands) }
{
}

auto Commands::TotalBeats() const -> beats_t
{
    if (mRunningBeatCount.empty()) {
        return 0;
    }
    return mRunningBeatCount.back();
}

auto Commands::BeatToCommandOffsetAndBeat(unsigned beat) const -> std::tuple<size_t, beats_t>
{
    auto where = std::ranges::find_if(mRunningBeatCount, [beat](auto thisBeat) { return beat < thisBeat; });
    if (where == mRunningBeatCount.end()) {
        return { mRunningBeatCount.size(), beat - TotalBeats() };
    }
    auto index = std::distance(mRunningBeatCount.begin(), where);
    return { index, beat - (*where - NumBeats(mCommands.at(index))) };
}

auto Commands::MarcherInfoAtBeat(beats_t beat) const -> MarcherInfo
{
    auto [which, newBeat] = BeatToCommandOffsetAndBeat(beat);
    if (which >= mCommands.size()) {
        return {};
    }
    return Animate::MarcherInfoAtBeat(mCommands.at(which), newBeat);
}

}
