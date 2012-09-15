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


class AnimateVariable
{
private:
	float v;
	bool valid;
public:
	AnimateVariable(): v(0.0), valid(false) {}
	inline bool IsValid() const { return valid; }
	inline float GetValue() const { return v; }
	inline void SetValue(float newv) { v = newv; valid = true; }
	inline void ClearValue() { v = 0.0; valid = false; }
};


AnimateCompile::AnimateCompile(CC_show* show)
: mShow(show), okay(true)
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


void AnimateCompile::FreeErrorMarkers()
{
	for (unsigned i = 0; i < NUM_ANIMERR; i++)
	{
		error_markers[i].Reset();
	}
}


float AnimateCompile::GetVarValue(int varnum, const ContToken *token)
{
	if (vars[varnum][curr_pt].IsValid())
	{
		return vars[varnum][curr_pt].GetValue();
	}
	RegisterError(ANIMERR_UNDEFINED, token);
	return 0.0;
}


void AnimateCompile::SetVarValue(int varnum, float value)
{
	vars[varnum][curr_pt].SetValue(value);
}

