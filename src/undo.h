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
#include "cc_sheet.h"
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
class SetContinuityIndexCommand : public wxCommand
{
public:
	SetContinuityIndexCommand(CC_show& show, unsigned index);
	virtual ~SetContinuityIndexCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	const CC_show::SelectionList mPoints;
	std::map<unsigned, std::pair<unsigned,unsigned> > mContinuity;
};

// this command is to move points around on a sheet
// position is a mapping of which point to two positions, the original and new.
// When you do/undo the command, the show will be set to the sheet, and the selection
// list will revert to that selection list.
class SetSymbolAndContCommand : public wxCommand
{
public:
	SetSymbolAndContCommand(CC_show& show, SYMBOL_TYPE sym);
	virtual ~SetSymbolAndContCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	const CC_show::SelectionList mPoints;
	CC_sheet::ContContainer mOrigAnimcont;
	typedef std::pair<SYMBOL_TYPE,unsigned> sym_cont_t;
	std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> > mSymsAndCont;
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

// this command is to move points around on a sheet
// position is a mapping of which point to two positions, the original and new.
// When you do/undo the command, the show will be set to the sheet, and the selection
// list will revert to that selection list.
class SetContinuityTextCommand : public wxCommand
{
public:
	SetContinuityTextCommand(CC_show& show, unsigned i, const wxString& text);
	virtual ~SetContinuityTextCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	const CC_show::SelectionList mPoints;
	unsigned mWhichCont;
	std::pair<wxString,wxString> mContinuity;
};

// Added or remove continuity base class
class AddRemoveContinuityCommand : public wxCommand
{
public:
	AddRemoveContinuityCommand(CC_show& show);
	virtual ~AddRemoveContinuityCommand();

	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	CC_sheet::ContContainer mOrigAnimcont;
};

// Added continuity
class AddContinuityCommand : public AddRemoveContinuityCommand
{
public:
	AddContinuityCommand(CC_show& show, const wxString& text);
	virtual ~AddContinuityCommand();

	virtual bool Do();

protected:
	const wxString mContName;
};

// Added continuity
class RemoveContinuityCommand : public AddRemoveContinuityCommand
{
public:
	RemoveContinuityCommand(CC_show& show, unsigned i);
	virtual ~RemoveContinuityCommand();

	virtual bool Do();

protected:
	const unsigned mIndexToRemove;
};

// Show description changes
class SetSheetTitleCommand : public wxCommand
{
public:
	SetSheetTitleCommand(CC_show& show, const wxString& newname);
	virtual ~SetSheetTitleCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	std::pair<wxString,wxString> mDescription;
};

class SetSheetBeatsCommand : public wxCommand
{
public:
	SetSheetBeatsCommand(CC_show& show, unsigned short beats);
	virtual ~SetSheetBeatsCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	std::pair<unsigned short,unsigned short> mBeats;
};

// Show description changes
class SetDescriptionCommand : public wxCommand
{
public:
	SetDescriptionCommand(CC_show& show, const wxString& newdescr);
	virtual ~SetDescriptionCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	std::pair<wxString,wxString> mDescription;
};

class ShowUndoList
{
public:
	ShowUndoList(CC_show *shw, int lim = -1);
	~ShowUndoList();

	int Undo(CC_show *show);
	int Redo(CC_show *show);
	void Add(ShowUndo *undo);

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
