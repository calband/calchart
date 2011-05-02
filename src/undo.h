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

// this command is to move points labels
class SetLabelCommand : public wxCommand
{
public:
	SetLabelCommand(CC_show& show);
	virtual ~SetLabelCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	const unsigned mSheetNum;
	const CC_show::SelectionList mPoints;
	std::map<unsigned, std::pair<bool,bool> > mLabelPos;
};

class SetLabelRightCommand : public SetLabelCommand
{
public:
	SetLabelRightCommand(CC_show& show, bool right);
	virtual ~SetLabelRightCommand();
};

class SetLabelFlipCommand : public SetLabelCommand
{
public:
	SetLabelFlipCommand(CC_show& show);
	virtual ~SetLabelFlipCommand();
};

// For adding a single or container of sheets
class AddSheetsCommand : public wxCommand
{
public:
	AddSheetsCommand(CC_show& show, const CC_show::CC_sheet_container_t& sheets, unsigned where);
	virtual ~AddSheetsCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
};

// For removing a single
class RemoveSheetsCommand : public wxCommand
{
public:
	RemoveSheetsCommand(CC_show& show, unsigned where);
	virtual ~RemoveSheetsCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
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

// Show description changes
class SetModeCommand : public wxCommand
{
public:
	SetModeCommand(CC_show& show, const wxString& newdescr);
	virtual ~SetModeCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	std::pair<wxString,wxString> mMode;
};

// Show info 
class SetShowInfoCommand : public wxCommand
{
public:
	SetShowInfoCommand(CC_show& show, unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels);
	virtual ~SetShowInfoCommand();

	virtual bool Do();
	virtual bool Undo();

protected:
	CC_show& mShow;
	unsigned mNumPoints;
	unsigned mNumColumns;
	unsigned mOriginalNumPoints;
	std::pair<std::vector<wxString>,std::vector<wxString> > mLabels;
	std::vector<std::vector<CC_point> > mOriginalPoints;
};

#endif
