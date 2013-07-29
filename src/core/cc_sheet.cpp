/*
 * cc_sheet.cpp
 * Defintion for calchart sheet class
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

#include "cc_sheet.h"
#include "cc_show.h"
#include "cc_command.h"
#include "modes.h"
#include "confgr.h"
#include "draw.h"
#include <boost/algorithm/string/predicate.hpp>
#include <wx/wx.h>

const std::string contnames[] =
{
	"Plain",
	"Sol",
	"Bksl",
	"Sl",
	"X",
	"Solbksl",
	"Solsl",
	"Solx"
};


CC_sheet::CC_sheet(CC_show *shw) :
show(shw),
beats(1),
pts(show->GetNumPoints())
{
}


CC_sheet::CC_sheet(CC_show *shw, const std::string& newname) :
show(shw),
beats(1),
pts(show->GetNumPoints()),
name(newname)
{
}


CC_sheet::~CC_sheet()
{
}


// Find point at certain coords
int CC_sheet::FindPoint(Coord x, Coord y, unsigned ref) const
{
	unsigned i;
	CC_coord c;
	Coord w = Float2Coord(GetConfiguration_DotRatio());
	for (i = 0; i < show->GetNumPoints(); i++)
	{
		c = GetPosition(i, ref);
		if (((x+w) >= c.x) && ((x-w) <= c.x) && ((y+w) >= c.y) && ((y-w) <= c.y))
		{
			return i;
		}
	}
	return -1;
}


std::set<unsigned>
CC_sheet::SelectPointsOfContinuity(unsigned i) const
{
	unsigned j;

	CC_show::SelectionList select;
	for (j = 0; j < show->GetNumPoints(); j++)
	{
		if (pts[j].GetContinuityIndex() == i)
		{
			select.insert(j);
		}
	}
	return select;
}


void CC_sheet::SetNumPoints(unsigned num, unsigned columns)
{
	unsigned i, cpy, col;
	CC_coord c, coff(show->GetMode().FieldOffset());

	std::vector<CC_point> newpts(num);
	cpy = std::min<unsigned>(show->GetNumPoints(), num);
	for (i = 0; i < cpy; i++)
	{
		newpts[i] = pts[i];
	}
	for (c = coff, col = 0; i < num; i++, col++, c.x += Int2Coord(2))
	{
		const CC_continuity& plaincont = GetStandardContinuity(SYMBOL_PLAIN);
		if (col >= columns)
		{
			c.x = coff.x;
			c.y += Int2Coord(2);
			col = 0;
		}
		newpts[i] = CC_point(plaincont.GetNum(), c);
	}
	pts = newpts;
}


void CC_sheet::RelabelSheet(unsigned *table)
{
	std::vector<CC_point> newpts(show->GetNumPoints());
	for (unsigned i = 0; i < show->GetNumPoints(); i++)
	{
		newpts[i] = pts[table[i]];
	}
	pts = newpts;
}


const CC_continuity& CC_sheet::GetNthContinuity(unsigned i) const
{
	return animcont.at(i);
}


void CC_sheet::SetNthContinuity(const wxString& text, unsigned i)
{
	std::string ttext = text.ToStdString();
	animcont.at(i).SetText(ttext);
}


CC_continuity CC_sheet::RemoveNthContinuity(unsigned i)
{
	CC_continuity cont = animcont.at(i);
	animcont.erase(animcont.begin()+i);
	return cont;
}


void CC_sheet::InsertContinuity(const CC_continuity& newcont, unsigned i)
{
	animcont.insert(animcont.begin() + i, newcont);
}


void CC_sheet::AppendContinuity(const CC_continuity& newcont)
{
	animcont.push_back(newcont);
}


unsigned CC_sheet::NextUnusedContinuityNum()
{
	unsigned i = 0;
	bool found;

	do
	{
		found = false;
		for (ContContainer::const_iterator c = animcont.begin(); c != animcont.end(); ++c)
		{
			if (c->GetNum() == i)
			{
				found = true;
				i++;
				break;
			}
		}
	} while (found);
	return i;
}


// not undoable
const CC_continuity& CC_sheet::GetStandardContinuity(SYMBOL_TYPE sym)
{

	for (ContContainer::const_iterator c = animcont.begin(); c != animcont.end(); ++c)
	{
		if (boost::iequals(c->GetName(), contnames[sym]))
		{
			return *c;
		}
	}

	unsigned i,idx;
	i = (unsigned)sym;
	idx = 0;
// Put in correct postion
	while (i > 0)
	{
		idx = FindContinuityByName(contnames[--i]);
		if (idx != 0) break;
	}
	InsertContinuity(CC_continuity(contnames[sym], NextUnusedContinuityNum()), idx);
	return GetNthContinuity(idx);
}


unsigned CC_sheet::FindContinuityByName(const wxString& name) const
{
	unsigned idx;
	ContContainer::const_iterator c = animcont.begin();

	for (idx = 1; c != animcont.end(); idx++, ++c)
	{
		if (boost::iequals(c->GetName(), name))
		{
			break;
		}
	}
	if (c == animcont.end())
	{
		idx = 0;
	}
	return idx;
}


bool CC_sheet::ContinuityInUse(unsigned idx) const
{
	unsigned i;
	const CC_continuity& c = GetNthContinuity(idx);

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		if (pts[i].GetContinuityIndex() == c.GetNum()) return true;
	}
	return false;
}


std::string CC_sheet::GetName() const
{
	return name;
}

void CC_sheet::SetName(const std::string& newname)
{
	name = newname;
}

std::string CC_sheet::GetNumber() const
{
	return number;
}

void CC_sheet::SetNumber(const std::string& num)
{
	number = num;
}


unsigned short CC_sheet::GetBeats() const
{
	return beats;
}


void CC_sheet::SetBeats(unsigned short b)
{
	beats = b;
}


// Get position of point
CC_coord CC_sheet::GetPosition(unsigned i, unsigned ref) const
{
	return pts[i].GetPos(ref);
}


// Set position of point and all refs
void CC_sheet::SetAllPositions(const CC_coord& val, unsigned i)
{
	for (unsigned j = 0; j <= CC_point::kNumRefPoints; j++)
	{
		pts[i].SetPos(val, j);
	}
}


// Set position of point
void CC_sheet::SetPosition(const CC_coord& val, unsigned i, unsigned ref)
{
	unsigned j;
	CC_coord clippedval = show->GetMode().ClipPosition(val);
	if (ref == 0)
	{
		for (j = 1; j <= CC_point::kNumRefPoints; j++)
		{
			if (pts[i].GetPos(j) == pts[i].GetPos(0))
			{
				pts[i].SetPos(clippedval, j);
			}
		}
		pts[i].SetPos(clippedval);
	}
	else
	{
		pts[i].SetPos(clippedval, ref);
	}
}


