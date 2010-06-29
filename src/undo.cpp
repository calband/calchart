/* undo.cpp
 * Handle wxWindows interface
 *
 * Modification history:
 * 8-30-95    Garrick Meeker              Created
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

#include "undo.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include <wx/utils.h>

ShowUndo::~ShowUndo() {}

ShowUndoMany::ShowUndoMany(ShowUndo *undolist)
:ShowUndo(0), list(undolist) {}

ShowUndoMany::~ShowUndoMany()
{
	ShowUndo *next;

	while (list)
	{
		next = list->next;
		delete list;
		list = next;
	}
}


int ShowUndoMany::Undo(CC_show *show, ShowUndo** newundo)
{
	ShowUndo *undo;
	ShowUndo *newnode;
	ShowUndo *newlist;
	int i;

	newlist = newnode = NULL;
	i = -1;
	for (undo = list; undo != NULL; undo = undo->next)
	{
		if (newlist)
		{
			undo->Undo(show, &newnode->next);
			if (newnode->next) newnode = newnode->next;
		}
		else
		{
			i = undo->Undo(show, &newlist);
			newnode = newlist;
		}
	}
	*newundo = new ShowUndoMany(newlist);
	return i;
}


unsigned ShowUndoMany::Size()
{
	ShowUndo *undo;
	unsigned i;

	for (i = sizeof(*this), undo = list; undo != NULL; undo = undo->next)
	{
		i += undo->Size();
	}
	return i;
}


const wxChar *ShowUndoMany::UndoDescription()
{
	if (list) return list->UndoDescription();
	else return NULL;
}


const wxChar *ShowUndoMany::RedoDescription()
{
	if (list) return list->RedoDescription();
	else return NULL;
}


ShowUndoMove::ShowUndoMove(unsigned sheetnum, CC_sheet *sheet, unsigned ref)
:ShowUndo(sheetnum), refnum(ref)
{
	unsigned i, j;

	num = sheet->GetNumSelectedPoints();
	elems.assign(num, ShowUndoMoveElem());
	for (num=0,i=0; i < sheet->show->GetNumPoints(); i++)
	{
		if (sheet->show->IsSelected(i))
		{
			elems[num].idx = i;
			elems[num].pos = sheet->GetPosition(i, refnum);
			elems[num].refmask = 1<<ref;
			if (refnum == 0) for (j = 1; j <= NUM_REF_PNTS; j++)
			{
				if (sheet->GetPosition(i, j) == sheet->GetPosition(i, 0))
				{
					elems[num].refmask |= 1<<j;
				}
			}
			num++;
		}
	}
}


ShowUndoMove::ShowUndoMove(ShowUndoMove* old, CC_sheet *sheet)
:ShowUndo(old->sheetidx), num(old->num), refnum(old->refnum)
{
	unsigned i;

	elems.assign(num, ShowUndoMoveElem());
	for (i = 0; i < num; i++)
	{
		elems[i].idx = old->elems[i].idx;
		elems[i].refmask = old->elems[i].refmask;
		elems[i].pos = sheet->GetPosition(elems[i].idx, refnum);
	}
}


ShowUndoMove::~ShowUndoMove()
{
}


int ShowUndoMove::Undo(CC_show *show, ShowUndo** newundo)
{
	unsigned i, j;
	CC_sheet *sheet = show->GetNthSheet(sheetidx);

	*newundo = new ShowUndoMove(this, sheet);
	for (i = 0; i < num; i++)
	{
		for (j = 0; j <= NUM_REF_PNTS; j++)
		{
			if (elems[i].refmask & (1<<j))
			{
				sheet->SetPositionQuick(elems[i].pos, elems[i].idx, j);
			}
		}
	}
	gTheApp->GetWindowList().UpdatePointsOnSheet(sheetidx, refnum);
	return (int)sheetidx;
}


unsigned ShowUndoMove::Size()
{
	return sizeof(ShowUndoMoveElem) * num + sizeof(*this);
}


const wxChar *ShowUndoMove::UndoDescription() { return wxT("Undo movement"); }
const wxChar *ShowUndoMove::RedoDescription() { return wxT("Redo movement"); }

ShowUndoSym::ShowUndoSym(unsigned sheetnum, CC_sheet *sheet, bool contchanged)
:ShowUndo(sheetnum), contchange(contchanged)
{
	unsigned i;

	num = sheet->GetNumSelectedPoints();
	elems.assign(num, ShowUndoSymElem());
	for (num=0,i=0; i < sheet->show->GetNumPoints(); i++)
	{
		if (sheet->show->IsSelected(i))
		{
			elems[num].idx = i;
			elems[num].sym = sheet->GetPoint(i).sym;
			elems[num].cont = sheet->GetPoint(i).cont;
			num++;
		}
	}
}


ShowUndoSym::ShowUndoSym(ShowUndoSym* old, CC_sheet *sheet)
:ShowUndo(old->sheetidx), num(old->num), contchange(old->contchange)
{
	unsigned i;

	elems.assign(num, ShowUndoSymElem());
	for (i = 0; i < num; i++)
	{
		elems[i].idx = old->elems[i].idx;
		elems[i].sym = sheet->GetPoint(elems[i].idx).sym;
		elems[i].cont = sheet->GetPoint(elems[i].idx).cont;
	}
}


ShowUndoSym::~ShowUndoSym()
{
}


int ShowUndoSym::Undo(CC_show *show, ShowUndo** newundo)
{
	unsigned i;
	CC_sheet *sheet = show->GetNthSheet(sheetidx);

	*newundo = new ShowUndoSym(this, sheet);
	for (i = 0; i < num; i++)
	{
		sheet->GetPoint(elems[i].idx).sym = elems[i].sym;
		sheet->GetPoint(elems[i].idx).cont = elems[i].cont;
	}
	gTheApp->GetWindowList().UpdatePointsOnSheet(sheetidx);
	return (int)sheetidx;
}


unsigned ShowUndoSym::Size()
{
	return sizeof(ShowUndoSymElem) * num + sizeof(*this);
}


const wxChar *ShowUndoSym::UndoDescription()
{
	return contchange ? wxT("Undo continuity assignment") : wxT("Undo symbol change");
}


const wxChar *ShowUndoSym::RedoDescription()
{
	return contchange ? wxT("Redo continuity assignment") : wxT("Redo symbol change");
}


ShowUndoLbl::ShowUndoLbl(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum)
{
	unsigned i;

	num = sheet->GetNumSelectedPoints();
	elems.assign(num, ShowUndoLblElem());
	for (num=0,i=0; i < sheet->show->GetNumPoints(); i++)
	{
		if (sheet->show->IsSelected(i))
		{
			elems[num].idx = i;
			elems[num].right = sheet->GetPoint(i).GetFlip();
			num++;
		}
	}
}


ShowUndoLbl::ShowUndoLbl(ShowUndoLbl* old, CC_sheet *sheet)
:ShowUndo(old->sheetidx), num(old->num)
{
	unsigned i;

	elems.assign(num, ShowUndoLblElem());
	for (i = 0; i < num; i++)
	{
		elems[i].idx = old->elems[i].idx;
		elems[i].right = sheet->GetPoint(elems[i].idx).GetFlip();
	}
}


ShowUndoLbl::~ShowUndoLbl()
{
}


int ShowUndoLbl::Undo(CC_show *show, ShowUndo** newundo)
{
	unsigned i;
	CC_sheet *sheet = show->GetNthSheet(sheetidx);

	*newundo = new ShowUndoLbl(this, sheet);
	for (i = 0; i < num; i++)
	{
		sheet->GetPoint(elems[i].idx).Flip(elems[i].right);
	}
	gTheApp->GetWindowList().UpdatePointsOnSheet(sheetidx);
	return (int)sheetidx;
}


unsigned ShowUndoLbl::Size()
{
	return sizeof(ShowUndoLblElem) * num + sizeof(*this);
}


const wxChar *ShowUndoLbl::UndoDescription() { return wxT("Undo label location"); }
const wxChar *ShowUndoLbl::RedoDescription() { return wxT("Redo label location"); }

ShowUndoCopy::ShowUndoCopy(unsigned sheetnum)
:ShowUndo(sheetnum)
{
}


ShowUndoCopy::~ShowUndoCopy()
{
}


int ShowUndoCopy::Undo(CC_show *show, ShowUndo** newundo)
{
	CC_sheet *sheet = show->RemoveNthSheet(sheetidx);
	*newundo = new ShowUndoDelete(sheetidx, sheet);
	if (sheetidx > 0)
	{
		return (sheetidx-1);
	}
	else
	{
		return 0;
	}
}


unsigned ShowUndoCopy::Size() { return sizeof(*this); }

const wxChar *ShowUndoCopy::UndoDescription() { return wxT("Undo copy stuntsheet"); }
const wxChar *ShowUndoCopy::RedoDescription() { return wxT("Redo delete stuntsheet"); }

ShowUndoDelete::ShowUndoDelete(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), deleted_sheet(sheet)
{
}


ShowUndoDelete::~ShowUndoDelete()
{
	if (deleted_sheet) delete deleted_sheet;
}


int ShowUndoDelete::Undo(CC_show *show, ShowUndo** newundo)
{
	show->InsertSheet(deleted_sheet, sheetidx);
	*newundo = new ShowUndoCopy(sheetidx);
	deleted_sheet = NULL;						  // So it isn't deleted
	return (int)sheetidx;
}


unsigned ShowUndoDelete::Size()
{
	return sizeof(*this) + sizeof(*deleted_sheet);
}


const wxChar *ShowUndoDelete::UndoDescription() { return wxT("Undo delete stuntsheet"); }
const wxChar *ShowUndoDelete::RedoDescription() { return wxT("Redo copy stuntsheet"); }

ShowUndoAppendSheets::ShowUndoAppendSheets(unsigned sheetnum)
:ShowUndo(sheetnum)
{
}


ShowUndoAppendSheets::~ShowUndoAppendSheets()
{
}


int ShowUndoAppendSheets::Undo(CC_show *show, ShowUndo** newundo)
{
	CC_sheet *sheet = show->RemoveLastSheets(sheetidx);
	*newundo = new ShowUndoDeleteAppendSheets(sheet);
	return -1;
}


unsigned ShowUndoAppendSheets::Size() { return sizeof(*this); }

const wxChar *ShowUndoAppendSheets::UndoDescription() { return wxT("Undo append sheets"); }
const wxChar *ShowUndoAppendSheets::RedoDescription() { return wxT(""); }

ShowUndoDeleteAppendSheets::ShowUndoDeleteAppendSheets(CC_sheet *sheet)
:ShowUndo(0), deleted_sheets(sheet)
{
}


ShowUndoDeleteAppendSheets::~ShowUndoDeleteAppendSheets()
{
	CC_sheet *sheet;

	while (deleted_sheets)
	{
		sheet = deleted_sheets->next;
		delete deleted_sheets;
		deleted_sheets = sheet;
	}
}


int ShowUndoDeleteAppendSheets::Undo(CC_show *show, ShowUndo** newundo)
{
	*newundo = new ShowUndoAppendSheets(show->GetNumSheets());
	show->Append(deleted_sheets);
	deleted_sheets = NULL;						  // So they aren't deleted
	return -1;
}


unsigned ShowUndoDeleteAppendSheets::Size()
{
	unsigned i;
	CC_sheet *sheet;

	for (i = sizeof(*this), sheet = deleted_sheets;
		sheet != NULL;
		sheet = sheet->next)
	{
		i += sizeof(deleted_sheets);
	}
	return i;
}


const wxChar *ShowUndoDeleteAppendSheets::UndoDescription() { return wxT(""); }
const wxChar *ShowUndoDeleteAppendSheets::RedoDescription() { return wxT("Redo append sheets"); }

ShowUndoName::ShowUndoName(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum)
{

	name = sheet->GetName();
}


ShowUndoName::~ShowUndoName()
{
}


int ShowUndoName::Undo(CC_show *show, ShowUndo** newundo)
{
	CC_sheet *sheet = show->GetNthSheet(sheetidx);

	*newundo = new ShowUndoName(sheetidx, sheet);
	sheet->SetName(name);
	gTheApp->GetWindowList().ChangeTitle(sheetidx);
	return (int)sheetidx;
}


unsigned ShowUndoName::Size()
{
	return name.length()*sizeof(wxChar) + sizeof(*this);
}


const wxChar *ShowUndoName::UndoDescription() { return wxT("Undo change stuntsheet name"); }
const wxChar *ShowUndoName::RedoDescription() { return wxT("Redo change stuntsheet name"); }

ShowUndoBeat::ShowUndoBeat(unsigned sheetnum, CC_sheet *sheet)
:ShowUndo(sheetnum), beats(sheet->GetBeats()) {}

ShowUndoBeat::~ShowUndoBeat() {}

int ShowUndoBeat::Undo(CC_show *show, ShowUndo** newundo)
{
	CC_sheet *sheet = show->GetNthSheet(sheetidx);

	*newundo = new ShowUndoBeat(sheetidx, sheet);
	sheet->SetBeats(beats);
	gTheApp->GetWindowList().ChangeTitle(sheetidx);
	return (int)sheetidx;
}


unsigned ShowUndoBeat::Size()
{
	return sizeof(*this);
}


const wxChar *ShowUndoBeat::UndoDescription() { return wxT("Undo set number of beats"); }
const wxChar *ShowUndoBeat::RedoDescription() { return wxT("Redo set number of beats"); }

ShowUndoCont::ShowUndoCont(unsigned sheetnum, unsigned contnum,
CC_sheet *sheet)
:ShowUndo(sheetnum), cont(contnum)
{

	conttext = sheet->GetNthContinuity(cont)->text;
}


ShowUndoCont::~ShowUndoCont()
{
}


int ShowUndoCont::Undo(CC_show *show, ShowUndo** newundo)
{
	CC_sheet *sheet = show->GetNthSheet(sheetidx);

	*newundo = new ShowUndoCont(sheetidx, cont, sheet);
	sheet->SetNthContinuity(conttext, cont);
	gTheApp->GetWindowList().SetContinuity(NULL, sheetidx, cont);
	return (int)sheetidx;
}


unsigned ShowUndoCont::Size()
{
	return conttext.length()*sizeof(wxChar) + sizeof(*this);
}


const wxChar *ShowUndoCont::UndoDescription() { return wxT("Undo edit continuity"); }
const wxChar *ShowUndoCont::RedoDescription() { return wxT("Redo edit continuity"); }

ShowUndoAddContinuity::ShowUndoAddContinuity(unsigned sheetnum,
unsigned contnum)
:ShowUndo(sheetnum), addcontnum(contnum)
{
}


ShowUndoAddContinuity::~ShowUndoAddContinuity()
{
}


int ShowUndoAddContinuity::Undo(CC_show *show, ShowUndo** newundo)
{
	CC_continuity *cont;

	cont = show->GetNthSheet(sheetidx)->RemoveNthContinuity(addcontnum);
	*newundo = new ShowUndoDeleteContinuity(sheetidx, addcontnum, cont);
	return (int)sheetidx;
}


unsigned ShowUndoAddContinuity::Size() { return sizeof(*this); }

const wxChar *ShowUndoAddContinuity::UndoDescription()
{
	return wxT("Undo add continuity");
}


const wxChar *ShowUndoAddContinuity::RedoDescription()
{
	return wxT("Redo delete continuity");
}


ShowUndoDeleteContinuity::ShowUndoDeleteContinuity(unsigned sheetnum,
unsigned contnum,
CC_continuity *cont)
:ShowUndo(sheetnum), deleted_cont(cont), delcontnum(contnum)
{
}


ShowUndoDeleteContinuity::~ShowUndoDeleteContinuity()
{
	if (deleted_cont) delete deleted_cont;
}


int ShowUndoDeleteContinuity::Undo(CC_show *show, ShowUndo** newundo)
{
	show->GetNthSheet(sheetidx)->InsertContinuity(deleted_cont, delcontnum);
	*newundo = new ShowUndoAddContinuity(sheetidx, delcontnum);
	deleted_cont = NULL;						  // So it isn't deleted
	return (int)sheetidx;
}


unsigned ShowUndoDeleteContinuity::Size()
{
	return sizeof(*this) + sizeof(*deleted_cont);
}


const wxChar *ShowUndoDeleteContinuity::UndoDescription()
{
	return wxT("Undo delete continuity");
}


const wxChar *ShowUndoDeleteContinuity::RedoDescription()
{
	return wxT("Redo add continuity");
}


ShowUndoDescr::ShowUndoDescr(CC_show *show)
:ShowUndo(0)
{

	descrtext = show->GetDescr();
}


ShowUndoDescr::~ShowUndoDescr()
{
}


int ShowUndoDescr::Undo(CC_show *show, ShowUndo** newundo)
{
	*newundo = new ShowUndoDescr(show);
	show->SetDescr(descrtext);
	gTheApp->GetWindowList().SetDescr(NULL);
	return -1;									  // don't goto another sheet
}


unsigned ShowUndoDescr::Size()
{
	return descrtext.length()*sizeof(wxChar) + sizeof(*this);
}


const wxChar *ShowUndoDescr::UndoDescription() { return wxT("Undo edit show description"); }
const wxChar *ShowUndoDescr::RedoDescription() { return wxT("Redo edit show description"); }

ShowUndoList::ShowUndoList(CC_show *shw, int lim)
:show(shw), list(NULL), redolist(NULL), limit(lim) {}

ShowUndoList::~ShowUndoList()
{
	EraseAll();
}


// Execute one undo
int ShowUndoList::Undo(CC_show *show)
{
	unsigned i;
	ShowUndo *undo = Pop();
	ShowUndo *redo;

	if (undo)
	{
		redo = NULL;
		i = undo->Undo(show, &redo);
		if (redo) PushRedo(redo);
		show->SetModified(undo->was_modified);
		delete undo;
		return (int)i;
	}
	else
	{
		return -1;
	}
}


// Execute one redo
int ShowUndoList::Redo(CC_show *show)
{
	unsigned i;
	ShowUndo *undo = PopRedo();
	ShowUndo *newundo;

	if (undo)
	{
		newundo = NULL;
		i = undo->Undo(show, &newundo);
		if (newundo) Push(newundo);
		show->SetModified(true);
		delete undo;
		return (int)i;
	}
	else
	{
		return -1;
	}
}


// Add an entry to list and clean if necessary
void ShowUndoList::Add(ShowUndo *undo)
{
	EraseAllRedo();
	Push(undo);
	Clean();
}


// Prepare to add multiple actions
void ShowUndoList::StartMulti()
{
	oldlist = list;
}


// End adding multiple actions
void ShowUndoList::EndMulti()
{
	ShowUndo *newlist, *tmplist;

	newlist = NULL;
	for (tmplist = list; tmplist != NULL; tmplist = tmplist->next)
	{
		if (tmplist->next == oldlist)
		{
			newlist = list;
			list = oldlist;
			tmplist->next = NULL;
			break;
		}
	}
	if (newlist)
	{
		Add(new ShowUndoMany(newlist));
	}
}


// Return description of undo stack
const wxChar *ShowUndoList::UndoDescription()
{
	if (list)
	{
		return list->UndoDescription();
	}
	else
	{
		return wxT("No undo information");
	}
}


// Return description of redo stack
const wxChar *ShowUndoList::RedoDescription()
{
	if (redolist)
	{
		return redolist->RedoDescription();
	}
	else
	{
		return wxT("No redo information");
	}
}


// Pop one entry off list
ShowUndo *ShowUndoList::Pop()
{
	ShowUndo *undo;

	undo = list;
	if (undo)
	{
		list = list->next;
	}
	return undo;
}


// Push one entry onto list
void ShowUndoList::Push(ShowUndo *undo)
{
	undo->next = list;
	undo->was_modified = show->Modified();
	show->SetModified(true);					  // Show is now modified
	list = undo;
}


// Pop one entry off list
ShowUndo *ShowUndoList::PopRedo()
{
	ShowUndo *undo;

	undo = redolist;
	if (undo)
	{
		redolist = redolist->next;
	}
	return undo;
}


// Push one entry onto list
void ShowUndoList::PushRedo(ShowUndo *undo)
{
	undo->next = redolist;
	undo->was_modified = true;					  // Show will be modified
	redolist = undo;
}


// Remove everything from list
void ShowUndoList::EraseAll()
{
	ShowUndo *undo;

	EraseAllRedo();
	while ((undo = Pop()) != NULL)
	{
		delete undo;
	}
}


// Remove everything from list
void ShowUndoList::EraseAllRedo()
{
	ShowUndo *undo;

	while ((undo = PopRedo()) != NULL)
	{
		delete undo;
	}
}


// Make sure list memory requirements
void ShowUndoList::Clean()
{
	ShowUndo *ptr, *tmpptr;
	unsigned size, total;

// < 0 means no limit
// 0 means no list
// > 0 means try to make total less than or equal to limit, but always
//     allow one event
	ptr = list;
	if (ptr != NULL)
	{
		if (limit >= 0)
		{
			if (limit > 0)
			{
				total = ptr->Size();
				tmpptr = ptr;
				ptr = ptr->next;
			}
			else
			{
				total = 0;
				tmpptr = NULL;
				list = NULL;
			}
			while (ptr != NULL)
			{
				size = ptr->Size();
				if ((total + size) <= (unsigned)limit)
				{
					total += size;
					tmpptr = ptr;
					ptr = ptr->next;
				} else break;
			}
			if (tmpptr != NULL)
			{
				tmpptr->next = NULL;
			}
// Delete rest of list
			while (ptr != NULL)
			{
				tmpptr = ptr->next;
				delete ptr;
				ptr = tmpptr;
			}
		}
	}
}


std::ostream& operator<< (std::ostream& s, const ShowUndoList& list)
{
	ShowUndo *elem;

	elem = list.list;
	while (elem)
	{
		s << elem->UndoDescription() << std::endl;
		elem = elem->next;
	}
	s << "*** End of undo stack." << std::endl;
	elem = list.redolist;
	while (elem)
	{
		s << elem->RedoDescription() << std::endl;
		elem = elem->next;
	}
	s << "*** End of redo stack." << std::endl;
	return s;
}
