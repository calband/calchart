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
#include "math_utils.h"

#include <boost/format.hpp>

#define _USE_MATH_DEFINES
#include <cmath>
#include <string>
#include <sstream>

extern int parsecontinuity();
extern const char *yyinputbuffer;
extern ContProcedure *ParsedContinuity;

std::string animate_err_msgs(size_t which)
{
	const std::string s_animate_err_msgs[] = {
		"Ran out of time",
		"Not enough to do",
		"Didn't make it to position",
		"Invalid countermarch",
		"Invalid fountain",
		"Division by zero",
		"Undefined value",
		"Syntax error",
		"Non-integer value",
		"Negative value",
	};
	if (which < sizeof(s_animate_err_msgs)/sizeof(std::string))
	{
		return s_animate_err_msgs[which];
	}
	return "";
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


// AnimateSheet is a snapshot of CC_sheet
class AnimateSheet
{
public:
	AnimateSheet(const std::vector<AnimatePoint>& thePoints, const std::vector<std::vector<boost::shared_ptr<AnimateCommand> > >& theCommands, const std::string& s, unsigned beats) : pts(thePoints), commands(theCommands), name(s), numbeats(beats) {}
	~AnimateSheet() {}
	std::string GetName() const { return name; }
	unsigned GetNumBeats() const { return numbeats; }
	
	std::vector<AnimatePoint> pts; // should probably be const
	std::vector<std::vector<boost::shared_ptr<AnimateCommand> > > commands;
private:
	std::string name;
	unsigned numbeats;
};


Animation::Animation(const CC_show& show, NotifyStatus notifyStatus, NotifyErrorList notifyErrorList)
: numpts(show.GetNumPoints()), pts(numpts), curr_cmds(numpts),
curr_sheetnum(0),
mCollisionAction(NULL)
{
	// the variables are persistant through the entire compile process.
	AnimationVariables variablesStates;

	for (auto curr_sheet = show.GetSheetBegin(); curr_sheet != show.GetSheetEnd(); ++curr_sheet)
	{
		if (!curr_sheet->IsInAnimation()) continue;

// Now parse continuity
		AnimateCompile comp(show, variablesStates);
		std::vector<std::vector<boost::shared_ptr<AnimateCommand> > > theCommands(numpts);
		for (auto& current_symbol : k_symbols)
		{
			if (curr_sheet->ContinuityInUse(current_symbol))
			{
				auto& current_continuity = curr_sheet->GetContinuityBySymbol(current_symbol);
				std::string tmpBuffer(current_continuity.GetText());
				yyinputbuffer = tmpBuffer.c_str();
				if (notifyStatus)
				{
					notifyStatus((boost::format("Compiling \"%1%\" %2%...") % curr_sheet->GetName().substr(0,32) % GetNameForSymbol(current_symbol).substr(0,32)).str());
				}
				// parse out the error
				if (parsecontinuity() != 0)
				{
					// Supply a generic parse error
					ContToken dummy;
					comp.RegisterError(ANIMERR_SYNTAX, &dummy);
				}
				for (unsigned j = 0; j < numpts; j++)
				{
					if (curr_sheet->GetPoint(j).GetSymbol() == current_symbol)
					{
						theCommands[j] = comp.Compile(curr_sheet, j, current_symbol, ParsedContinuity);
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
		if (notifyStatus)
		{
			notifyStatus((boost::format("Compiling \"%1%\"...") % curr_sheet->GetName().substr(0,32)).str());
		}
		for (unsigned j = 0; j < numpts; j++)
		{
			if (theCommands[j].empty())
			{
				theCommands[j] = comp.Compile(curr_sheet, j, MAX_NUM_SYMBOLS, NULL);
			}
		}
		if (!comp.Okay() && notifyErrorList)
		{
			if (notifyErrorList(comp.GetErrorMarkers(), std::distance(show.GetSheetBegin(), curr_sheet), (boost::format("Errors for \"%1%\"") % curr_sheet->GetName().substr(0, 32)).str()))
			{
				break;
			}
		}
		std::vector<AnimatePoint> thePoints(numpts);
		for (unsigned i = 0; i < numpts; i++)
		{
			thePoints.at(i) = curr_sheet->GetPosition(i);
		}
		AnimateSheet new_sheet(thePoints, theCommands, curr_sheet->GetName(), curr_sheet->GetBeats());
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


Animation::animate_info_t
Animation::GetAnimateInfo(unsigned which) const
{
	return Animation::animate_info_t( mCollisions.count(which), (*curr_cmds.at(which))->Direction(), (*curr_cmds.at(which))->RealDirection(), pts.at(which) );
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

std::string Animation::GetCurrentSheetName() const
{
	return sheets.at(curr_sheetnum).GetName();
}

std::vector<boost::shared_ptr<AnimateCommand> >
Animation::GetCommands(unsigned whichPoint) const
{
	return sheets.at(curr_sheetnum).commands.at(whichPoint);
}


std::vector<CC_DrawCommand>
Animation::GenPathToDraw(unsigned point, const CC_coord& offset) const
{
	auto animation_commands = GetCommands(point);
	auto position = pts.at(point);
	std::vector<CC_DrawCommand> draw_commands;
	for (auto commands = animation_commands.begin(); commands != animation_commands.end(); ++commands)
	{
		draw_commands.push_back((*commands)->GenCC_DrawCommand(position, offset));
		(*commands)->ApplyForward(position);
	}
	return draw_commands;
}

AnimatePoint
Animation::EndPosition(unsigned point, const CC_coord& offset) const
{
	auto animation_commands = GetCommands(point);
	auto position = pts.at(point);
	for (auto commands = animation_commands.begin(); commands != animation_commands.end(); ++commands)
	{
		(*commands)->ApplyForward(position);
	}
	position += offset;
	return position;
}

std::pair<std::string, std::vector<std::string> >
Animation::GetCurrentInfo() const
{
	std::vector<std::string> each;
	for (size_t i = 0; i < numpts; ++i)
	{
		std::ostringstream each_string;
		auto info = GetAnimateInfo(i);
		each_string<<"pt "<<i<<": ("<<info.mPosition.x<<", "<<info.mPosition.y<<"), dir="<<info.mDirection<<", realdir="<<info.mRealDirection<<(info.mCollision ? ", collision!" : "");
		each.push_back(each_string.str());
	}
	std::ostringstream output;
	output<<GetCurrentSheetName()<<" ("<<GetCurrentSheet()<<" of "<<GetNumberSheets()<<")\n";
	output<<"beat "<<GetCurrentBeat() <<" of "<<GetNumberBeats()<<"\n";
	return std::pair<std::string, std::vector<std::string> >(output.str(), each);
}
