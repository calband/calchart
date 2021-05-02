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

#include "CalChartAnimationCompile.h"
#include "CalChartAnimationErrors.h"
#include "CalChartAnimationCommand.h"
#include "cc_sheet.h"
#include "cont.h"

namespace CalChart {

AnimationCompile::AnimationCompile(const Show& show, SYMBOL_TYPE cont_symbol, unsigned pt_num, Show::const_Sheet_iterator_t c_sheet, AnimateState& state)
    : mShow(show)
    , contsymbol(cont_symbol)
    , curr_pt(pt_num)
    , curr_sheet(c_sheet)
    , mState(state)
{
}

AnimationCommands
AnimationCompile::Compile(
    const Show& show, AnimationVariables& variablesStates,
    AnimationErrors& errors, Show::const_Sheet_iterator_t c_sheet,
    unsigned pt_num, SYMBOL_TYPE cont_symbol,
    std::vector<std::unique_ptr<ContProcedure>> const& procs)
{
    AnimateState state{
        c_sheet->GetPosition(pt_num),
        c_sheet->GetBeats(),
        variablesStates,
        errors,
        {},
    };

    AnimationCompile ac(show, cont_symbol, pt_num, c_sheet, state);

    if (procs.empty()) {
        // no continuity was specified
        auto s = c_sheet + 1;
        for (; s != show.GetSheetEnd(); ++s) {
            if (s->IsInAnimation()) {
                // use EVEN REM NP
                ContProcEven defcont(new ContValueFloat(state.beats_rem),
                    new ContNextPoint());
                defcont.Compile(ac);
                break;
            }
        }
        if (s == show.GetSheetEnd()) {
            // use MTRM E
            ContProcMTRM defcont(new ContValueDefined(CC_E));
            defcont.Compile(ac);
        }
    }

    for (auto& proc : procs) {
        proc->Compile(ac);
    }

    if ((c_sheet + 1) != show.GetSheetEnd()) {
        auto next_point = (c_sheet + 1)->GetPosition(pt_num);
        if (state.pt != next_point) {
            auto c = next_point - state.pt;
            ac.RegisterError(ANIMERR_WRONGPLACE, NULL);
            ac.Append(std::make_shared<AnimationCommandMove>(state.beats_rem, c), NULL);
        }
    }
    if (state.beats_rem) {
        ac.RegisterError(ANIMERR_EXTRATIME, NULL);
        ac.Append(std::make_shared<AnimationCommandMT>(state.beats_rem, ANIMDIR_E), NULL);
    }
    return state.cmds;
}

bool AnimationCompile::Append(std::shared_ptr<AnimationCommand> cmd,
    const ContToken* token)
{
    if (mState.beats_rem < cmd->NumBeats()) {
        RegisterError(ANIMERR_OUTOFTIME, token);
        if (mState.beats_rem == 0) {
            return false;
        }
        cmd->ClipBeats(mState.beats_rem);
    }
    mState.cmds.push_back(cmd);
    mState.beats_rem -= cmd->NumBeats();

    cmd->ApplyForward(mState.pt); // Move current point to new position
    SetVarValue(CONTVAR_DOF, cmd->MotionDirection());
    SetVarValue(CONTVAR_DOH, cmd->RealDirection());
    return true;
}

void AnimationCompile::RegisterError(AnimateError err,
    const ContToken* token) const
{
    mState.error_markers.RegisterError(err, token, curr_pt, contsymbol);
}

float AnimationCompile::GetVarValue(int varnum, const ContToken* token) const
{
    auto i = mState.mVars[varnum].find(curr_pt);
    if (i != mState.mVars[varnum].end()) {
        return i->second;
    }
    RegisterError(ANIMERR_UNDEFINED, token);
    return 0.0;
}

void AnimationCompile::SetVarValue(int varnum, float value)
{
    mState.mVars[varnum][curr_pt] = value;
}

Coord AnimationCompile::GetStartingPosition() const
{
    return curr_sheet->GetPosition(GetCurrentPoint());
}

Coord AnimationCompile::GetEndingPosition(const ContToken* token) const
{
    auto sheet = curr_sheet + 1;

    while (1) {
        if (sheet == mShow.GetSheetEnd()) {
            RegisterError(ANIMERR_UNDEFINED, token);
            return GetPointPosition();
        }
        if (sheet->IsInAnimation()) {
            return sheet->GetPosition(GetCurrentPoint());
        }
        ++sheet;
    }
}

Coord AnimationCompile::GetReferencePointPosition(unsigned refnum) const
{
    return curr_sheet->GetPosition(GetCurrentPoint(), refnum + 1);
}

}
