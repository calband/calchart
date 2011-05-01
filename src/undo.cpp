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
#include "cc_continuity.h"
#include <wx/utils.h>

ShowUndo::~ShowUndo() {}

MovePointsOnSheetCommand::MovePointsOnSheetCommand(CC_show& show, unsigned ref)
: wxCommand(true, wxT("Moving points")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList()), mRef(ref)
{
}

MovePointsOnSheetCommand::~MovePointsOnSheetCommand()
{
}


bool MovePointsOnSheetCommand::Do()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<CC_coord,CC_coord> >::const_iterator i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(i->second.second, i->first, mRef);
	}
	mShow.Modify(true);
	wxGetApp().GetWindowList().UpdatePointsOnSheet(mSheetNum, mRef);
	return true;
}

bool MovePointsOnSheetCommand::Undo()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<CC_coord,CC_coord> >::const_iterator i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(i->second.first, i->first, mRef);
	}
	mShow.Modify(true);
	wxGetApp().GetWindowList().UpdatePointsOnSheet(mSheetNum, mRef);
	return true;
}

TranslatePointsByDeltaCommand::TranslatePointsByDeltaCommand(CC_show& show, const CC_coord& delta, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mPositions[*i] = std::pair<CC_coord,CC_coord>(sheet->GetPosition(*i, mRef), sheet->GetPosition(*i, mRef) + delta);
	}
}

TranslatePointsByDeltaCommand::~TranslatePointsByDeltaCommand()
{
}


// Move points
TransformPointsCommand::TransformPointsCommand(CC_show& show, const Matrix& transmat, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		CC_coord c = sheet->GetPosition(*i, ref);
		Vector v = Vector(c.x, c.y, 0);
		v = transmat * v;
		v.Homogenize();
		c = CC_coord((Coord)CLIPFLOAT(v.GetX()), (Coord)CLIPFLOAT(v.GetY()));
		mPositions[*i] = std::pair<CC_coord,CC_coord>(sheet->GetPosition(*i, mRef), c);
	}
}

TransformPointsCommand::~TransformPointsCommand()
{
}

// Move points into a line (old smart move)
TransformPointsInALineCommand::TransformPointsInALineCommand(CC_show& show, const CC_coord& start, const CC_coord& second, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	CC_coord curr_pos = start;
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i, curr_pos += second - start)
	{
		mPositions[*i] = std::pair<CC_coord,CC_coord>(sheet->GetPosition(*i, mRef), curr_pos);
	}
}

TransformPointsInALineCommand::~TransformPointsInALineCommand()
{
}


SetContinuityIndexCommand::SetContinuityIndexCommand(CC_show& show, unsigned index)
: wxCommand(true, wxT("Setting Continuity Index")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList())
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mContinuity[*i] = std::pair<unsigned,unsigned>(sheet->GetPoint(*i).cont, index);
	}
}

SetContinuityIndexCommand::~SetContinuityIndexCommand()
{
}

bool SetContinuityIndexCommand::Do()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<unsigned,unsigned> >::const_iterator i = mContinuity.begin(); i != mContinuity.end(); ++i)
	{
		sheet->GetPoint(i->first).cont = i->second.second;
	}
	wxGetApp().GetWindowList().UpdatePointsOnSheet(mSheetNum);
	return true;
}

bool SetContinuityIndexCommand::Undo()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<unsigned,unsigned> >::const_iterator i = mContinuity.begin(); i != mContinuity.end(); ++i)
	{
		sheet->GetPoint(i->first).cont = i->second.first;
	}
	wxGetApp().GetWindowList().UpdatePointsOnSheet(mSheetNum);
	return true;
}


SetSymbolAndContCommand::SetSymbolAndContCommand(CC_show& show, SYMBOL_TYPE sym)
: wxCommand(true, wxT("Setting Continuity Index")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList())
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	// because getting the standard continuity can create one, we need to make it so we can undo
	// the modification of the continuity list.
	mOrigAnimcont = sheet->animcont;
	
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		// Only do work on points that have different symbols
		if (sym != sheet->GetPoint(*i).sym)
		{
			mSymsAndCont[*i] = std::pair<sym_cont_t,sym_cont_t>(sym_cont_t(sheet->GetPoint(*i).sym, sheet->GetPoint(*i).cont), sym_cont_t(sym, sheet->GetPoint(*i).cont));
		}
	}
}

SetSymbolAndContCommand::~SetSymbolAndContCommand()
{
}

bool SetSymbolAndContCommand::Do()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

// possible optimization:  Don't save as map, since they all get moved to new symbol...
	for (std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> >::const_iterator i = mSymsAndCont.begin(); i != mSymsAndCont.end(); ++i)
	{
		sheet->GetPoint(i->first).sym = i->second.second.first;
		sheet->GetPoint(i->first).cont = sheet->GetStandardContinuity(i->second.second.first).GetNum();
	}
	wxGetApp().GetWindowList().UpdatePointsOnSheet(mSheetNum);
	return true;
}

bool SetSymbolAndContCommand::Undo()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> >::const_iterator i = mSymsAndCont.begin(); i != mSymsAndCont.end(); ++i)
	{
		sheet->GetPoint(i->first).sym = i->second.first.first;
		sheet->GetPoint(i->first).cont = i->second.first.second;
	}
	sheet->animcont = mOrigAnimcont;
	wxGetApp().GetWindowList().UpdatePointsOnSheet(mSheetNum);
	return true;
}


SetContinuityTextCommand::SetContinuityTextCommand(CC_show& show, unsigned i, const wxString& text)
: wxCommand(true, wxT("Setting Continuity Text")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList()), mWhichCont(i)
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	mContinuity = std::pair<wxString,wxString>(sheet->GetNthContinuity(mWhichCont).GetText(), text);
}

