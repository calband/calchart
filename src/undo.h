/* undo.h
 * Definitions for the undo classes
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

#ifndef _UNDO_H_
#define _UNDO_H_

#ifdef __GNUG__
#pragma interface
#endif

#include "show.h"
#include "cc_continuity.h"
#include "cc_coord.h"
#include "cc_show.h"
#include <ostream>
#include <deque>
#include <map>
#include <wx/cmdproc.h>

class ShowUndo
{
public:
	ShowUndo(unsigned sheetnum): next(NULL), sheetidx(sheetnum) {}
	virtual ~ShowUndo();

// returns the sheet to go to
	virtual int Undo(CC_show *show, ShowUndo** newundo) = 0;
// returns amount of memory used
	virtual unsigned Size() = 0;
// returns description of this event
	virtual const wxChar *UndoDescription() = 0;
	virtual const wxChar *RedoDescription() = 0;

	ShowUndo *next;
	unsigned sheetidx;
	bool was_modified;
};

// Multiple undos in one entry
class ShowUndoMany : public ShowUndo
{
public:
	ShowUndoMany(ShowUndo *undolist);
	virtual ~ShowUndoMany();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	ShowUndo *list;
};

// this command is to move points around on a sheet
// position is a mapping of which point to two positions, the original and new.
// When you do/undo the command, the show will be set to the sheet, and the selection
// list will revert to that selection list.
class MovePointsOnSheetCommand : public wxCommand
{
public:
	MovePointsOnSheetCommand(CC_show& show, unsigned ref);
	virtual ~MovePointsOnSheetCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	std::map<unsigned, std::pair<CC_coord,CC_coord> > mPositions;
	const CC_show::SelectionList mPoints;
	const unsigned mRef;
};

class TranslatePointsByDeltaCommand : public MovePointsOnSheetCommand
{
public:
	TranslatePointsByDeltaCommand(CC_show& show, const CC_coord& delta, unsigned ref);
	virtual ~TranslatePointsByDeltaCommand();
};

class TransformPointsCommand : public MovePointsOnSheetCommand
{
public:
	TransformPointsCommand(CC_show& show, const Matrix& transmat, unsigned ref);
	virtual ~TransformPointsCommand();
};

class TransformPointsInALineCommand : public MovePointsOnSheetCommand
{
public:
	TransformPointsInALineCommand(CC_show& show, const CC_coord& start, const CC_coord& second, unsigned ref);
	virtual ~TransformPointsInALineCommand();
};

// this command is to move points around on a sheet
// position is a mapping of which point to two positions, the original and new.
// When you do/undo the command, the show will be set to the sheet, and the selection
// list will revert to that selection list.
class SetContinuityCommand : public wxCommand
{
public:
	SetContinuityCommand(CC_show& show, unsigned i);
	virtual ~SetContinuityCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	const CC_show::SelectionList mPoints;
	std::map<unsigned, std::pair<unsigned,unsigned> > mContinuity;
};

// Point symbol changes
struct ShowUndoSymElem
{
	unsigned idx;
	SYMBOL_TYPE sym;
	unsigned char cont;
};
class ShowUndoSym : public ShowUndo
{
public:
	ShowUndoSym(unsigned sheetnum, CC_sheet *sheet, bool contchanged = false);
	ShowUndoSym(ShowUndoSym* old, CC_sheet *sheet);
	virtual ~ShowUndoSym();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	unsigned num;
	std::deque<ShowUndoSymElem> elems;
	bool contchange;
};

// Point label changes
struct ShowUndoLblElem
{
	unsigned idx;
	bool right;
};
class ShowUndoLbl : public ShowUndo
{
public:
	ShowUndoLbl(unsigned sheetnum, CC_sheet *sheet);
	ShowUndoLbl(ShowUndoLbl* old, CC_sheet *sheet);
	virtual ~ShowUndoLbl();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	unsigned num;
	std::deque<ShowUndoLblElem> elems;
};

// Copied stuntsheet
class ShowUndoCopy : public ShowUndo
{
public:
	ShowUndoCopy(unsigned sheetnum);
	virtual ~ShowUndoCopy();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
};

// Deleted stuntsheet
class ShowUndoDelete : public ShowUndo
{
public:
	ShowUndoDelete(unsigned sheetnum, CC_sheet *sheet);
	virtual ~ShowUndoDelete();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	CC_sheet *deleted_sheet;
};

// Appended sheets
class ShowUndoAppendSheets : public ShowUndo
{
public:
	ShowUndoAppendSheets(unsigned sheetnum);
	virtual ~ShowUndoAppendSheets();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
};

// Only for redoing append sheets
class ShowUndoDeleteAppendSheets : public ShowUndo
{
public:
	ShowUndoDeleteAppendSheets(CC_sheet *sheet);
	virtual ~ShowUndoDeleteAppendSheets();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	CC_sheet *deleted_sheets;
};

// Stuntsheet name changes
class ShowUndoName : public ShowUndo
{
public:
	ShowUndoName(unsigned sheetnum, CC_sheet *sheet);
	virtual ~ShowUndoName();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	wxString name;
};

// Stuntsheet # of beat changes
class ShowUndoBeat : public ShowUndo
{
public:
	ShowUndoBeat(unsigned sheetnum, CC_sheet *sheet);
	virtual ~ShowUndoBeat();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	unsigned short beats;
};

// Stuntsheet animation continuity changes
class ShowUndoCont : public ShowUndo
{
public:
	ShowUndoCont(unsigned sheetnum, unsigned contnum, CC_sheet *sheet);
	virtual ~ShowUndoCont();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	unsigned cont;
	wxString conttext;
};

// Added continuity
class ShowUndoAddContinuity : public ShowUndo
{
public:
	ShowUndoAddContinuity(unsigned sheetnum, unsigned contnum);
	virtual ~ShowUndoAddContinuity();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	unsigned addcontnum;
};

// Deleted continuity
class ShowUndoDeleteContinuity : public ShowUndo
{
public:
	ShowUndoDeleteContinuity(unsigned sheetnum, unsigned contnum,
		CC_continuity_ptr cont);
	virtual ~ShowUndoDeleteContinuity();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	CC_continuity_ptr deleted_cont;
	unsigned delcontnum;
};

// Show description changes
class ShowUndoDescr : public ShowUndo
{
public:
	ShowUndoDescr(CC_show *show);
	virtual ~ShowUndoDescr();

	virtual int Undo(CC_show *show, ShowUndo** newundo);
	virtual unsigned Size();
	virtual const wxChar *UndoDescription();
	virtual const wxChar *RedoDescription();
private:
	wxString descrtext;
};

class ShowUndoList
{
public:
	ShowUndoList(CC_show *shw, int lim = -1);
	~ShowUndoList();

	int Undo(CC_show *show);
	int Redo(CC_show *show);
	void Add(ShowUndo *undo);

// For doing multiple actions in one entry
	void StartMulti();
	void EndMulti();

	const wxChar *UndoDescription();
	const wxChar *RedoDescription();
	inline void Limit(int val) { limit=val; Clean(); }
	void EraseAll();

private:
	ShowUndo *Pop();
	void Push(ShowUndo *undo);
	void Clean();

	ShowUndo *PopRedo();
	void PushRedo(ShowUndo *undo);
	void EraseAllRedo();

	friend std::ostream& operator<< (std::ostream&, const ShowUndoList&);

	CC_show *show;
	ShowUndo *list;
	ShowUndo *redolist;
	ShowUndo *oldlist;						  // For adding multiple entries
	int limit;
};

std::ostream& operator<< (std::ostream&, const ShowUndoList&);
#endif
