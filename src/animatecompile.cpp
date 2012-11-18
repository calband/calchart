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


float AnimationVariables::GetVarValue(int varnum, unsigned whichPoint) const
{
	std::map<unsigned,AnimateVariable>::const_iterator i = mVars[varnum].find(whichPoint);
	if (i == mVars[varnum].end() || !i->second.IsValid())
	{
		throw AnimationVariableException();
	}
	return i->second.GetValue();
}


void AnimationVariables::SetVarValue(int varnum, unsigned whichPoint, float value)
{
	mVars[varnum][whichPoint].SetValue(value);
}


AnimateCompile::AnimateCompile(CC_show* show, AnimationVariables& variablesStates) :
mShow(show),
error_markers(NUM_ANIMERR),
vars(variablesStates),
okay(true)
{
}


AnimateCompile::~AnimateCompile()
{
}


void AnimateCompile::Compile(CC_show::const_CC_sheet_iterator_t c_sheet, unsigned pt_num, unsigned cont_num, ContProcedure* proc)
{
	CC_coord c;

	contnum = cont_num;
	curr_sheet = c_sheet;
	pt = curr_sheet->GetPosition(pt_num);
	cmds.clear();
	curr_pt = pt_num;
	beats_rem = curr_sheet->GetBeats();

	if (proc == NULL)
	{
// no continuity was specified
		CC_show::const_CC_sheet_iterator_t s;
		for (s = curr_sheet + 1; s != mShow->GetSheetEnd(); ++s)
		{
			if (s->IsInAnimation())
			{
//use EVEN REM NP
				ContProcEven defcont(new ContValueFloat(beats_rem),
					new ContNextPoint());
				defcont.Compile(this);
				break;
			}
		}
		if (s == mShow->GetSheetEnd())
		{
//use MTRM E
			ContProcMTRM defcont(new ContValueDefined(CC_E));
			defcont.Compile(this);
		}
	}

	for (; proc; proc = proc->next)
	{
		proc->Compile(this);
	}
	if ((curr_sheet + 1) != mShow->GetSheetEnd())
	{
		CC_show::const_CC_sheet_iterator_t curr_sheet_next = curr_sheet+1;
		if (pt !=
			curr_sheet_next->GetPosition(curr_pt))
		{
			c = curr_sheet_next->GetPosition(curr_pt) - pt;
			RegisterError(ANIMERR_WRONGPLACE, NULL);
			Append(boost::shared_ptr<AnimateCommand>(new AnimateCommandMove(beats_rem, c)), NULL);
		}
	}
	if (beats_rem)
	{
		RegisterError(ANIMERR_EXTRATIME, NULL);
		Append(boost::shared_ptr<AnimateCommand>(new AnimateCommandMT(beats_rem, ANIMDIR_E)), NULL);
	}
}


bool AnimateCompile::Append(boost::shared_ptr<AnimateCommand> cmd, const ContToken *token)
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


void AnimateCompile::RegisterError(AnimateError err, const ContToken *token)
{
	error_markers[err].contnum = contnum;
	if (token != NULL)
	{
		error_markers[err].line = token->line;
		error_markers[err].col = token->col;
	}
	error_markers[err].pntgroup.insert(curr_pt);
	SetStatus(false);
}


float AnimateCompile::GetVarValue(int varnum, const ContToken *token)
{
	try {
		return vars.GetVarValue(varnum, curr_pt);
	}
	catch (AnimationVariables::AnimationVariableException&)
	{
	}
	RegisterError(ANIMERR_UNDEFINED, token);
	return 0.0;
}


void AnimateCompile::SetVarValue(int varnum, float value)
{
	vars.SetVarValue(varnum, curr_pt, value);
}


AnimatePoint AnimateCompile::GetStartingPosition() const
{
	return curr_sheet->GetPosition(GetCurrentPoint());
}


AnimatePoint AnimateCompile::GetEndingPosition(const ContToken *token)
{
	CC_show::const_CC_sheet_iterator_t sheet = curr_sheet + 1;
	
	while (1)
	{
		if (sheet == mShow->GetSheetEnd())
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


