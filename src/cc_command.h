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

/**
 * A command that modifies the CalChart show. 
 * When you modify points or activate a menu option that applies some change to
 * the show (for example, the menu option that changes the number of beats
 * associated with a stunt sheet), a BasicCalChartCommand is created and
 * processed by the CalChartDoc. The big advantage of applying changes to
 * the show in this fashion is that you can remember each action that was
 * applied, and thus undo them. These commands, therefore, have methods
 * to both Do() and Undo() themselves. The BasicCalChartCommand automatically
 * handles the Undo() and Do() methods, granted that each command class derived
 * from BasicCalChartCommand can apply the changes for which they are
 * responsible when the DoAction() method is called on them.
 */
class BasicCalChartCommand : public wxCommand
{
public:
	/**
	 * Creates a command that operates on the given CalChartDoc.
	 * @param doc The CalChartDoc to modify through this command.
	 * @param cmdName The name of the command.
	 */
	BasicCalChartCommand(CalChartDoc& doc, const wxString cmdName);
	
	/** 
	 * Performs cleanup. This is abstract to force the class to be
	 * abstract.
	 */
	virtual ~BasicCalChartCommand() = 0;

	/**
	 * Applies the command.
	 * @return True if the operation was successful; false otherwise.
	 */
	virtual bool Do();

	/**
	 * Undoes the changes made by the command.
	 * @return True if the operation was successful; false otherwise.
	 */
	virtual bool Undo();

protected:

	/** 
	 * The CalChart document that is modified by the command.
	 */
	CalChartDoc& mDoc;

private:

	/**
	 * Modifies the show.
	 */
	virtual void DoAction() = 0;

	/**
	 * Records whether or not the show had been modified since the
	 * last save before changes were applied by the command.
	 * TODO what happens when show is saved and then commands are undone to before save?
	 */
	bool mDocModified;

	/**
	 * A snapshot of what the show looked like before it was changed
	 * through this command. When the command is undone, the show is
	 * restored to the saved snapshot.
	 */
	CC_show mSnapShot;
};

///// Global show commands.  Changes settings to a whole show.

/** 
 * A command that sets the description of the show.
 */
class SetDescriptionCommand : public BasicCalChartCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The show to modify.
	 * @param newdecr The new desciption for the show.
	 */
	SetDescriptionCommand(CalChartDoc& show, const wxString& newdescr);

	/**
	 * Cleanup.
	 */
	virtual ~SetDescriptionCommand();

protected:

	virtual void DoAction();

	/**
	 * The new description to apply to the show.
	 */
	wxString mDescription;
};


/**
 * A command that sets the show mode.
 */
class SetModeCommand : public BasicCalChartCommand
{
public:

	/** 
	 * Make the command.
	 * @param show The show to modify.
	 * @param newmode The new show mode to apply to the show.
	 */
	SetModeCommand(CalChartDoc& show, const wxString& newmode);

	/**
	* Cleanup.
	*/
	virtual ~SetModeCommand();

protected:
	virtual void DoAction();

	/**
	 * The new show mode to apply to the show.
	 */
	wxString mMode;
};


/**
 * A command that sets the number of points in the show and their labels.
 */
class SetShowInfoCommand : public BasicCalChartCommand
{
public:
	
	/**
	 * Make the command.
	 * @param show The show to modify.
	 * @param numPoints The new number of points existing in the show.
	 * @param numColumns TODO
	 * @param labels The labels for all of the points.
	 */
	SetShowInfoCommand(CalChartDoc& show, unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels);

	/**
	* Cleanup.
	*/
	virtual ~SetShowInfoCommand();

protected:

	virtual void DoAction();

	/**
	 * The number of points that the show will have after the command is applied.
	 */
	unsigned mNumPoints;

	/**
	 * TODO
	 */
	unsigned mNumColumns;

	/**
	 * The new set of labels for the points.
	 */
	std::vector<wxString> mLabels;
};


///// Sheet commands.  Changes settings on a sheet

/**
 * A command that applies a change that is specific to a particular
 * stunt sheet. These commands record the sheet that is active when they
 * are created, thus effectively memorizing which stunt sheet the user
 * intends for them to modify.
 */
class SetSheetCommand : public BasicCalChartCommand
{
public:

	/**
	 * Make the command.
	 * @param show The show to modify with the command.
	 * @param cmdName The name of the command.
	 */
	SetSheetCommand(CalChartDoc& show, const wxString cmdName);

	/**
	* Cleanup. This is abstract to force the class to be abstract.
	*/
	virtual ~SetSheetCommand() = 0;

protected:

	virtual void DoAction();

	/** 
	 * The index of the sheet that will be modified by the command.
	 */
	const unsigned mSheetNum;
};


/**
 * A command that sets the title of a stunt sheet.
 */
class SetSheetTitleCommand : public SetSheetCommand
{
public:

	/**
	 * Make the command.
	 * @param show The show to modify with this command.
	 * @param newname The new name for the stunt sheet.
	 */
	SetSheetTitleCommand(CalChartDoc& show, const wxString& newname);

	/**
	* Cleanup.
	*/
	virtual ~SetSheetTitleCommand();

protected:

	virtual void DoAction();

	/**
	 * The new description for the sheet.
	 */
	wxString mDescription;
};


