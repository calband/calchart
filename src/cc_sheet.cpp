/* cc_winlist.cpp
 * Member functions for winlist and winnode classes
 *
 */

/*
   Copyright (C) 1995-2010  Richard Powell

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

#include "cc_sheet.h"
#include "cc_show.h"
#include "show.h"
#include "undo.h"
#include "modes.h"
#include "confgr.h"
#include "calchartapp.h"

CC_sheet::CC_sheet(CC_show *shw)
: next(NULL), show(shw),
picked(true), beats(1), pts(show->GetNumPoints())
{
}


CC_sheet::CC_sheet(CC_show *shw, const wxString& newname)
: next(NULL), show(shw),
picked(true), beats(1), pts(show->GetNumPoints()), name(newname)
{
}


CC_sheet::CC_sheet(CC_sheet *sht)
: next(NULL), show(sht->show),
picked(sht->picked), beats(1), pts(show->GetNumPoints()), name(sht->name), number(sht->number)
{
	int i;

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		pts[i] = sht->pts[i];
		pts[i].sym = SYMBOL_PLAIN;
		pts[i].cont = 0;
	}
	animcont.push_back(CC_continuity_ptr(new CC_continuity(contnames[0], 0)));
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
	bool changed = false;

	for (j = 0; j < show->GetNumPoints(); j++)
	{
		if (pts[j].cont == i)
		{
			if (!show->IsSelected(j))
			{
				show->Select(j, true);
				changed = true;
			}
		}
		else
		{
			if (show->IsSelected(j))
			{
				show->Select(j, false);
				changed = true;
			}
		}
	}
	return changed;
}


void CC_sheet::SetContinuityIndex(unsigned i)
{
	unsigned j;

// Create undo entry (sym also does continuity)
	show->undolist->Add(new ShowUndoSym(show->GetSheetPos(this), this, true));

	for (j = 0; j < show->GetNumPoints(); j++)
	{
		if (show->IsSelected(j))
		{
			pts[j].cont = i;
		}
	}
}


void CC_sheet::SetNumPoints(unsigned num, unsigned columns)
{
	unsigned i, j, cpy, col;
	CC_coord c, coff(show->GetMode().FieldOffset());
	CC_continuity_ptr plaincont;

	std::vector<CC_point> newpts(num);
	cpy = MIN(show->GetNumPoints(), num);
	for (i = 0; i < cpy; i++)
	{
		newpts[i] = pts[i];
	}
	for (c = coff, col = 0; i < num; i++, col++, c.x += INT2COORD(2))
	{
		plaincont = GetStandardContinuity(SYMBOL_PLAIN);
		if (col >= columns)
		{
			c.x = coff.x;
			c.y += INT2COORD(2);
			col = 0;
		}
		newpts[i].flags = 0;
		newpts[i].sym = SYMBOL_PLAIN;
		newpts[i].cont = plaincont->GetNum();
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


const CC_continuity_ptr CC_sheet::GetNthContinuity(unsigned i) const
{
	return animcont.at(i);
}

CC_continuity_ptr CC_sheet::GetNthContinuity(unsigned i)
{
	return animcont.at(i);
}


void CC_sheet::SetNthContinuity(const wxString& text, unsigned i)
{
	CC_continuity_ptr c;

	c = GetNthContinuity(i);
	if (c)
	{
		c->SetText(text);
	}
	show->UpdateAllViews();
}


CC_continuity_ptr CC_sheet::RemoveNthContinuity(unsigned i)
{
	CC_continuity_ptr cont = animcont.at(i);
	animcont.erase(animcont.begin()+i);
	wxGetApp().GetWindowList().DeleteContinuity(show->GetSheetPos(this), i);
	return cont;
}


void CC_sheet::UserDeleteContinuity(unsigned i)
{
	CC_continuity_ptr cont = RemoveNthContinuity(i);
	show->undolist->Add(new ShowUndoDeleteContinuity(show->GetSheetPos(this),
		i, cont));
}


void CC_sheet::InsertContinuity(CC_continuity_ptr newcont, unsigned i)
{
	animcont.insert(animcont.begin() + i, newcont);
	wxGetApp().GetWindowList().AddContinuity(show->GetSheetPos(this), i);
}


void CC_sheet::AppendContinuity(CC_continuity_ptr newcont)
{
	animcont.push_back(newcont);
}


CC_continuity_ptr CC_sheet::UserNewContinuity(const wxString& name)
{
	CC_continuity_ptr newcont(new CC_continuity(name, NextUnusedContinuityNum()));
	AppendContinuity(newcont);
	wxGetApp().GetWindowList().AddContinuity(show->GetSheetPos(this), animcont.size()-1);
	show->undolist->Add(new ShowUndoAddContinuity(show->GetSheetPos(this),
		animcont.size()-1));
	return newcont;
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
			if ((*c)->GetNum() == i)
			{
				found = true;
				i++;
				break;
			}
		}
	} while (found);
	return i;
}


CC_continuity_ptr CC_sheet::GetStandardContinuity(SYMBOL_TYPE sym)
{

	for (ContContainer::const_iterator c = animcont.begin(); c != animcont.end(); ++c)
	{
		if ((*c)->GetName().CompareTo(contnames[sym], wxString::ignoreCase) == 0)
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
	CC_continuity_ptr c(new CC_continuity(contnames[sym], NextUnusedContinuityNum()));
	InsertContinuity(c, idx);
	show->undolist->Add(new ShowUndoAddContinuity(show->GetSheetPos(this),
		idx));
	return c;
}


unsigned CC_sheet::FindContinuityByName(const wxString& name) const
{
	unsigned idx;
	ContContainer::const_iterator c = animcont.begin();

	for (idx = 1; c != animcont.end(); idx++, ++c)
	{
		if ((*c)->GetName().CompareTo(name, wxString::ignoreCase) == 0)
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
	const CC_continuity_ptr c = GetNthContinuity(idx);

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		if (pts[i].cont == c->GetNum()) return true;
	}
	return false;
}


void CC_sheet::UserSetName(const wxString& newname)
{
// Create undo entry
	show->undolist->Add(new ShowUndoName(show->GetSheetPos(this), this));
	SetName(newname);
	wxGetApp().GetWindowList().ChangeTitle(show->GetSheetPos(this));
}


void CC_sheet::UserSetBeats(unsigned short b)
{
// Create undo entry
	show->undolist->Add(new ShowUndoBeat(show->GetSheetPos(this), this));
	SetBeats(b);
	wxGetApp().GetWindowList().ChangeTitle(show->GetSheetPos(this));
}


// Set point symbols
bool CC_sheet::SetPointsSym(SYMBOL_TYPE sym)
{
	unsigned i;
	bool change = false;

	if (GetNumSelectedPoints() <= 0) return false;

// Create undo entries
	show->undolist->StartMulti();
	CC_continuity_ptr c = GetStandardContinuity(sym);
	show->undolist->Add(new ShowUndoSym(show->GetSheetPos(this), this));
	show->undolist->EndMulti();

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		if (show->IsSelected(i))
		{
			if (pts[i].sym != sym)
			{
				pts[i].sym = sym;
				pts[i].cont = c->GetNum();
				change = true;
			}
		}
	}
	return change;
}


// Set point labels
bool CC_sheet::SetPointsLabel(bool right)
{
	unsigned i;
	bool change = false;

	if (GetNumSelectedPoints() <= 0) return false;

// Create undo entry
	show->undolist->Add(new ShowUndoLbl(show->GetSheetPos(this), this));

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		if (show->IsSelected(i))
		{
			pts[i].Flip(right);
			change = true;
		}
	}
	return change;
}


// Set point labels
bool CC_sheet::SetPointsLabelFlip()
{
	unsigned i;
	bool change = false;

	if (GetNumSelectedPoints() <= 0) return false;

// Create undo entry
	show->undolist->Add(new ShowUndoLbl(show->GetSheetPos(this), this));

	for (i = 0; i < show->GetNumPoints(); i++)
	{
		if (show->IsSelected(i))
		{
			pts[i].FlipToggle();
			change = true;
		}
	}
	return change;
}


// Set a point from an old format disk point
void CC_sheet::SetPoint(const cc_oldpoint& val, unsigned i)
{
	pts[i].flags = (val.flags & OLD_FLAG_FLIP) ? PNT_LABEL:0;
	if ((val.sym < 111) || (val.sym > 118))
		pts[i].sym = SYMBOL_PLAIN;
	else pts[i].sym = (SYMBOL_TYPE)(val.sym - 111);
	pts[i].cont = get_lil_word(&val.cont);
	SetAllPositions(val.pos, i);
	for (unsigned j = 0; j < 3; j++)
	{
// -1 means undefined (endian doesn't matter)
		if ((val.ref[j].x == 0xFFFF) && (val.ref[j].y == 0xFFFF))
		{
			SetPosition(val.pos, i, j+1);
		}
		else
		{
			SetPosition(val.ref[j], i, j+1);
		}
	}
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


