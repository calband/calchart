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

#include "animatecompile.h"
#include "cont.h"
#include "animatecommand.h"
#include "cc_sheet.h"

void
AnimationErrors::RegisterError(AnimateError err, const ContToken *token, unsigned curr_pt, SYMBOL_TYPE contsymbol)
{
	mErrorMarkers[err].contsymbol = contsymbol;
	if (token != NULL)
	{
		mErrorMarkers[err].line = token->line;
		mErrorMarkers[err].col = token->col;
	}
	mErrorMarkers[err].pntgroup.insert(curr_pt);
}



AnimateCompile::AnimateCompile(const CC_show& show, SYMBOL_TYPE cont_symbol, unsigned pt_num, CC_show::const_CC_sheet_iterator_t c_sheet, unsigned& beats_rem, CC_coord& pt, AnimationVariables& variablesStates, AnimationErrors& errors, AnimateCommands& cmds) :
mShow(show),
contsymbol(cont_symbol),
curr_pt(pt_num),
curr_sheet(c_sheet),
pt(pt),
beats_rem(beats_rem),
mVars(variablesStates),
cmds(cmds),
error_markers(errors)
{
}



AnimateCommands
AnimateCompile::Compile(const CC_show& show, AnimationVariables& variablesStates, AnimationErrors& errors, CC_show::const_CC_sheet_iterator_t c_sheet, unsigned pt_num, SYMBOL_TYPE cont_symbol, ContProcedure* proc)
{
	AnimateCommands cmds;
	unsigned beats_rem = c_sheet->GetBeats();
	auto pt = c_sheet->GetPosition(pt_num);
	AnimateCompile ac(show, cont_symbol, pt_num, c_sheet, beats_rem, pt, variablesStates, errors, cmds);

	if (proc == NULL)
	{
// no continuity was specified
		auto s = c_sheet + 1;
		for (; s != show.GetSheetEnd(); ++s)
		{
			if (s->IsInAnimation())
			{
//use EVEN REM NP
				ContProcEven defcont(new ContValueFloat(beats_rem), new ContNextPoint());
				defcont.Compile(&ac);
				break;
			}
		}
		if (s == show.GetSheetEnd())
		{
//use MTRM E
			ContProcMTRM defcont(new ContValueDefined(CC_E));
			defcont.Compile(&ac);
		}
	}

	for (; proc; proc = proc->next)
	{
		proc->Compile(&ac);
	}

	if ((c_sheet + 1) != show.GetSheetEnd())
	{
		auto next_point = (c_sheet + 1)->GetPosition(pt_num);
		if (pt != next_point)
		{
			auto c = next_point - pt;
			ac.RegisterError(ANIMERR_WRONGPLACE, NULL);
			ac.Append(std::make_shared<AnimateCommandMove>(beats_rem, c), NULL);
		}
	}
	if (beats_rem)
	{
		ac.RegisterError(ANIMERR_EXTRATIME, NULL);
		ac.Append(std::make_shared<AnimateCommandMT>(beats_rem, ANIMDIR_E), NULL);
	}
	return cmds;
}


bool AnimateCompile::Append(std::shared_ptr<AnimateCommand> cmd, const ContToken *token)
{
	bool clipped;

	if (beats_rem < cmd->NumBeats())
	{
		RegisterError(ANIMERR_OUTOFTIME, token);
		if (beats_rem == 0)
		{
			return false;
		}
		cmd->ClipBeats(beats_rem);
		clipped = true;
	}
	else
	{
		clipped = false;
	}
	cmds.push_back(cmd);
	beats_rem -= cmd->NumBeats();

	cmd->ApplyForward(pt);						  // Move current point to new position
	SetVarValue(CONTVAR_DOF, cmd->MotionDirection());
	SetVarValue(CONTVAR_DOH, cmd->RealDirection());
	return true;
}


void AnimateCompile::RegisterError(AnimateError err, const ContToken *token) const
{
	error_markers.RegisterError(err, token, curr_pt, contsymbol);
}


float AnimateCompile::GetVarValue(int varnum, const ContToken *token) const
{
	auto i = mVars[varnum].find(curr_pt);
	if (i != mVars[varnum].end())
	{
		return i->second;
	}
	RegisterError(ANIMERR_UNDEFINED, token);
	return 0.0;
}


void AnimateCompile::SetVarValue(int varnum, float value)
{
	mVars[varnum][curr_pt] = value;
}


AnimatePoint AnimateCompile::GetStartingPosition() const
{
	return curr_sheet->GetPosition(GetCurrentPoint());
}


AnimatePoint AnimateCompile::GetEndingPosition(const ContToken *token) const
{
	CC_show::const_CC_sheet_iterator_t sheet = curr_sheet + 1;
	
	while (1)
	{
		if (sheet == mShow.GetSheetEnd())
		{
			RegisterError(ANIMERR_UNDEFINED, token);
			return GetPointPosition();
		}
		if (sheet->IsInAnimation())
		{
			return sheet->GetPosition(GetCurrentPoint());
		}
		++sheet;
	}
}


AnimatePoint AnimateCompile::GetReferencePointPosition(unsigned refnum) const
{
	return curr_sheet->GetPosition(GetCurrentPoint(), refnum+1);
}


