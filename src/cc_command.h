/*
 * cc_command.h
 * Handles interaction from View to Doc
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

#ifndef _UNDO_H_
#define _UNDO_H_

#include "cc_continuity.h"
#include "cc_coord.h"
#include "calchartdoc.h"
#include "cc_show.h"
#include "cc_sheet.h"
#include <ostream>
#include <deque>
#include <map>
#include <wx/cmdproc.h>

// We assume that all commands are issued as soon as they are created.
// This is important because the commands may "snap-shot" the system to know
// how to do the 'undo'.

// We implement our command in the base class, and each of the commands
// needs to implement a "DoAction" and "UndoAction".  This allows the
// base class to control the preamble and postamble of the calls.
 
// BasicCalChartCommand
// The low level command all other commands should inherit from.
// Holds the show reference, and handles setting the modify-ness of the show.
class BasicCalChartCommand : public wxCommand
{
public:
	BasicCalChartCommand(CalChartDoc& doc, const wxString cmdName);
	// make destructor virtual and implement it to force abstract base class
	virtual ~BasicCalChartCommand() = 0;

	virtual bool Do();
	virtual bool Undo();

protected:
	CalChartDoc& mDoc;

private:
	virtual void DoAction() = 0;
	virtual void UndoAction() = 0;
	bool mDocModified;
};

///// Global show commands.  Changes settings to a whole show.

// SetDescriptionCommand
// Set the description of this show
class SetDescriptionCommand : public BasicCalChartCommand
{
public:
	SetDescriptionCommand(CalChartDoc& show, const wxString& newdescr);
	virtual ~SetDescriptionCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::pair<wxString,wxString> mDescription;
};


// SetModeCommand
// Set show mode
class SetModeCommand : public BasicCalChartCommand
{
public:
	SetModeCommand(CalChartDoc& show, const wxString& newdescr);
	virtual ~SetModeCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::pair<wxString,wxString> mMode;
};


// SetShowInfoCommand
// Sets the shows number of points and labels 
class SetShowInfoCommand : public BasicCalChartCommand
{
public:
	SetShowInfoCommand(CalChartDoc& show, unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels);
	virtual ~SetShowInfoCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	unsigned mNumPoints;
	unsigned mNumColumns;
	unsigned mOriginalNumPoints;
	std::pair<std::vector<wxString>,std::vector<wxString> > mLabels;
	std::vector<std::vector<CC_point> > mOriginalPoints;
};


///// Sheet commands.  Changes settings on a sheet

// SetSheetCommand:
// Base class for other commands.  Will set sheet to the page they were
// when command was created.
class SetSheetCommand : public BasicCalChartCommand
{
public:
	SetSheetCommand(CalChartDoc& show, const wxString cmdName);
	// make destructor virtual and implement it to force abstract base class
	virtual ~SetSheetCommand() = 0;

protected:
	virtual void DoAction();
	virtual void UndoAction();

	const unsigned mSheetNum;
};


// SetSheetTitleCommand
// Set the description of the current sheet
class SetSheetTitleCommand : public SetSheetCommand
{
public:
	SetSheetTitleCommand(CalChartDoc& show, const wxString& newname);
	virtual ~SetSheetTitleCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::pair<wxString,wxString> mDescription;
};


// SetSheetBeatsCommand
// Set the beats of the current sheet
class SetSheetBeatsCommand : public SetSheetCommand
{
public:
	SetSheetBeatsCommand(CalChartDoc& show, unsigned short beats);
	virtual ~SetSheetBeatsCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::pair<unsigned short,unsigned short> mBeats;
};


// AddSheetsCommand
// For adding a container of sheets
class AddSheetsCommand : public SetSheetCommand
{
public:
	AddSheetsCommand(CalChartDoc& show, const CC_show::CC_sheet_container_t& sheets, unsigned where);
	virtual ~AddSheetsCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
};

// RemoveSheetsCommand
// For removing a sheets
class RemoveSheetsCommand : public SetSheetCommand
{
public:
	RemoveSheetsCommand(CalChartDoc& show, unsigned where);
	virtual ~RemoveSheetsCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
};


// AddRemoveContinuityCommand
// Base for both adding and removing a continuity
class AddRemoveContinuityCommand : public SetSheetCommand
{
public:
	AddRemoveContinuityCommand(CalChartDoc& show);
	virtual ~AddRemoveContinuityCommand();

protected:
	virtual void UndoAction();

	CC_sheet::ContContainer mOrigAnimcont;
};

// AddContinuityCommand
// Adding a continuity
class AddContinuityCommand : public AddRemoveContinuityCommand
{
public:
	AddContinuityCommand(CalChartDoc& show, const wxString& text);
	virtual ~AddContinuityCommand();

protected:
	virtual void DoAction();

	const wxString mContName;
};

// RemoveContinuityCommand
// Removing a continuity
class RemoveContinuityCommand : public AddRemoveContinuityCommand
{
public:
	RemoveContinuityCommand(CalChartDoc& show, unsigned i);
	virtual ~RemoveContinuityCommand();

protected:
	virtual void DoAction();

	const unsigned mIndexToRemove;
};


///// Sheet and point commands.  Changes for points selected on a sheet
// SetSheetAndSelectCommand:
// Base class for other commands.  Will set selection and page to the state they were
// when command was created.
class SetSheetAndSelectCommand : public SetSheetCommand
{
public:
	SetSheetAndSelectCommand(CalChartDoc& show, const wxString cmdName);
	// make destructor virtual and implement it to force abstract base class
	virtual ~SetSheetAndSelectCommand() = 0;

protected:
	virtual void DoAction();
	virtual void UndoAction();

	const SelectionList mPoints;
};


// MovePointsOnSheetCommand:
// Move points around on a sheet.  mPosition is a mapping of which point to two positions,
// the original and new.  Do will move points to the new position, and undo will move them back.
// Fill out the mPosition in the constructor.
class MovePointsOnSheetCommand : public SetSheetAndSelectCommand
{
public:
	MovePointsOnSheetCommand(CalChartDoc& show, unsigned ref);
	// make destructor virtual and implement it to force abstract base class
	virtual ~MovePointsOnSheetCommand() = 0;

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::map<unsigned, std::pair<CC_coord,CC_coord> > mPositions;
	const unsigned mRef;
};


// TranslatePointsByDeltaCommand:
// Move the selected points by a fixed delta
class TranslatePointsByDeltaCommand : public MovePointsOnSheetCommand
{
public:
	TranslatePointsByDeltaCommand(CalChartDoc& show, const CC_coord& delta, unsigned ref);
	virtual ~TranslatePointsByDeltaCommand();
};


// TransformPointsCommand:
// Move the selected points by a matrix function
class TransformPointsCommand : public MovePointsOnSheetCommand
{
public:
	TransformPointsCommand(CalChartDoc& show, const Matrix& transmat, unsigned ref);
	virtual ~TransformPointsCommand();
};


// TransformPointsInALineCommand:
// Move the selected points by a line function
class TransformPointsInALineCommand : public MovePointsOnSheetCommand
{
public:
	TransformPointsInALineCommand(CalChartDoc& show, const CC_coord& start, const CC_coord& second, unsigned ref);
	virtual ~TransformPointsInALineCommand();
};


// SetReferencePointToRef0 :
// Reset a reference point position to ref point 0.
class SetReferencePointToRef0 : public MovePointsOnSheetCommand
{
public:
	SetReferencePointToRef0(CalChartDoc& show, unsigned ref);
	virtual ~SetReferencePointToRef0();
};


// SetContinuityIndexCommand:
// Sets the continuity index of the selected points.
class SetContinuityIndexCommand : public SetSheetAndSelectCommand
{
public:
	SetContinuityIndexCommand(CalChartDoc& show, unsigned index);
	virtual ~SetContinuityIndexCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::map<unsigned, std::pair<unsigned,unsigned> > mContinuity;
};


// SetContinuityIndexCommand:
// Sets the symbol for the selected points, creating one if it doesn't exist
class SetSymbolAndContCommand : public SetSheetAndSelectCommand
{
public:
	SetSymbolAndContCommand(CalChartDoc& show, SYMBOL_TYPE sym);
	virtual ~SetSymbolAndContCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	CC_sheet::ContContainer mOrigAnimcont;
	typedef std::pair<SYMBOL_TYPE,unsigned> sym_cont_t;
	std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> > mSymsAndCont;
};


// SetContinuityTextCommand
// Sets the continuity text
class SetContinuityTextCommand : public SetSheetAndSelectCommand
{
public:
	SetContinuityTextCommand(CalChartDoc& show, unsigned i, const wxString& text);
	virtual ~SetContinuityTextCommand();

protected:
	virtual void DoAction();
	virtual void UndoAction();

	unsigned mWhichCont;
	std::pair<wxString,wxString> mContinuity;
};


// SetLabelCommand
// Base class for setting the labels.  Set the mLabelPos with values
class SetLabelCommand : public SetSheetAndSelectCommand
{
public:
	SetLabelCommand(CalChartDoc& show);
	// make destructor virtual and implement it to force abstract base class
	virtual ~SetLabelCommand() = 0;

protected:
	virtual void DoAction();
	virtual void UndoAction();

	std::map<unsigned, std::pair<bool,bool> > mLabelPos;
};


// SetLabelRightCommand
// Set label on right
class SetLabelRightCommand : public SetLabelCommand
{
public:
	SetLabelRightCommand(CalChartDoc& show, bool right);
	virtual ~SetLabelRightCommand();
};


// SetLabelFlipCommand
// Set label on left
class SetLabelFlipCommand : public SetLabelCommand
{
public:
	SetLabelFlipCommand(CalChartDoc& show);
	virtual ~SetLabelFlipCommand();
};

#endif
