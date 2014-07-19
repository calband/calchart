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
#include <ostream>
#include <deque>
#include <map>
#include <wx/cmdproc.h>

class Matrix;

// We take a snapshot of the show when the command is created and also
// when it's Do() is called.  This is a little bit unnecessary.
// The Undo() command reverts the show to the original snapshot

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
	bool mDocModified;
	CC_show mSnapShot;
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
	wxString mDescription;
};


// SetModeCommand
// Set show mode
class SetModeCommand : public BasicCalChartCommand
{
public:
	SetModeCommand(CalChartDoc& show, const wxString& newmode);
	virtual ~SetModeCommand();

protected:
	virtual void DoAction();
	wxString mMode;
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
	unsigned mNumPoints;
	unsigned mNumColumns;
	std::vector<wxString> mLabels;
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
	wxString mDescription;
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
	unsigned short mBeats;
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
	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
};

// AddSheetsCommand
// For adding a container of sheets
class AddSheetsOtherShowCommand : public SetSheetCommand
{
public:
	AddSheetsOtherShowCommand(CalChartDoc& show, const CC_show::CC_sheet_container_t& sheets, unsigned where, unsigned endpoint);
	virtual ~AddSheetsOtherShowCommand();
    
protected:
	virtual void DoAction();
	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
    const unsigned mEndpoint;
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
	const unsigned mWhere;
};


// SettingPrintContinuityCommand
// Setting the print continuity for a sheet
class ImportPrintContinuityCommand : public SetSheetCommand
{
public:
	ImportPrintContinuityCommand(CalChartDoc& show, const std::vector<std::string>& print_cont);
	virtual ~ImportPrintContinuityCommand();
	
protected:
	virtual void DoAction();
	const std::vector<std::string> mPrintCont;
};

// SettingPrintContinuityCommand
// Setting the print continuity for a sheet
class SetPrintContinuityCommand : public SetSheetCommand
{
public:
	SetPrintContinuityCommand(CalChartDoc& show, unsigned sheet, const std::string& number, const std::string& print_cont);
	virtual ~SetPrintContinuityCommand();
	
protected:
	virtual void DoAction();
	const unsigned mWhichSheet;
	const std::string mNumber;
	const std::string mPrintCont;
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

	std::map<unsigned, CC_coord> mPositions;
	const unsigned mRef;
};


class RotatePointPositionsCommand : public MovePointsOnSheetCommand
{
private:
	using super = MovePointsOnSheetCommand;
public:
	RotatePointPositionsCommand(CalChartDoc& show, unsigned rotateAmount, unsigned ref);
	virtual ~RotatePointPositionsCommand();
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
// Sets the symbol for the selected points, creating one if it doesn't exist
class SetSymbolCommand : public SetSheetAndSelectCommand
{
public:
	SetSymbolCommand(CalChartDoc& show, SYMBOL_TYPE sym);
	virtual ~SetSymbolCommand();

protected:
	virtual void DoAction();
	std::map<unsigned, SYMBOL_TYPE> mSyms;
};


// SetContinuityTextCommand
// Sets the continuity text
class SetContinuityTextCommand : public SetSheetAndSelectCommand
{
public:
	SetContinuityTextCommand(CalChartDoc& show, SYMBOL_TYPE i, const wxString& text);
	virtual ~SetContinuityTextCommand();

protected:
	virtual void DoAction();
	SYMBOL_TYPE mWhichCont;
	wxString mContinuity;
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

	std::map<unsigned, bool> mLabelPos;
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


// SetLabelVisibilityCommand
// Base class for setting label visibility
class SetLabelVisibilityCommand : public SetSheetAndSelectCommand
{
private:
	using super = SetSheetAndSelectCommand;
public:
	SetLabelVisibilityCommand(CalChartDoc& show);
	virtual ~SetLabelVisibilityCommand() = 0;

protected:
	virtual void DoAction();

	std::map<unsigned, bool> mLabelVisibility;
};


// SetLabelVisibleCommand
// Set label to be visible or invisible
class SetLabelVisibleCommand : public SetLabelVisibilityCommand
{
private:
	using super = SetLabelVisibilityCommand;
public:
	SetLabelVisibleCommand(CalChartDoc& show, bool isVisible);
	virtual ~SetLabelVisibleCommand();
};


// ToggleVisibilityCommand
// Toggle whether or not the label is visible
class ToggleLabelVisibilityCommand : public SetLabelVisibilityCommand
{
private:
	using super = SetLabelVisibilityCommand;
public:
	ToggleLabelVisibilityCommand(CalChartDoc& show);
	virtual ~ToggleLabelVisibilityCommand();
};


#endif
