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
#include "CalChartAnimationErrors.h"
#include "CalChartAnimationCommand.h"
#include "CalChartSheet.h"
#include "CalChartContinuityToken.h"

namespace CalChart {

struct AnimationCompileState : public AnimationCompile {
    AnimationCompileState(SYMBOL_TYPE cont_symbol, unsigned pt_num, Show::const_Sheet_iterator_t c_sheet, Show::const_Sheet_iterator_t endSheet, AnimationVariables& variablesStates, AnimationErrors& errors);

    bool Append(std::unique_ptr<AnimationCommand> cmd, ContToken const* token) override;
    void RegisterError(AnimateError err, ContToken const* token) const override { mErrors.RegisterError(err, token, mWhichPoint, contsymbol); }

    float GetVarValue(int varnum, ContToken const* token) const override;
    void SetVarValue(int varnum, float value) override { mVars[varnum][mWhichPoint] = value; }

    virtual Coord GetPointPosition() const override { return mWhichPos; }
    virtual Coord GetStartingPosition() const override { return mCurrentSheet->GetPosition(GetCurrentPoint()); }
    virtual Coord GetEndingPosition(ContToken const* token) const override;
    virtual Coord GetReferencePointPosition(unsigned refnum) const override { return mCurrentSheet->GetPosition(GetCurrentPoint(), refnum + 1); }
    virtual unsigned GetCurrentPoint() const override { return mWhichPoint; }
    virtual unsigned GetBeatsRemaining() const override { return mBeatsRem; }

    auto GetCommands() const { return mCmds; }

private:
    SYMBOL_TYPE contsymbol;
    unsigned mWhichPoint;
    Coord mWhichPos;
    unsigned mBeatsRem;
    Show::const_Sheet_iterator_t mCurrentSheet;
    Show::const_Sheet_iterator_t mEndSheet;
    AnimationVariables& mVars;
    AnimationErrors& mErrors;
    AnimationCommands mCmds{};
};

AnimationCommands
Compile(
    AnimationVariables& variablesStates,
    AnimationErrors& errors,
    Show::const_Sheet_iterator_t c_sheet,
    Show::const_Sheet_iterator_t endSheet,
    unsigned pt_num,
    SYMBOL_TYPE cont_symbol,
    std::vector<std::unique_ptr<ContProcedure>> const& procs)
{
    AnimationCompileState ac(cont_symbol, pt_num, c_sheet, endSheet, variablesStates, errors);

    // no continuity was specified
    if (procs.empty()) {
        // find the first animation sheet.
        auto s = std::find_if(c_sheet + 1, endSheet, [](auto& sheet) { return sheet.IsInAnimation(); });

        if (s == endSheet) {
            // use MTRM E
            ContProcMTRM defcont(std::make_unique<ContValueDefined>(CC_E));
            defcont.Compile(ac);
        }
        else {
            // use EVEN REM NP
            ContProcEven defcont(std::make_unique<ContValueFloat>(ac.GetBeatsRemaining()), std::make_unique<ContNextPoint>());
            defcont.Compile(ac);
        }
    }

    // compile all the commands
    for (auto& proc : procs) {
        proc->Compile(ac);
    }

    // report if the point didn't make it
    if ((c_sheet + 1) != endSheet) {
        auto next_point = (c_sheet + 1)->GetPosition(pt_num);
        if (ac.GetPointPosition() != next_point) {
            auto c = next_point - ac.GetPointPosition();
            ac.RegisterError(AnimateError::WRONGPLACE, NULL);
            ac.Append(std::make_unique<AnimationCommandMove>(ac.GetBeatsRemaining(), c), NULL);
        }
    }

    // report if we have extra time.
    if (ac.GetBeatsRemaining()) {
        ac.RegisterError(AnimateError::EXTRATIME, NULL);
        ac.Append(std::make_unique<AnimationCommandMT>(ac.GetBeatsRemaining(), AnimateDir::E), NULL);
    }

    return ac.GetCommands();
}

AnimationCompileState::AnimationCompileState(SYMBOL_TYPE cont_symbol, unsigned pt_num, Show::const_Sheet_iterator_t c_sheet, Show::const_Sheet_iterator_t endSheet, AnimationVariables& variablesStates, AnimationErrors& errors)
    : contsymbol(cont_symbol)
    , mWhichPoint(pt_num)
    , mWhichPos(c_sheet->GetPosition(pt_num))
    , mBeatsRem(c_sheet->GetBeats())
    , mCurrentSheet(c_sheet)
    , mEndSheet(endSheet)
    , mVars(variablesStates)
    , mErrors(errors)
{
}

bool AnimationCompileState::Append(std::unique_ptr<AnimationCommand> cmd, ContToken const* token)
{
    if (mBeatsRem < cmd->NumBeats()) {
        RegisterError(AnimateError::OUTOFTIME, token);
        if (mBeatsRem == 0) {
            return false;
        }
        cmd->ClipBeats(mBeatsRem);
    }
    mBeatsRem -= cmd->NumBeats();

    cmd->ApplyForward(mWhichPos); // Move current point to new position
    SetVarValue(CONTVAR_DOF, cmd->MotionDirection());
    SetVarValue(CONTVAR_DOH, cmd->RealDirection());
    mCmds.emplace_back(cmd.release());
    return true;
}

float AnimationCompileState::GetVarValue(int varnum, const ContToken* token) const
{
    auto i = mVars[varnum].find(mWhichPoint);
    if (i != mVars[varnum].end()) {
        return i->second;
    }
    RegisterError(AnimateError::UNDEFINED, token);
    return 0.0;
}

Coord AnimationCompileState::GetEndingPosition(const ContToken* token) const
{
    auto sheet = mCurrentSheet + 1;

    while (1) {
        if (sheet == mEndSheet) {
            RegisterError(AnimateError::UNDEFINED, token);
            return GetPointPosition();
        }
        if (sheet->IsInAnimation()) {
            return sheet->GetPosition(GetCurrentPoint());
        }
        ++sheet;
    }
}

}