SetContinuityTextCommand::~SetContinuityTextCommand()
{
}

bool SetContinuityTextCommand::Do()
{
//	mShow.UnselectAll();
//	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
//		mShow.Select(*i, true);
//
	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	sheet->SetNthContinuity(mContinuity.second, mWhichCont);
	return true;
}

bool SetContinuityTextCommand::Undo()
{
//	mShow.UnselectAll();
//	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
//		mShow.Select(*i, true);
//
	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	sheet->SetNthContinuity(mContinuity.first, mWhichCont);
	return true;
}


SetLabelCommand::SetLabelCommand(CC_show& show)
: wxCommand(true, wxT("Moving points")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList())
{
}

SetLabelCommand::~SetLabelCommand()
{
}


bool SetLabelCommand::Do()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<bool,bool> >::const_iterator i = mLabelPos.begin(); i != mLabelPos.end(); ++i)
	{
		sheet->GetPoint(i->first).Flip(i->second.second);
	}
	mShow.Modify(true);
	return true;
}

bool SetLabelCommand::Undo()
{
	mShow.UnselectAll();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
		mShow.Select(*i, true);

	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<bool,bool> >::const_iterator i = mLabelPos.begin(); i != mLabelPos.end(); ++i)
	{
		sheet->GetPoint(i->first).Flip(i->second.first);
	}
	mShow.Modify(true);
	return true;
}

SetLabelRightCommand::SetLabelRightCommand(CC_show& show, bool right)
: SetLabelCommand(show)
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = std::pair<bool,bool>(sheet->GetPoint(*i).GetFlip(), right);
	}
}

SetLabelRightCommand::~SetLabelRightCommand()
{
}


SetLabelFlipCommand::SetLabelFlipCommand(CC_show& show)
: SetLabelCommand(show)
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = std::pair<bool,bool>(sheet->GetPoint(*i).GetFlip(), !sheet->GetPoint(*i).GetFlip());
	}
}

SetLabelFlipCommand::~SetLabelFlipCommand()
{
}


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

// Added or remove continuity base class
AddRemoveContinuityCommand::AddRemoveContinuityCommand(CC_show& show)
: wxCommand(true, wxT("Adding or removing continuity")),
mShow(show), mSheetNum(show.GetCurrentSheetNum())
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	mOrigAnimcont = sheet->animcont;
}

AddRemoveContinuityCommand::~AddRemoveContinuityCommand()
{
}

bool AddRemoveContinuityCommand::Undo()
{
	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();
	sheet->animcont = mOrigAnimcont;
	mShow.Modify(true);
	return true;
}

AddContinuityCommand::AddContinuityCommand(CC_show& show, const wxString& text)
: AddRemoveContinuityCommand(show),
mContName(text)
{
}

AddContinuityCommand::~AddContinuityCommand()
{
}

bool AddContinuityCommand::Do()
{
	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();
	CC_continuity newcont(mContName, sheet->NextUnusedContinuityNum());
	sheet->AppendContinuity(newcont);
	mShow.Modify(true);
	return true;
}

RemoveContinuityCommand::RemoveContinuityCommand(CC_show& show, unsigned index)
: AddRemoveContinuityCommand(show),
mIndexToRemove(index)
{
}

RemoveContinuityCommand::~RemoveContinuityCommand()
{
}

bool RemoveContinuityCommand::Do()
{
	mShow.SetCurrentSheet(mSheetNum);
	CC_sheet *sheet = mShow.GetCurrentSheet();
	sheet->RemoveNthContinuity(mIndexToRemove);
	mShow.Modify(true);
	return true;
}


SetSheetTitleCommand::SetSheetTitleCommand(CC_show& show, const wxString& newname)
: wxCommand(true, wxT("Moving points")),
mShow(show), mSheetNum(show.GetCurrentSheetNum())
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	mDescription = std::pair<wxString,wxString>(sheet->GetName(), newname);
}

SetSheetTitleCommand::~SetSheetTitleCommand()
{
}


bool SetSheetTitleCommand::Do()
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetName(mDescription.second);
	mShow.Modify(true);
	return true;
}

bool SetSheetTitleCommand::Undo()
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetName(mDescription.first);
	mShow.Modify(true);
	return true;
}

SetSheetBeatsCommand::SetSheetBeatsCommand(CC_show& show, unsigned short beats)
: wxCommand(true, wxT("setting beats")),
mShow(show), mSheetNum(show.GetCurrentSheetNum())
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	mBeats = std::pair<unsigned short,unsigned short>(sheet->GetBeats(), beats);
}

SetSheetBeatsCommand::~SetSheetBeatsCommand()
{
}


bool SetSheetBeatsCommand::Do()
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetBeats(mBeats.second);
	mShow.Modify(true);
	return true;
}

bool SetSheetBeatsCommand::Undo()
{
	CC_sheet *sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetBeats(mBeats.first);
	mShow.Modify(true);
	return true;
}

SetDescriptionCommand::SetDescriptionCommand(CC_show& show, const wxString& newdescr)
: wxCommand(true, wxT("Moving points")),
mShow(show)
{
	mDescription = std::pair<wxString,wxString>(show.GetDescr(), newdescr);
}

SetDescriptionCommand::~SetDescriptionCommand()
{
}


bool SetDescriptionCommand::Do()
{
	mShow.SetDescr(mDescription.second);
	mShow.Modify(true);
	return true;
}

bool SetDescriptionCommand::Undo()
{
	mShow.SetDescr(mDescription.first);
	mShow.Modify(true);
	return true;
}

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
		show->Modify(undo->was_modified);
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
		show->Modify(true);
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
	undo->was_modified = show->IsModified();
	show->Modify(true);					  // Show is now modified
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
