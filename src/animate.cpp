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

#include "anim_ui.h"
#include "cont.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "platconf.h"
#include "confgr.h"
#include <math.h>
#include <string>

extern int parsecontinuity();
extern const char *yyinputbuffer;
extern ContProcedure *ParsedContinuity;

const wxString animate_err_msgs[] =
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
: numbeats(beats) {}

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
	pt.x +=
		((long)beat * vector.x / (short)numbeats) -
		((long)(beat-1) * vector.x / (short)numbeats);
	pt.y +=
		((long)beat * vector.y / (short)numbeats) -
		((long)(beat-1) * vector.y / (short)numbeats);
	return b;
}


bool AnimateCommandMove::PrevBeat(AnimatePoint& pt)
{
	if (AnimateCommand::PrevBeat(pt))
	{
		pt.x +=
			((long)beat * vector.x / (short)numbeats) -
			((long)(beat+1) * vector.x / (short)numbeats);
		pt.y +=
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
	pt += vector;
}


void AnimateCommandMove::ApplyBackward(AnimatePoint& pt)
{
	AnimateCommand::ApplyBackward(pt);
	pt -= vector;
}


float AnimateCommandMove::MotionDirection()
{
	return vector.Direction();
}


void AnimateCommandMove::ClipBeats(unsigned beats)
{
	AnimateCommand::ClipBeats(beats);
}


void AnimateCommandMove::DrawCommand(wxDC& dc, const AnimatePoint& pt, const CC_coord& offset) const
{
	dc.DrawLine(pt.x + offset.x, pt.y + offset.y, pt.x + vector.x + offset.x, pt.y + vector.y + offset.y);
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
	pt.x = (Coord)CLIPFLOAT(origin.x + cos(curr_ang)*r);
	pt.y = (Coord)CLIPFLOAT(origin.y - sin(curr_ang)*r);
	return b;
}


bool AnimateCommandRotate::PrevBeat(AnimatePoint& pt)
{
	if (AnimateCommand::PrevBeat(pt))
	{
		float curr_ang = ((ang_end - ang_start) * beat / numbeats + ang_start)
			* PI / 180.0;
		pt.x = (Coord)CLIPFLOAT(origin.x + cos(curr_ang)*r);
		pt.y = (Coord)CLIPFLOAT(origin.y - sin(curr_ang)*r);
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
	pt.x = (Coord)CLIPFLOAT(origin.x + cos(ang_end*PI/180.0)*r);
	pt.y = (Coord)CLIPFLOAT(origin.y - sin(ang_end*PI/180.0)*r);
}


void AnimateCommandRotate::ApplyBackward(AnimatePoint& pt)
{
	AnimateCommand::ApplyBackward(pt);
	pt.x = (Coord)CLIPFLOAT(origin.x + cos(ang_start*PI/180.0)*r);
	pt.y = (Coord)CLIPFLOAT(origin.y - sin(ang_start*PI/180.0)*r);
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


void AnimateCommandRotate::DrawCommand(wxDC& dc, const AnimatePoint& pt, const CC_coord& offset) const
{
	float start = (ang_start < ang_end) ? ang_start : ang_end;
	float end = (ang_start < ang_end) ? ang_end : ang_start;
	wxCoord x_start = (Coord)CLIPFLOAT(origin.x + cos(start*PI/180.0)*r) + offset.x;
	wxCoord y_start = (Coord)CLIPFLOAT(origin.y - sin(start*PI/180.0)*r) + offset.y;
	wxCoord x_end = (Coord)CLIPFLOAT(origin.x + cos(end*PI/180.0)*r) + offset.x;
	wxCoord y_end = (Coord)CLIPFLOAT(origin.y - sin(end*PI/180.0)*r) + offset.y;

	dc.DrawArc(x_start, y_start, x_end, y_end, origin.x + offset.x, origin.y + offset.y);
}


AnimateSheet::AnimateSheet(const std::vector<AnimatePoint>& thePoints)
: pts(thePoints), commands(thePoints.size())
{}


AnimateSheet::~AnimateSheet()
{}


void AnimateSheet::SetName(const wxString& s)
{
	name = s;
}


Animation::Animation(CC_show *show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList)
: numpts(show->GetNumPoints()), pts(numpts), curr_cmds(numpts),
curr_sheetnum(0)
{
	unsigned j;
	AnimateCompile comp(show);
	wxString tempbuf;

	unsigned sheetnum = 0;
	for (CC_show::const_CC_sheet_iterator_t curr_sheet = show->GetSheetBegin(); curr_sheet != show->GetSheetEnd();
		++curr_sheet, sheetnum++)
	{
		if (! curr_sheet->IsInAnimation()) continue;
		std::vector<AnimatePoint> thePoints(numpts);
		for (unsigned i = 0; i < numpts; i++)
		{
			thePoints.at(i) = curr_sheet->GetPosition(i);
		}
		AnimateSheet new_sheet(thePoints);
		new_sheet.SetName(curr_sheet->GetName());
		new_sheet.numbeats = curr_sheet->GetBeats();

// Now parse continuity
		comp.SetStatus(true);
		comp.FreeErrorMarkers();
		int contnum = 0;
		for (CC_sheet::ContContainer::const_iterator currcont = curr_sheet->animcont.begin(); currcont != curr_sheet->animcont.end();
			++currcont, contnum++)
		{
			if (!currcont->GetText().IsEmpty())
			{
				std::string tmpBuffer(currcont->GetText().mb_str());
				yyinputbuffer = tmpBuffer.c_str();
				tempbuf.Printf(wxT("Compiling \"%.32s\" %.32s..."),
					curr_sheet->GetName().c_str(), currcont->GetName().c_str());
				if (notifyStatus)
				{
					notifyStatus(tempbuf);
				}
				int parseerr = parsecontinuity();
				ContToken dummy;				  // get position of parse error
				for (j = 0; j < numpts; j++)
				{
					if (curr_sheet->GetPoint(j).cont == currcont->GetNum())
					{
						comp.Compile(curr_sheet, j, contnum, ParsedContinuity);
						new_sheet.commands[j] = comp.cmds;
						if (parseerr != 0)
						{
							comp.RegisterError(ANIMERR_SYNTAX, &dummy);
						}
					}
				}
				while (ParsedContinuity)
				{
					ContProcedure* curr_proc = ParsedContinuity->next;
					delete ParsedContinuity;
					ParsedContinuity = curr_proc;
				}
			}
		}
// Handle points that don't have continuity (shouldn't happen)
		tempbuf.Printf(wxT("Compiling \"%.32s\"..."), curr_sheet->GetName().c_str());
		if (notifyStatus)
		{
			notifyStatus(tempbuf);
		}
		for (j = 0; j < numpts; j++)
		{
			if (new_sheet.commands[j].empty())
			{
				comp.Compile(curr_sheet, j, contnum, ParsedContinuity);
				new_sheet.commands[j] = comp.cmds;
			}
		}
		if (!comp.Okay() && notifyErrorList)
		{
			tempbuf.Printf(wxT("Errors for \"%.32s\""), curr_sheet->GetName().c_str());
			if (notifyErrorList(comp.error_markers, sheetnum, tempbuf))
			{
				break;
			}
		}
		sheets.push_back(new_sheet);
	}
}


Animation::~Animation()
{}


bool Animation::PrevSheet()
{
	if (curr_beat == 0)
	{
		if (curr_sheetnum > 0)
		{
			curr_sheetnum--;
		}
	}
	RefreshSheet();
	CheckCollisions();
	return true;
}


bool Animation::NextSheet()
{
	if ((curr_sheetnum + 1) != sheets.size())
	{
		curr_sheetnum++;
		RefreshSheet();
		CheckCollisions();
	}
	else
	{
		if (curr_beat >= sheets.at(curr_sheetnum).numbeats)
		{
			if (sheets.at(curr_sheetnum).numbeats == 0)
			{
				curr_beat = 0;
			}
			else
			{
				curr_beat = sheets.at(curr_sheetnum).numbeats-1;
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
		if (curr_sheetnum == 0) return false;
		curr_sheetnum--;
		for (i = 0; i < numpts; i++)
		{
			curr_cmds[i] = sheets.at(curr_sheetnum).commands[i].end() - 1;
			EndCmd(i);
		}
		curr_beat = sheets.at(curr_sheetnum).numbeats;
	}
	for (i = 0; i < numpts; i++)
	{
		if (!(*curr_cmds.at(i))->PrevBeat(pts[i]))
		{
// Advance to prev command, skipping zero beat commands
			if (curr_cmds[i] != sheets.at(curr_sheetnum).commands[i].begin())
			{
				--curr_cmds[i];
				EndCmd(i);
// Set to next-to-last beat of this command
// Should always return true
				(*curr_cmds[i])->PrevBeat(pts[i]);
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
	if (curr_beat >= sheets.at(curr_sheetnum).numbeats)
	{
		return NextSheet();
	}
	for (i = 0; i < numpts; i++)
	{
		if (!(*curr_cmds.at(i))->NextBeat(pts[i]))
		{
// Advance to next command, skipping zero beat commands
			if ((curr_cmds[i] + 1) != sheets.at(curr_sheetnum).commands.at(i).end())
			{
				++curr_cmds[i];
				BeginCmd(i);
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
	RefreshSheet();
	CheckCollisions();
}


void Animation::BeginCmd(unsigned i)
{
	while (!(*curr_cmds.at(i))->Begin(pts[i]))
	{
		if ((curr_cmds[i] + 1) != sheets.at(curr_sheetnum).commands.at(i).end())
			return;
		++curr_cmds[i];
	}
}


void Animation::EndCmd(unsigned i)
{
	while (!(*curr_cmds.at(i))->End(pts[i]))
	{
		if ((curr_cmds[i]) == sheets.at(curr_sheetnum).commands.at(i).begin())
			return;
		--curr_cmds[i];
	}
}


void Animation::RefreshSheet()
{
	unsigned i;

	for (i = 0; i < numpts; i++)
	{
		pts[i] = sheets.at(curr_sheetnum).pts[i];
		curr_cmds[i] = sheets.at(curr_sheetnum).commands[i].begin();
		BeginCmd(i);
	}
	curr_beat = 0;
}


void Animation::CheckCollisions()
{
	mCollisions.clear();
	if (check_collis != COLLISION_NONE)
	{
		bool beep = false;
		for (unsigned i = 0; i < numpts; i++)
		{
			for (unsigned j = i+1; j < numpts; j++)
			{
				if (pts[i].Collides(pts[j]))
				{
					mCollisions.insert(i);
					mCollisions.insert(j);
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


bool Animation::IsCollision(unsigned which) const
{
	return mCollisions.count(which);
}

AnimateDir Animation::Direction(unsigned which) const
{
	return (*curr_cmds.at(which))->Direction();
	return ANIMDIR_E;
}

CC_coord Animation::Position(unsigned which) const
{
	return pts.at(which);
}

int Animation::GetNumberSheets() const
{
	return sheets.size();
}

int Animation::GetCurrentSheet() const
{
	return curr_sheetnum;
}

int Animation::GetNumberBeats() const
{
	return sheets.at(curr_sheetnum).numbeats;
}

int Animation::GetCurrentBeat() const
{
	return curr_beat;
}

const wxString& Animation::GetCurrentSheetName() const
{
	return sheets.at(curr_sheetnum).name;
}

void Animation::DrawPath(wxDC& dc, int whichPoint, const CC_coord& offset) const
{
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*CalChartPens[COLOR_PATHS]);
	// get the point we want to draw:
	const AnimateSheet& sheet = sheets.at(curr_sheetnum);
	AnimatePoint point = sheet.pts.at(whichPoint);
	for (std::vector<boost::shared_ptr<AnimateCommand> >::const_iterator commands = sheet.commands.at(whichPoint).begin(); commands != sheet.commands.at(whichPoint).end(); ++commands)
	{
		(*commands)->DrawCommand(dc, point, offset);
		(*commands)->ApplyForward(point);
	}
	dc.SetBrush(*CalChartBrushes[COLOR_PATHS]);
	float circ_r = FLOAT2COORD(GetConfiguration_DotRatio());
	dc.DrawEllipse(point.x - circ_r/2 + offset.x, point.y - circ_r/2 + offset.y, circ_r, circ_r);
	return ;
}

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

	if (beats_rem < cmd->numbeats)
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
	beats_rem -= cmd->numbeats;

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

