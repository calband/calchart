/*
 * CalChartAnimationCompile.cpp
 * Classes for compiling the animation command
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

#include "CalChartAnimationCompile.h"
#include "CalChartAnimationCommand.h"
#include "CalChartAnimationErrors.h"
#include "CalChartSheet.h"

namespace CalChart {

struct AnimationCompileState : public AnimationCompile {
    AnimationCompileState(SYMBOL_TYPE cont_symbol, unsigned whichMarcher, Point point, beats_t beats, std::optional<Coord> endPosition, AnimationVariables& variablesStates, AnimationErrors& errors);

    [[nodiscard]] auto Append(Animate::Command cmd) -> bool override;
    void RegisterError(AnimateError err) const override { RegisterAnimationError(mErrors, err, mWhichMarcher, contsymbol); }

    [[nodiscard]] auto GetVarValue(Cont::Variable varnum) const -> float override;
    void SetVarValue(Cont::Variable varnum, float value) override { mVars.at(toUType(varnum))[mWhichMarcher] = value; }

    [[nodiscard]] auto GetPointPosition() const -> Coord override { return mWhichPos; }
    [[nodiscard]] auto GetStartingPosition() const -> Coord override { return mPoint.GetPos(0); }
    [[nodiscard]] auto GetEndingPosition() const -> Coord override;
    [[nodiscard]] auto GetReferencePointPosition(unsigned refnum) const -> Coord override { return mPoint.GetPos(refnum); }
    [[nodiscard]] auto GetCurrentPoint() const -> unsigned override { return mWhichMarcher; }
    [[nodiscard]] auto GetBeatsRemaining() const -> unsigned override { return mBeatsRem; }

    [[nodiscard]] auto GetCommands() const { return mCmds; }

private:
    SYMBOL_TYPE contsymbol;
    unsigned mWhichMarcher;
    Point mPoint;
    Coord mWhichPos;
    unsigned mBeatsRem;
    std::optional<Coord> mEndPosition;
    AnimationVariables& mVars;
    AnimationErrors& mErrors;
    std::vector<Animate::Command> mCmds{};
};

auto Compile(
    AnimationVariables& variablesStates,
    AnimationErrors& errors,
    unsigned whichMarcher,
    SYMBOL_TYPE cont_symbol,
    Point point,
    beats_t beats,
    bool isLastAnimationSheet,
    std::optional<Coord> endPosition,
    std::optional<Coord> nextPosition,
    std::vector<std::unique_ptr<Cont::Procedure>> const& procs) -> AnimationCompileResult
{
    AnimationCompileState ac(cont_symbol, whichMarcher, point, beats, endPosition, variablesStates, errors);

    // no continuity was specified
    if (procs.empty()) {
        if (isLastAnimationSheet) {
            // use MTRM E
            Cont::ProcMTRM defcont(std::make_unique<Cont::ValueDefined>(Cont::CC_E));
            defcont.Compile(ac);
        } else {
            // use EVEN REM NP
            Cont::ProcEven defcont(std::make_unique<Cont::ValueFloat>(ac.GetBeatsRemaining()), std::make_unique<Cont::NextPoint>());
            defcont.Compile(ac);
        }
    }

    // compile all the commands
    for (auto const& proc : procs) {
        proc->Compile(ac);
    }

    // report if the point didn't make it
    if (nextPosition) {
        auto next_point = *nextPosition;
        if (ac.GetPointPosition() != next_point) {
            auto c = next_point - ac.GetPointPosition();
            ac.RegisterError(AnimateError::WRONGPLACE);
            ac.Append(Animate::CommandMove{ ac.GetPointPosition(), ac.GetBeatsRemaining(), c });
        }
    }

    // report if we have extra time.
    if (ac.GetBeatsRemaining()) {
        ac.RegisterError(AnimateError::EXTRATIME);
        ac.Append(Animate::CommandStill{ ac.GetPointPosition(), ac.GetBeatsRemaining(), Animate::CommandStill::Style::MarkTime, CalChart::Degree::East() });
    }

    return ac.GetCommands();
}

AnimationCompileState::AnimationCompileState(SYMBOL_TYPE cont_symbol, unsigned whichMarcher, Point point, beats_t beats, std::optional<Coord> endPosition, AnimationVariables& variablesStates, AnimationErrors& errors)
    : contsymbol(cont_symbol)
    , mWhichMarcher(whichMarcher)
    , mPoint{ point }
    , mWhichPos(mPoint.GetPos(0))
    , mBeatsRem(beats)
    , mEndPosition{ endPosition }
    , mVars(variablesStates)
    , mErrors(errors)
{
}

bool AnimationCompileState::Append(Animate::Command cmd)
{
    if (mBeatsRem < NumBeats(cmd)) {
        RegisterError(AnimateError::OUTOFTIME);
        if (mBeatsRem == 0) {
            return false;
        }
        cmd = WithBeats(cmd, mBeatsRem);
    }
    auto numBeats = NumBeats(cmd);
    mBeatsRem -= numBeats;

    mWhichPos = End(cmd); // Move current point to new position
    SetVarValue(Cont::Variable::DOF, MotionDirectionAtBeat(cmd, numBeats).getValue());
    SetVarValue(Cont::Variable::DOH, FacingDirectionAtBeat(cmd, numBeats).getValue());
    mCmds.push_back(cmd);
    return true;
}

float AnimationCompileState::GetVarValue(Cont::Variable varnum) const
{
    auto i = mVars[toUType(varnum)].find(mWhichMarcher);
    if (i != mVars[toUType(varnum)].end()) {
        return i->second;
    }
    RegisterError(AnimateError::UNDEFINED);
    return 0.0;
}

Coord AnimationCompileState::GetEndingPosition() const
{
    if (mEndPosition) {
        return *mEndPosition;
    } else {
        RegisterError(AnimateError::UNDEFINED);
        return GetPointPosition();
    }
}
}
