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
	 * The number of columns in the initial configuration of points that the show
	 * wll have after the command is applied.
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
	/**
	 * The sheets to insert into the show.
	 */
	CC_show::CC_sheet_container_t mSheets;
	/**
	 * The index that the first added sheet will have after the command is applied.
	 */
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
	/**
	 * The index of the stuntsheet to remove from the show.
	 */
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
	/**
	 * The list of selected points, to which the command will be applied.
	 */
	const SelectionList mPoints;
};


/**
 * A command that moves the currently selected  points to new positions on the
 * stunt sheet when applied.
 */
class MovePointsOnSheetCommand : public SetSheetAndSelectCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The show to edit.
	 * @param ref The index of the reference point to edit for each point
	 * through this command.
	 */
	MovePointsOnSheetCommand(CalChartDoc& show, unsigned ref);
	
	/**
	 * Cleanup. This is abstract to force the class to be abstract.
	 */
	virtual ~MovePointsOnSheetCommand() = 0;

protected:
	virtual void DoAction();

	/**
	 * A mapping of from a point's index to the new position for that point
	 * after the command is applied.
	 */
	std::map<unsigned, CC_coord> mPositions;
	/**
	 * The index of the reference point for the dots which will be altered
	 * through this command.
	 */
	const unsigned mRef;
};


/**
 * Moves all currently selected points by some change (delta).
 */
class TranslatePointsByDeltaCommand : public MovePointsOnSheetCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The show to modify through this command.
	 * @param delta The amount by which the positions of the selecte points should change.
	 * @param ref The index of the reference point to move for each selected point.
	 */
	TranslatePointsByDeltaCommand(CalChartDoc& show, const CC_coord& delta, unsigned ref);

	/**
	 * Cleanup.
	 */
	virtual ~TranslatePointsByDeltaCommand();
};


/**
 * Transforms the selected points by a transformation matrix when the command
 * is applied. The points are multiplied by the transformation matrix as if
 * they were vectors.
 */
class TransformPointsCommand : public MovePointsOnSheetCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through this command.
	 * @param transmat The transformation matrix to apply to the selected points.
	 * @param ref The reference point to transform with the matrix.
	 */
	TransformPointsCommand(CalChartDoc& show, const Matrix& transmat, unsigned ref);

	/**
	 * Cleanup.
	 */
	virtual ~TransformPointsCommand();
};


/**
 * Moves the selected points into a line when the command is applied.
 */
class TransformPointsInALineCommand : public MovePointsOnSheetCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through this command.
	 * @param start The starting point for the line.
	 * @param second The position of the second point in the line.
	 * @param ref The reference point to modify through this command.
	 */
	TransformPointsInALineCommand(CalChartDoc& show, const CC_coord& start, const CC_coord& second, unsigned ref);

	/**
	 * Cleanup.
	 */
	virtual ~TransformPointsInALineCommand();
};


/**
 * Sets the position of a particular reference point for each of the selected
 * points to the actual position of the point itself (the position of its
 * zeroth reference point) when applied.
 */
class SetReferencePointToRef0 : public MovePointsOnSheetCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through the command.
	 * @param ref The index of the reference point that should be modified
	 * through this command.
	 */
	SetReferencePointToRef0(CalChartDoc& show, unsigned ref);

	/**
	 * Cleanup.
	 */
	virtual ~SetReferencePointToRef0();
};


/**
 * Sets the symbol of the currently selected points when the command is
 * applied.
 */
class SetSymbolCommand : public SetSheetAndSelectCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through the command.
	 * @param sym The symbol to associate with the points after
	 * the command is applied.
	 */
	SetSymbolCommand(CalChartDoc& show, SYMBOL_TYPE sym);

	/**
	 * Cleanup.
	 */
	virtual ~SetSymbolCommand();

protected:
	virtual void DoAction();
	/**
	 * A map of point index to the symbol to apply to that point
	 * when the command is applied.
	 */
	std::map<unsigned, SYMBOL_TYPE> mSyms;
};


/**
 * Sets the text of a continuity associated with the current stunt sheet
 * (the text of a continuity is the script associated with the continuity
 * as submitted through the Continuity Editor) when applied.
 */
class SetContinuityTextCommand : public SetSheetAndSelectCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through this command.
	 * @param i The symbol type associated with the target continuity.
	 * @param text The new text for the continuity.
	 */
	SetContinuityTextCommand(CalChartDoc& show, SYMBOL_TYPE i, const wxString& text);

	/**
	 * Cleanup.
	 */
	virtual ~SetContinuityTextCommand();

protected:
	virtual void DoAction();
	/**
	 * The symbol type associated with the target continuity.
	 */
	SYMBOL_TYPE mWhichCont;
	/**
	 * The new text for the continuity.
	 */
	wxString mContinuity;
};


/** 
 * A command that changes the positions of the point labels for the currently
 * selected set of points.
 */
class SetLabelCommand : public SetSheetAndSelectCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through this command.
	 */
	SetLabelCommand(CalChartDoc& show);
	
	/**
	 * Cleanup. This is abstract to force the class to be abstract.
	 */
	virtual ~SetLabelCommand() = 0;

protected:
	virtual void DoAction();

	/**
	 * A mapping from the index of a point to the final position of the
	 * label relative to the point (true if the label will be on the
	 * right of the point, false if on the left) after the command
	 * is appied.
	 */
	std::map<unsigned, bool> mLabelPos;
};


/**
 * A command that positions the point labels of the currently selected set
 * of points to the right/left of their dots.
 */
class SetLabelRightCommand : public SetLabelCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The document to modify through this command.
	 * @param right True if the label should be placed to the right
	 * of the dots, false if the label should be placed to the left.
	 */
	SetLabelRightCommand(CalChartDoc& show, bool right);

	/**
	 * Cleanup.
	 */
	virtual ~SetLabelRightCommand();
};


/**
 * A command that toggles the positions the point labels of the currently
 * selected set of points between the right and left of the dots.
 */
class SetLabelFlipCommand : public SetLabelCommand
{
public:
	/**
	 * Makes the command.
	 * @param show The show to modify through this command.
	 */
	SetLabelFlipCommand(CalChartDoc& show);

	/**
	 * Cleanup.
	 */
	virtual ~SetLabelFlipCommand();
};

#endif