/**
 * A command that sets the duration of a stunt sheet, in beats.
 */
class SetSheetBeatsCommand : public SetSheetCommand
{
public:

	/** 
	 * Make the command.
	 * @param show The show to apply the command to.
	 * @param beats The new duration of the sheet after the command
	 * is applied.
	 */
	SetSheetBeatsCommand(CalChartDoc& show, unsigned short beats);


	/**
	* Cleanup.
	*/
	virtual ~SetSheetBeatsCommand();

protected:

	virtual void DoAction();

	/**
	 * The new duration of the target sheet, after the command is
	 * applied.
	 */
	unsigned short mBeats;
};


/**
 * A command that adds stunt sheets to the show at a particular location.
 */
class AddSheetsCommand : public SetSheetCommand
{
public:

	/**
	 * Make the command.
	 * @param show The show to modify.
	 * @param sheets The sheets to add to the show.
	 * @param where TODO ?
	 */
	AddSheetsCommand(CalChartDoc& show, const CC_show::CC_sheet_container_t& sheets, unsigned where);


	/**
	* Cleanup.
	*/
	virtual ~AddSheetsCommand();

protected:

	virtual void DoAction();
	CC_show::CC_sheet_container_t mSheets;
	const unsigned mWhere;
};

/**
 * A command that removes a stunt sheet from the show.
 */
class RemoveSheetsCommand : public SetSheetCommand
{
public:
	RemoveSheetsCommand(CalChartDoc& show, unsigned where);


	/**
	* Cleanup.
	*/
	virtual ~RemoveSheetsCommand();

protected:
	virtual void DoAction();
	const unsigned mWhere;
};


/**
 * A command that applies changes to the currently selected set of points on the
 * currently active stuntsheet.
 */
class SetSheetAndSelectCommand : public SetSheetCommand
{
public:
	SetSheetAndSelectCommand(CalChartDoc& show, const wxString cmdName);

	/**
	* Cleanup. This is abstract to force the class to be abstract.
	*/
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
	
	/**
	* Cleanup. This is abstract to force the class to be abstract.
	*/
	virtual ~MovePointsOnSheetCommand() = 0;

protected:
	virtual void DoAction();

	std::map<unsigned, CC_coord> mPositions;
	const unsigned mRef;
};


// TranslatePointsByDeltaCommand:
// Move the selected points by a fixed delta
class TranslatePointsByDeltaCommand : public MovePointsOnSheetCommand
{
public:
	TranslatePointsByDeltaCommand(CalChartDoc& show, const CC_coord& delta, unsigned ref);

	/**
	* Cleanup.
	*/
	virtual ~TranslatePointsByDeltaCommand();
};


// TransformPointsCommand:
// Move the selected points by a matrix function
class TransformPointsCommand : public MovePointsOnSheetCommand
{
public:
	TransformPointsCommand(CalChartDoc& show, const Matrix& transmat, unsigned ref);

	/**
	* Cleanup.
	*/
	virtual ~TransformPointsCommand();
};


// TransformPointsInALineCommand:
// Move the selected points by a line function
class TransformPointsInALineCommand : public MovePointsOnSheetCommand
{
public:
	TransformPointsInALineCommand(CalChartDoc& show, const CC_coord& start, const CC_coord& second, unsigned ref);

	/**
	* Cleanup.
	*/
	virtual ~TransformPointsInALineCommand();
};


// SetReferencePointToRef0 :
// Reset a reference point position to ref point 0.
class SetReferencePointToRef0 : public MovePointsOnSheetCommand
{
public:
	SetReferencePointToRef0(CalChartDoc& show, unsigned ref);

	/**
	* Cleanup.
	*/
	virtual ~SetReferencePointToRef0();
};


// SetContinuityIndexCommand:
// Sets the symbol for the selected points, creating one if it doesn't exist
class SetSymbolCommand : public SetSheetAndSelectCommand
{
public:
	SetSymbolCommand(CalChartDoc& show, SYMBOL_TYPE sym);

	/**
	* Cleanup.
	*/
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

	/**
	* Cleanup.
	*/
	virtual ~SetContinuityTextCommand();

protected:
	virtual void DoAction();
	SYMBOL_TYPE mWhichCont;
	wxString mContinuity;
};


/** 
 * A command that changes the positions of the point labels for the currently
 * selected set of points.
 */
class SetLabelCommand : public SetSheetAndSelectCommand
{
public:
	SetLabelCommand(CalChartDoc& show);
	
	/**
	* Cleanup. This is abstract to force the class to be abstract.
	*/
	virtual ~SetLabelCommand() = 0;

protected:
	virtual void DoAction();

	std::map<unsigned, bool> mLabelPos;
};


/**
 * A command that positions the point labels of the currently selected set
 * of points to the right of their dots.
 */
class SetLabelRightCommand : public SetLabelCommand
{
public:
	SetLabelRightCommand(CalChartDoc& show, bool right);

	/**
	* Cleanup.
	*/
	virtual ~SetLabelRightCommand();
};


/**
* A command that positions the point labels of the currently selected set
* of points to the left of their dots.
*/
class SetLabelFlipCommand : public SetLabelCommand
{
public:
	SetLabelFlipCommand(CalChartDoc& show);

	/**
	* Cleanup.
	*/
	virtual ~SetLabelFlipCommand();
};

#endif
