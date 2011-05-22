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
#include "show.h"
#include "undo.h"
#include "modes.h"
#include "confgr.h"

CC_sheet::CC_sheet(CC_show *shw)
: show(shw),
picked(true), beats(1), pts(show->GetNumPoints())
{
}


CC_sheet::CC_sheet(CC_show *shw, const wxString& newname)
: show(shw),
picked(true), beats(1), pts(show->GetNumPoints()), name(newname)
{
}


CC_sheet::CC_sheet(CC_sheet *sht)
: show(sht->show),
picked(sht->picked), beats(1), pts(show->GetNumPoints()), name(sht->name), number(sht->number)
{
	int i;

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		pts[i] = sht->pts[i];
		pts[i].sym = SYMBOL_PLAIN;
		pts[i].cont = 0;
	}
	animcont.push_back(CC_continuity(contnames[0], 0));
}


CC_sheet::~CC_sheet()
{
}


// Return the number of selected points
unsigned CC_sheet::GetNumSelectedPoints() const
{
	unsigned i,num;
	for (i=0,num=0; i<show->GetNumPoints(); i++)
	{
		if (show->IsSelected(i)) num++;
	}
	return num;
}


// Find point at certain coords
int CC_sheet::FindPoint(Coord x, Coord y, unsigned ref) const
{
	unsigned i;
	CC_coord c;
	Coord w = FLOAT2COORD(dot_ratio);
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


bool CC_sheet::SelectContinuity(unsigned i) const
{
	unsigned j;

	CC_show::SelectionList select;
	for (j = 0; j < show->GetNumPoints(); j++)
	{
		if (pts[j].cont == i)
		{
			select.insert(j);
		}
	}
	show->SetSelection(select);
	return true;
}


void CC_sheet::SetNumPoints(unsigned num, unsigned columns)
{
	unsigned i, j, cpy, col;
	CC_coord c, coff(show->GetMode().FieldOffset());

	std::vector<CC_point> newpts(num);
	cpy = MIN(show->GetNumPoints(), num);
	for (i = 0; i < cpy; i++)
	{
		newpts[i] = pts[i];
	}
	for (c = coff, col = 0; i < num; i++, col++, c.x += INT2COORD(2))
	{
		const CC_continuity& plaincont = GetStandardContinuity(SYMBOL_PLAIN);
		if (col >= columns)
		{
			c.x = coff.x;
			c.y += INT2COORD(2);
			col = 0;
		}
		newpts[i].flags = 0;
		newpts[i].sym = SYMBOL_PLAIN;
		newpts[i].cont = plaincont.GetNum();
		newpts[i].pos = c;
		for (j = 0; j < NUM_REF_PNTS; j++)
		{
			newpts[i].ref[j] = newpts[i].pos;
		}
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

CC_continuity& CC_sheet::GetNthContinuity(unsigned i)
{
	return animcont.at(i);
}


void CC_sheet::SetNthContinuity(const wxString& text, unsigned i)
{
	CC_continuity& c = GetNthContinuity(i);
	c.SetText(text);
	show->Modify(true);
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
		if (c->GetName().CompareTo(contnames[sym], wxString::ignoreCase) == 0)
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
		if (c->GetName().CompareTo(name, wxString::ignoreCase) == 0)
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
		if (pts[i].cont == c.GetNum()) return true;
	}
	return false;
}


const wxString& CC_sheet::GetName() const
{
	return name;
}

void CC_sheet::SetName(const wxString& newname)
{
	name = newname;
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
const CC_coord& CC_sheet::GetPosition(unsigned i, unsigned ref) const
{
	if (ref == 0)
		return pts[i].pos;
	else
		return pts[i].ref[ref-1];
}


// Set position of point and all refs
void CC_sheet::SetAllPositions(const CC_coord& val, unsigned i)
{
	unsigned j;

	pts[i].pos = val;
	for (j = 0; j < NUM_REF_PNTS; j++)
	{
		pts[i].ref[j] = val;
	}
}


// Set position of point
void CC_sheet::SetPosition(const CC_coord& val, unsigned i, unsigned ref)
{
	unsigned j;
	CC_coord clippedval = show->GetMode().ClipPosition(val);
	if (ref == 0)
	{
		for (j=0; j<NUM_REF_PNTS; j++)
		{
			if (pts[i].ref[j] == pts[i].pos)
			{
				pts[i].ref[j] = clippedval;
			}
		}
		pts[i].pos = clippedval;
	}
	else
	{
		pts[i].ref[ref-1] = clippedval;
	}
}


// Set position of point and don't touch reference points
void CC_sheet::SetPositionQuick(const CC_coord& val, unsigned i, unsigned ref)
{
	CC_coord clippedval = show->GetMode().ClipPosition(val);
	if (ref == 0)
	{
		pts[i].pos = clippedval;
	}
	else
	{
		pts[i].ref[ref-1] = clippedval;
	}
}

