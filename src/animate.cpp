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

#include "animate.h"
#include "animatecommand.h"
#include "cont.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "confgr.h"

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <sstream>

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

float NormalizeAngle(float ang)
{
	while (ang >= 360.0) ang -= 360.0;
	while (ang < 0.0) ang += 360.0;
	return ang;
}

AnimateDir AnimGetDirFromAngle(float ang)
{
	ang = NormalizeAngle(ang);
	// rotate angle by 22.5:
	ang += 22.5;
	size_t quadrant = ang/45.0;
	switch (quadrant)
	{
		case 0: return ANIMDIR_N;
		case 1: return ANIMDIR_NW;
		case 2: return ANIMDIR_W;
		case 3: return ANIMDIR_SW;
		case 4: return ANIMDIR_S;
		case 5: return ANIMDIR_SE;
		case 6: return ANIMDIR_E;
		case 7: return ANIMDIR_NE;
		case 8: return ANIMDIR_N;
	}
	return ANIMDIR_N;
}


// An animation sheet is a collection of commands for each of the points.
class AnimateSheet
{
public:
	AnimateSheet(const std::vector<AnimatePoint>& thePoints, const wxString s, unsigned beats) : pts(thePoints), commands(thePoints.size()), name(s), numbeats(beats) {}
	~AnimateSheet() {}
	wxString GetName() const { return name; }
	unsigned GetNumBeats() const { return numbeats; }
	
	std::vector<AnimatePoint> pts; // should probably be const
	std::vector<std::vector<boost::shared_ptr<AnimateCommand> > > commands;
private:
	wxString name;
	unsigned numbeats;
};


Animation::Animation(CC_show *show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList)
: numpts(show->GetNumPoints()), pts(numpts), curr_cmds(numpts),
curr_sheetnum(0),
mCollisionAction(NULL)
{
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
		AnimateSheet new_sheet(thePoints, curr_sheet->GetName(), curr_sheet->GetBeats());

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
				for (unsigned j = 0; j < numpts; j++)
				{
					if (curr_sheet->GetPoint(j).GetContinuityIndex() == currcont->GetNum())
					{
						comp.Compile(curr_sheet, j, contnum, ParsedContinuity);
						new_sheet.commands[j] = comp.GetCommands();
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
		for (unsigned j = 0; j < numpts; j++)
		{
			if (new_sheet.commands[j].empty())
			{
				comp.Compile(curr_sheet, j, contnum, ParsedContinuity);
				new_sheet.commands[j] = comp.GetCommands();
			}
		}
		if (!comp.Okay() && notifyErrorList)
		{
			tempbuf.Printf(wxT("Errors for \"%.32s\""), curr_sheet->GetName().c_str());
			if (notifyErrorList(comp.GetErrorMarkers(), sheetnum, tempbuf))
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
		if (curr_beat >= sheets.at(curr_sheetnum).GetNumBeats())
		{
			if (sheets.at(curr_sheetnum).GetNumBeats() == 0)
			{
				curr_beat = 0;
			}
			else
			{
				curr_beat = sheets.at(curr_sheetnum).GetNumBeats()-1;
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
		curr_beat = sheets.at(curr_sheetnum).GetNumBeats();
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
	if (curr_beat >= sheets.at(curr_sheetnum).GetNumBeats())
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
	for (unsigned i = 0; i < numpts; i++)
	{
		for (unsigned j = i+1; j < numpts; j++)
		{
			if (pts[i].Collides(pts[j]))
			{
				mCollisions.insert(i);
				mCollisions.insert(j);
			}
		}
	}
	if (!mCollisions.empty() && mCollisionAction)
	{
		mCollisionAction();
	}
}


bool Animation::IsCollision(unsigned which) const
{
	return mCollisions.count(which);
}

AnimateDir Animation::Direction(unsigned which) const
{
	return (*curr_cmds.at(which))->Direction();
}

float Animation::RealDirection(unsigned which) const
{
	return (*curr_cmds.at(which))->RealDirection();
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
	return sheets.at(curr_sheetnum).GetNumBeats();
}

int Animation::GetCurrentBeat() const
{
	return curr_beat;
}

wxString Animation::GetCurrentSheetName() const
{
	return sheets.at(curr_sheetnum).GetName();
}

void Animation::DrawPath(wxDC& dc, int whichPoint, const CC_coord& offset) const
{
	dc.SetBrush(*wxTRANSPARENT_BRUSH);
	dc.SetPen(*CalChartPens[COLOR_PATHS]);
	// get the point we want to draw:
	const AnimateSheet& sheet = sheets.at(curr_sheetnum);
	AnimatePoint point = pts.at(whichPoint);
	for (std::vector<boost::shared_ptr<AnimateCommand> >::const_iterator commands = sheet.commands.at(whichPoint).begin(); commands != sheet.commands.at(whichPoint).end(); ++commands)
	{
		(*commands)->DrawCommand(dc, point, offset);
		(*commands)->ApplyForward(point);
	}
	dc.SetBrush(*CalChartBrushes[COLOR_PATHS]);
	float circ_r = Float2Coord(GetConfiguration_DotRatio());
	dc.DrawEllipse(point.x - circ_r/2 + offset.x, point.y - circ_r/2 + offset.y, circ_r, circ_r);
	return ;
}


std::string
Animation::GetCurrentInfo() const
{
	std::ostringstream output;
	output<< GetCurrentSheetName().ToStdString()<<" ("<<GetCurrentSheet()<<" of "<<GetNumberSheets()<<")\n";
	output<<"beat "<<GetCurrentBeat() <<" of "<<GetNumberBeats()<<"\n";
	for (size_t i = 0; i < numpts; ++i)
	{
		output<<"pt "<<i<<": ("<<Position(i).x<<", "<<Position(i).y<<"), dir="<<Direction(i)<<", realdir="<<RealDirection(i)<<(IsCollision(i) ? ", collision!" : "")<< "\n";
	}
	return output.str();
}
