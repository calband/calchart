/* animate.cpp
 * Classes for animating shows
 *
 * Modification history:
 * 12-29-95   Garrick Meeker              Created
 *
 */

/*
   Copyright (C) 1995-2008  Garrick Brian Meeker

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

#ifdef __GNUG__
#pragma implementation
#endif

#include "anim_ui.h"
#include "cont.h"
#include <math.h>
#include <string>

extern int parsecontinuity();
extern const char *yyinputbuffer;
extern ContProcedure *ParsedContinuity;

const wxChar *animate_err_msgs[] =
{
	wxT("Ran out of time"),
	wxT("Not enough to do"),
	wxT("Didn't make it to position"),
	wxT("Invalid countermarch"),
	wxT("Invalid fountain"),
	wxT("Division by zero"),
	wxT("Undefined value"),
	wxT("Syntax error"),
	wxT("Non-integer value"),
	wxT("Negative value"),
};

AnimateDir AnimGetDirFromVector(CC_coord& vector)
{
	if (vector.y < vector.x/2)
	{
		if (vector.y < -vector.x/2)
		{
			if (vector.y/2 < vector.x)
			{
				if (-vector.y/2 < vector.x)
				{
					return ANIMDIR_W;
				}
				else
				{
					return ANIMDIR_SW;
				}
			}
			else
			{
				return ANIMDIR_NW;
			}
		}
		else
		{
			return ANIMDIR_S;
		}
	}
	else
	{
		if (vector.y < -vector.x/2)
		{
			return ANIMDIR_N;
		}
		else
		{
			if (vector.y/2 < vector.x)
			{
				return ANIMDIR_SE;
			}
			else
			{
				if (-vector.y/2 < vector.x)
				{
					return ANIMDIR_NE;
				}
				else
				{
					return ANIMDIR_E;
				}
			}
		}
	}
}


AnimateDir AnimGetDirFromAngle(float ang)
{
	while (ang >= 360.0) ang -= 360.0;
	while (ang < 0.0) ang += 360.0;
	if (ang > 22.5)
	{
		if (ang > 67.5)
		{
			if (ang > 112.5)
			{
				if (ang > 157.5)
				{
					if (ang > 202.5)
					{
						if (ang > 247.5)
						{
							if (ang > 292.5)
							{
								if (ang > 337.5)
								{
									return ANIMDIR_N;
								}
								else
								{
									return ANIMDIR_NE;
								}
							}
							else
							{
								return ANIMDIR_E;
							}
						}
						else
						{
							return ANIMDIR_SE;
						}
					}
					else
					{
						return ANIMDIR_S;
					}
				}
				else
				{
					return ANIMDIR_SW;
				}
			}
			else
			{
				return ANIMDIR_W;
			}
		}
		else
		{
			return ANIMDIR_NW;
		}
	}
	else
	{
		return ANIMDIR_N;
	}
}


AnimateCommand::AnimateCommand(unsigned beats)
: next(NULL), prev(NULL), numbeats(beats) {}

bool AnimateCommand::Begin(AnimatePoint& pt)
{
	beat = 0;
	if (numbeats == 0)
	{
		ApplyForward(pt);
		return false;
	}
	else return true;
}


bool AnimateCommand::End(AnimatePoint& pt)
{
	beat = numbeats;
	if (numbeats == 0)
	{
		ApplyBackward(pt);
		return false;
	}
	else return true;
}


bool AnimateCommand::NextBeat(AnimatePoint&)
{
	beat++;
	if (beat >= numbeats) return false;
	else return true;
}


bool AnimateCommand::PrevBeat(AnimatePoint&)
{
	if (beat == 0) return false;
	else
	{
		beat--;
		return true;
	}
}


void AnimateCommand::ApplyForward(AnimatePoint&)
{
	beat = numbeats;
}


void AnimateCommand::ApplyBackward(AnimatePoint&)
{
	beat = 0;
}


float AnimateCommand::MotionDirection()
{
	return RealDirection();
}


void AnimateCommand::ClipBeats(unsigned beats)
{
	numbeats = beats;
}


AnimateCommandMT::AnimateCommandMT(unsigned beats, float direction)
: AnimateCommand(beats), dir(AnimGetDirFromAngle(direction)), realdir(direction)
{
}


AnimateDir AnimateCommandMT::Direction() { return dir; }

float AnimateCommandMT::RealDirection() { return realdir; }

AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement)
: AnimateCommandMT(beats, movement.Direction()), vector(movement)
{
}


AnimateCommandMove::AnimateCommandMove(unsigned beats, CC_coord movement, float direction)
: AnimateCommandMT(beats, direction), vector(movement)
{
}


bool AnimateCommandMove::NextBeat(AnimatePoint& pt)
{
	bool b = AnimateCommand::NextBeat(pt);
	pt.pos.x +=
		((long)beat * vector.x / (short)numbeats) -
		((long)(beat-1) * vector.x / (short)numbeats);
	pt.pos.y +=
		((long)beat * vector.y / (short)numbeats) -
		((long)(beat-1) * vector.y / (short)numbeats);
	return b;
}


bool AnimateCommandMove::PrevBeat(AnimatePoint& pt)
{
	if (AnimateCommand::PrevBeat(pt))
	{
		pt.pos.x +=
			((long)beat * vector.x / (short)numbeats) -
			((long)(beat+1) * vector.x / (short)numbeats);
		pt.pos.y +=
			((long)beat * vector.y / (short)numbeats) -
			((long)(beat+1) * vector.y / (short)numbeats);
		return true;
	}
	else
	{
		return false;
	}
}


void AnimateCommandMove::ApplyForward(AnimatePoint& pt)
{
	AnimateCommand::ApplyForward(pt);
	pt.pos += vector;
}


void AnimateCommandMove::ApplyBackward(AnimatePoint& pt)
{
	AnimateCommand::ApplyBackward(pt);
	pt.pos -= vector;
}


float AnimateCommandMove::MotionDirection()
{
	return vector.Direction();
}


void AnimateCommandMove::ClipBeats(unsigned beats)
{
	AnimateCommand::ClipBeats(beats);
}


AnimateCommandRotate::AnimateCommandRotate(unsigned beats, CC_coord cntr,
float rad, float ang1, float ang2,
bool backwards)
: AnimateCommand(beats), origin(cntr), r(rad), ang_start(ang1), ang_end(ang2)
{
	if (backwards) face = -90;
	else face = 90;
}


bool AnimateCommandRotate::NextBeat(AnimatePoint& pt)
{
	bool b = AnimateCommand::NextBeat(pt);
	float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
		* PI / 180.0;
	pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(curr_ang)*r);
	pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(curr_ang)*r);
	return b;
}


bool AnimateCommandRotate::PrevBeat(AnimatePoint& pt)
{
	if (AnimateCommand::PrevBeat(pt))
	{
		float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
			* PI / 180.0;
		pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(curr_ang)*r);
		pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(curr_ang)*r);
		return true;
	}
	else
	{
		return false;
	}
}


void AnimateCommandRotate::ApplyForward(AnimatePoint& pt)
{
	AnimateCommand::ApplyForward(pt);
	pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(ang_end*PI/180.0)*r);
	pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(ang_end*PI/180.0)*r);
}


void AnimateCommandRotate::ApplyBackward(AnimatePoint& pt)
{
	AnimateCommand::ApplyBackward(pt);
	pt.pos.x = (Coord)CLIPFLOAT(origin.x + cos(ang_start*PI/180.0)*r);
	pt.pos.y = (Coord)CLIPFLOAT(origin.y - sin(ang_start*PI/180.0)*r);
}


AnimateDir AnimateCommandRotate::Direction()
{
	return AnimGetDirFromAngle(RealDirection());
}


float AnimateCommandRotate::RealDirection()
{
	float curr_ang = (ang_end - ang_start) * beat / numbeats + ang_start;
	if (ang_end > ang_start)
	{
		return curr_ang + face;
	}
	else
	{
		return curr_ang - face;
	}
}


void AnimateCommandRotate::ClipBeats(unsigned beats)
{
	AnimateCommand::ClipBeats(beats);
}


AnimateSheet::AnimateSheet(unsigned numpoints)
: next(NULL), prev(NULL), pts(numpoints), numpts(numpoints)
{
	unsigned i;

	commands = new AnimateCommand*[numpts];
	end_cmds = new AnimateCommand*[numpts];
	for (i = 0; i < numpts; i++)
	{
		commands[i] = NULL;
		end_cmds[i] = NULL;
	}
}


AnimateSheet::~AnimateSheet()
{
	AnimateCommand *cmd, *tmp;
	unsigned i;

	if (commands)
	{
		for (i = 0; i < numpts; i++)
		{
			cmd = commands[i];
			while (cmd)
			{
				tmp = cmd->next;
				delete cmd;
				cmd = tmp;
			}
		}
		delete commands;
	}
	if (end_cmds) delete end_cmds;
}


void AnimateSheet::SetName(const wxChar *s)
{
	name = s;
}


Animation::Animation(CC_show *show, wxFrame *frame, CC_WinList *winlist)
: numpts(show->GetNumPoints()), pts(numpts), curr_cmds(numpts), collisions(numpts),
curr_sheet(NULL), numsheets(0), sheets(NULL)
{
	unsigned i, j;
	ContProcedure* curr_proc;
	AnimateCompile comp;
	CC_continuity *currcont;
	wxString tempbuf;

// Now compile
	comp.show = show;
	comp.show->winlist->FlushContinuity();		  // get all changes in text windows

	unsigned sheetnum = 0;
	for (comp.curr_sheet = show->GetSheet(); comp.curr_sheet;
		comp.curr_sheet = comp.curr_sheet->next, sheetnum++)
	{
		if (! comp.curr_sheet->IsInAnimation()) continue;
		if (curr_sheet)
		{
			curr_sheet->next = new AnimateSheet(numpts);
			curr_sheet->next->prev = curr_sheet;
			curr_sheet = curr_sheet->next;
		}
		else
		{
			sheets = curr_sheet = new AnimateSheet(numpts);
		}
		numsheets++;
		curr_sheet->SetName(comp.curr_sheet->GetName());
		curr_sheet->numbeats = comp.curr_sheet->GetBeats();
		for (i = 0; i < numpts; i++)
		{
			curr_sheet->pts[i].pos = comp.curr_sheet->GetPosition(i);
		}

// Now parse continuity
		comp.SetStatus(true);
		comp.FreeErrorMarkers();
		int contnum = 0;
		for (currcont = comp.curr_sheet->animcont; currcont != NULL;
			currcont  = currcont->next, contnum++)
		{
			if (currcont->text.mb_str() != NULL)
			{
				std::string tmpBuffer(currcont->text.mb_str());
				yyinputbuffer = tmpBuffer.c_str();
				tempbuf.Printf(wxT("Compiling \"%.32s\" %.32s..."),
					comp.curr_sheet->GetName().c_str(), currcont->name.c_str());
				frame->SetStatusText(tempbuf);
				int parseerr = parsecontinuity();
				ContToken dummy;				  // get position of parse error
				for (j = 0; j < numpts; j++)
				{
					if (comp.curr_sheet->GetPoint(j).cont == currcont->num)
					{
						comp.Compile(j, contnum, ParsedContinuity);
						curr_sheet->commands[j] = comp.cmds;
						curr_sheet->end_cmds[j] = comp.curr_cmd;
						if (parseerr != 0)
						{
							comp.RegisterError(ANIMERR_SYNTAX, &dummy);
						}
					}
				}
				while (ParsedContinuity)
				{
					curr_proc = ParsedContinuity->next;
					delete ParsedContinuity;
					ParsedContinuity = curr_proc;
				}
			}
		}
// Handle points that don't have continuity (shouldn't happen)
		tempbuf.Printf(wxT("Compiling \"%.32s\"..."), comp.curr_sheet->GetName().c_str());
		frame->SetStatusText(tempbuf);
		for (j = 0; j < numpts; j++)
		{
			if (curr_sheet->commands[j] == NULL)
			{
				comp.Compile(j, 0, NULL);
				curr_sheet->commands[j] = comp.cmds;
				curr_sheet->end_cmds[j] = comp.curr_cmd;
			}
		}
		if (!comp.Okay())
		{
			tempbuf.Printf(wxT("Errors for \"%.32s\""), comp.curr_sheet->GetName().c_str());
			(void)new AnimErrorList(&comp, winlist, sheetnum,
				frame, tempbuf);
			if (wxMessageBox(wxT("Ignore errors?"), wxT("Animate"), wxYES_NO) != wxYES)
			{
				break;
			}
		}
	}
}


Animation::~Animation()
{
	AnimateSheet *sht, *tmp;

	sht = sheets;
	while (sht)
	{
		tmp = sht->next;
		delete sht;
		sht = tmp;
	}
}


bool Animation::PrevSheet()
{
	if (curr_beat == 0)
	{
		if (curr_sheet->prev)
		{
			curr_sheet = curr_sheet->prev;
			curr_sheetnum--;
		}
	}
	RefreshSheet();
	CheckCollisions();
	return true;
}


bool Animation::NextSheet()
{
	if (curr_sheet->next)
	{
		curr_sheet = curr_sheet->next;
		curr_sheetnum++;
		RefreshSheet();
		CheckCollisions();
	}
	else
	{
		if (curr_beat >= curr_sheet->numbeats)
		{
			if (curr_sheet->numbeats == 0)
			{
				curr_beat = 0;
			}
			else
			{
				curr_beat = curr_sheet->numbeats-1;
			}
		}
		return false;
	}
	return true;
}


bool Animation::PrevBeat()
{
	unsigned i;

	if (curr_beat == 0)
	{
		if (curr_sheet->prev == NULL) return false;
		curr_sheet = curr_sheet->prev;
		curr_sheetnum--;
		for (i = 0; i < numpts; i++)
		{
			curr_cmds[i] = curr_sheet->end_cmds[i];
			EndCmd(i);
		}
		curr_beat = curr_sheet->numbeats;
	}
	for (i = 0; i < numpts; i++)
	{
		if (!curr_cmds[i]->PrevBeat(pts[i]))
		{
// Advance to prev command, skipping zero beat commands
			if (curr_cmds[i]->prev)
			{
				curr_cmds[i] = curr_cmds[i]->prev;
				EndCmd(i);
// Set to next-to-last beat of this command
// Should always return true
				curr_cmds[i]->PrevBeat(pts[i]);
			}
		}
	}
	if (curr_beat > 0)
		curr_beat--;
	CheckCollisions();
	return true;
}


bool Animation::NextBeat()
{
	unsigned i;

	curr_beat++;
	if (curr_beat >= curr_sheet->numbeats)
	{
		return NextSheet();
	}
	for (i = 0; i < numpts; i++)
	{
		if (curr_cmds[i])
		{
			if (!curr_cmds[i]->NextBeat(pts[i]))
			{
// Advance to next command, skipping zero beat commands
				if (curr_cmds[i]->next)
				{
					curr_cmds[i] = curr_cmds[i]->next;
					BeginCmd(i);
				}
			}
		}
	}
	CheckCollisions();
	return true;
}


void Animation::GotoBeat(unsigned i)
{
	while (curr_beat > i)
	{
		PrevBeat();
	}
	while (curr_beat < i)
	{
		NextBeat();
	}
}


void Animation::GotoSheet(unsigned i)
{
	curr_sheetnum = i;
	curr_sheet = sheets;
	while (i > 0)
	{
		curr_sheet = curr_sheet->next;
		i--;
	}
	RefreshSheet();
	CheckCollisions();
}


void Animation::BeginCmd(unsigned i)
{
	if (curr_cmds[i] != NULL)
	{
		while (!curr_cmds[i]->Begin(pts[i]))
		{
			if (curr_cmds[i]->next == NULL) return;
			curr_cmds[i] = curr_cmds[i]->next;
		}
	}
}


void Animation::EndCmd(unsigned i)
{
	if (curr_cmds[i] != NULL)
	{
		while (!curr_cmds[i]->End(pts[i]))
		{
			if (curr_cmds[i]->prev == NULL) return;
			curr_cmds[i] = curr_cmds[i]->prev;
		}
	}
}


void Animation::RefreshSheet()
{
	unsigned i;

	for (i = 0; i < numpts; i++)
	{
		pts[i].pos = curr_sheet->pts[i].pos;
		curr_cmds[i] = curr_sheet->commands[i];
		BeginCmd(i);
	}
	curr_beat = 0;
}


void Animation::CheckCollisions()
{
	unsigned i, j;
	bool beep = false;

	for (i = 0; i < numpts; i++)
	{
		collisions[i] = false;
	}
	if (check_collis != COLLISION_NONE)
	{
		for (i = 0; i < numpts; i++)
		{
			for (j = i+1; j < numpts; j++)
			{
				if (pts[i].pos.Collides(pts[j].pos))
				{
					collisions[i] = true;
					collisions[j] = true;
					beep = true;
				}
			}
		}
		if (beep && (check_collis == COLLISION_BEEP))
		{
			wxBell();
		}
	}
}


AnimateCompile::AnimateCompile()
: okay(true)
{
	unsigned i;

	for (i = 0; i < NUMCONTVARS; i++)
	{
		vars[i] = NULL;
	}
}


AnimateCompile::~AnimateCompile()
{
	unsigned i;

	FreeErrorMarkers();
	for (i = 0; i < NUMCONTVARS; i++)
	{
		if (vars[i] != NULL)
		{
			delete [] vars[i];
			vars[i] = NULL;
		}
	}
}


void AnimateCompile::Compile(unsigned pt_num, unsigned cont_num,
ContProcedure* proc)
{
	CC_coord c;

	contnum = cont_num;
	pt.pos = curr_sheet->GetPosition(pt_num);
	cmds = curr_cmd = NULL;
	curr_pt = pt_num;
	beats_rem = curr_sheet->GetBeats();

	if (proc == NULL)
	{
// no continuity was specified
		CC_sheet *s;
		for (s = curr_sheet->next; s != NULL; s = s->next)
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
		if (s == NULL)
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
	if (curr_sheet->next)
	{
		if (pt.pos !=
			curr_sheet->next->GetPosition(curr_pt))
		{
			c = curr_sheet->next->GetPosition(curr_pt) - pt.pos;
			RegisterError(ANIMERR_WRONGPLACE, NULL);
			Append(new AnimateCommandMove(beats_rem, c), NULL);
		}
	}
	if (beats_rem)
	{
		RegisterError(ANIMERR_EXTRATIME, NULL);
		Append(new AnimateCommandMT(beats_rem, ANIMDIR_E), NULL);
	}
}


bool AnimateCompile::Append(AnimateCommand *cmd, const ContToken *token)
{
	bool clipped;

	if (beats_rem < cmd->numbeats)
	{
		RegisterError(ANIMERR_OUTOFTIME, token);
		if (beats_rem == 0)
		{
			delete cmd;
			return false;
		}
		cmd->ClipBeats(beats_rem);
		clipped = true;
	}
	else
	{
		clipped = false;
	}
	if (curr_cmd)
	{
		curr_cmd->next = cmd;
		cmd->prev = curr_cmd;
		curr_cmd = cmd;
	}
	else
	{
		cmds = curr_cmd = cmd;
	}
	beats_rem -= cmd->numbeats;

	cmd->ApplyForward(pt);						  // Move current point to new position
	SetVarValue(CONTVAR_DOF, cmd->MotionDirection());
	SetVarValue(CONTVAR_DOH, cmd->RealDirection());
	return true;
}


void AnimateCompile::RegisterError(AnimateError err, const ContToken *token)
{
	MakeErrorMarker(err, token);
	error_markers[err].pntgroup[curr_pt] = true;
	SetStatus(false);
}


void AnimateCompile::FreeErrorMarkers()
{
	unsigned i;

	for (i = 0; i < NUM_ANIMERR; i++)
	{
		error_markers[i].Free();
	}
}


float AnimateCompile::GetVarValue(int varnum, const ContToken *token)
{
	if (vars[varnum])
	{
		if (vars[varnum][curr_pt].IsValid())
		{
			return vars[varnum][curr_pt].GetValue();
		}
	}
	RegisterError(ANIMERR_UNDEFINED, token);
	return 0.0;
}


void AnimateCompile::SetVarValue(int varnum, float value)
{
	if (!vars[varnum])
	{
		vars[varnum] = new AnimateVariable[show->GetNumPoints()];
	}
	vars[varnum][curr_pt].SetValue(value);
}


void AnimateCompile::MakeErrorMarker(AnimateError err,
const ContToken *token)
{
	unsigned i;

	if (!error_markers[err].pntgroup)
	{
		error_markers[err].pntgroup = new bool[show->GetNumPoints()];
		for (i = 0; i < show->GetNumPoints(); i++)
		{
			error_markers[err].pntgroup[i] = false;
		}
		error_markers[err].contnum = contnum;
		if (token != NULL)
		{
			error_markers[err].line = token->line;
			error_markers[err].col = token->col;
		}
	}
}
