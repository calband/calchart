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
#include "CalChartRanges.h"
#include "viewer_translate.h"
#include <nlohmann/json.hpp>

namespace CalChart::Animate {

auto CommandStill::GenCC_DrawCommand() const -> Draw::DrawCommand
{
    return Draw::Ignore{};
}

auto CommandStill::toOnlineViewerJSON() const -> nlohmann::json
{
    auto start = mStart;

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

auto CommandMove::PositionAtBeat(unsigned beat) const -> Coord
{
    auto start = mStart;
    auto numBeats = NumBeats();

    if (numBeats == 0) {
        return start;
    }
    return start + (mMovement * beat) / numBeats;
}

auto CommandMove::GenCC_DrawCommand() const -> Draw::DrawCommand
{
    auto start = mStart;
    return Draw::Line{ start, start + mMovement };
}

auto CommandMove::toOnlineViewerJSON() const -> nlohmann::json
{
    auto start = mStart;
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

auto CommandRotate::PositionAtBeat(unsigned beat) const -> Coord
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
    auto start = PositionAtBeat(0);
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
    auto GetBeatsPerCont(Range range)
    {
        return range | std::views::transform([](auto cmd) { return NumBeats(cmd); });
    }

    template <std::ranges::input_range Range>
        requires(std::is_convertible_v<std::ranges::range_value_t<Range>, CalChart::Animate::Command>)
    auto GetRunningBeats(Range range)
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

namespace {
    auto LeadingBeatsAre0(std::vector<Command> const& commands) -> size_t
    {
        return std::distance(
            commands.begin(),
            std::ranges::find_if(commands.begin(), commands.end(), [](auto&& cmd) {
                return NumBeats(cmd) != 0;
            }));
    }

}

auto Commands::BeatToCommandOffsetAndBeat(unsigned beat) const -> std::pair<std::pair<size_t, beats_t>, std::pair<size_t, beats_t>>
{
    // count the number of 0s
    auto leadingBeats = LeadingBeatsAre0(mCommands);
    if (LeadingBeatsAre0(mCommands) > 0 && beat == 0) {
        return { { 1, 0 }, { 0, 0 } };
    }
    beat -= leadingBeats;
    auto where = std::ranges::find_if(mRunningBeatCount, [beat](auto thisBeat) { return beat < thisBeat; });
    if (where == mRunningBeatCount.end()) {
        return { { mRunningBeatCount.size(), beat - TotalBeats() }, { mRunningBeatCount.size(), beat - TotalBeats() } };
    }
    auto index = std::distance(mRunningBeatCount.begin(), where);
    auto sheetIndex = std::distance(mRunningBeatCount.begin(), where);
    auto thisBeat = beat - (*where - NumBeats(mCommands.at(index)));
    // any sort of index makeup, we do here.
    if (thisBeat == 0 && index > 0 && leadingBeats == 0) {
        if (NumBeats(mCommands.at(index - 1)) == 0) {
            sheetIndex -= 1;
        }
    }

    return { { index, thisBeat }, { sheetIndex, thisBeat } };
}

namespace {
    // We treat position and facing because of very odd 0 beat continuities
    // These are artifacts of micro-position inconsistencies, where what can
    // happen is that to make up where the marcher is there can be 0 beat
    // continuities.  CalChart treats this like you've pivoted that direction --
    // that the direction you are facing is of the next beat but the position
    // is of the previous beat.
    auto MarcherInfoAtBeat(
        Command const& cmd,
        beats_t beat,
        Command const& facingCmd,
        beats_t facingBeat)
        -> MarcherInfo
    {
        return {
            PositionAtBeat(cmd, beat),
            CalChart::Radian{ FacingDirectionAtBeat(facingCmd, facingBeat) },
            StepStyle(cmd)
        };
    }
}

auto Commands::MarcherInfoAtBeat(beats_t beat) const -> MarcherInfo
{
    auto [position, facing] = BeatToCommandOffsetAndBeat(beat);
    auto [which, newBeat] = position;
    auto [whichFacing, newBeatFacing] = facing;
    if (which >= mCommands.size()) {
        return {};
    }
    return Animate::MarcherInfoAtBeat(
        mCommands.at(which),
        newBeat,
        mCommands.at(whichFacing),
        newBeatFacing);
}

auto Commands::GeneratePathToDraw(Coord::units endRadius) const -> std::vector<Draw::DrawCommand>
{
    auto drawCommands = CalChart::Ranges::ToVector<Draw::DrawCommand>(
        mCommands | std::views::transform([](auto&& cmd) {
            return GenCC_DrawCommand(cmd);
        }));
    auto position = End(mCommands.back());
    // now at this point we should put in a circle for end point
    drawCommands.emplace_back(CalChart::Draw::Circle{ position, endRadius, true });
    return drawCommands;
}

auto Commands::toOnlineViewerJSON() const -> std::vector<nlohmann::json>
{
    return CalChart::Ranges::ToVector<nlohmann::json>(
        mCommands | std::views::transform([](auto&& cmd) { return CalChart::Animate::toOnlineViewerJSON(cmd); }));
}
}
