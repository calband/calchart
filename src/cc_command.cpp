/*
 * cc_command.cpp
 * Handle commands from view to doc
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

#include "cc_command.h"
#include "linmath.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include <wx/utils.h>


// BasicCalChartCommand
// Holds the show reference, and handles setting the modify-ness of the show.
BasicCalChartCommand::BasicCalChartCommand(CalChartDoc& doc, const wxString cmdName)
: wxCommand(true, cmdName),
mDoc(doc),
mDocModified(doc.IsModified()),
mSnapShot(doc.ShowSnapShot())
{}

BasicCalChartCommand::~BasicCalChartCommand()
{}

// applying a command modifies the show
bool BasicCalChartCommand::Do()
{
	mSnapShot = mDoc.ShowSnapShot();
	DoAction();
	mDoc.Modify(true);
	return true;
}

// undoing a command puts the state back to original
bool BasicCalChartCommand::Undo()
{
	mDoc.RestoreSnapShot(mSnapShot);
	mDoc.Modify(mDocModified);
	return true;
}


// SetDescriptionCommand
// Set the description of this show
SetDescriptionCommand::SetDescriptionCommand(CalChartDoc& show, const wxString& newdescr)
: BasicCalChartCommand(show, wxT("Set Description")),
mDescription(newdescr)
{}

SetDescriptionCommand::~SetDescriptionCommand()
{}

void SetDescriptionCommand::DoAction()
{
	mDoc.SetDescr(mDescription.ToStdString());
}


// SetModeCommand
// Set the show mode
SetModeCommand::SetModeCommand(CalChartDoc& show, const wxString& newmode)
: BasicCalChartCommand(show, wxT("Set Mode")),
mMode(newmode)
{}

SetModeCommand::~SetModeCommand()
{}

void SetModeCommand::DoAction()
{
	auto newmode = wxGetApp().GetMode(mMode);
	if (newmode)
	{
		mDoc.SetMode(std::move(newmode));
	}
}


// SetShowInfoCommand
// Sets the shows number of points and labels 
SetShowInfoCommand::SetShowInfoCommand(CalChartDoc& show, unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels)
: BasicCalChartCommand(show, wxT("Set show info")),
mNumPoints(numPoints), mNumColumns(numColumns),
mLabels(labels)
{}

SetShowInfoCommand::~SetShowInfoCommand()
{}

void SetShowInfoCommand::DoAction()
{
	mDoc.SetNumPoints(mNumPoints, mNumColumns);
	mDoc.SetPointLabel(std::vector<std::string>(mLabels.begin(), mLabels.end()));
}


// SetSheetCommand:
// Base class for other commands.  Will set sheet to the state they were
// when command was created.
SetSheetCommand::SetSheetCommand(CalChartDoc& doc, const wxString cmdName)
: BasicCalChartCommand(doc, cmdName),
mSheetNum(doc.GetCurrentSheetNum())
{}

SetSheetCommand::~SetSheetCommand()
{}

void SetSheetCommand::DoAction()
{
	mDoc.SetCurrentSheet(mSheetNum);
}


// SetSheetTitleCommand
// Set the description of the current sheet
SetSheetTitleCommand::SetSheetTitleCommand(CalChartDoc& show, const wxString& newname)
: SetSheetCommand(show, wxT("Set title")),
mDescription(newname)
{}

SetSheetTitleCommand::~SetSheetTitleCommand()
{}

void SetSheetTitleCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.GetCurrentSheet()->SetName(std::string(mDescription));
}


// SetSheetBeatsCommand
// Set the beats of the current sheet
SetSheetBeatsCommand::SetSheetBeatsCommand(CalChartDoc& show, unsigned short beats)
: SetSheetCommand(show, wxT("Set beats")),
mBeats(beats)
{}

SetSheetBeatsCommand::~SetSheetBeatsCommand()
{}

void SetSheetBeatsCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.GetCurrentSheet()->SetBeats(mBeats);
}


// AddSheetsCommand
// For adding a container of sheets
AddSheetsCommand::AddSheetsCommand(CalChartDoc& show, const CC_show::CC_sheet_container_t& sheets, unsigned where)
: SetSheetCommand(show, wxT("Adding Sheets")),
mSheets(sheets), mWhere(where)
{}

AddSheetsCommand::~AddSheetsCommand()
{}

void AddSheetsCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.InsertSheetInternal(mSheets, mWhere);
}

// AddSheetsOtherShowCommand
// For adding a container of sheets
AddSheetsOtherShowCommand::AddSheetsOtherShowCommand(CalChartDoc& show, const CC_show::CC_sheet_container_t& sheets, unsigned where, unsigned endpoint)
: SetSheetCommand(show, wxT("Add From Other Show")),
mSheets(sheets), mWhere(where), mEndpoint(endpoint)
{}

AddSheetsOtherShowCommand::~AddSheetsOtherShowCommand()
{}

void AddSheetsOtherShowCommand::DoAction()
{
	SetSheetCommand::DoAction();
	mDoc.InsertSheetInternal(mSheets, mWhere);
}

// RemoveSheetsCommand
// For removing a sheets
RemoveSheetsCommand::RemoveSheetsCommand(CalChartDoc& show, unsigned where)
: SetSheetCommand(show, wxT("Remove Sheets")),
mWhere(where)
{}

RemoveSheetsCommand::~RemoveSheetsCommand()
{}

void RemoveSheetsCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.RemoveNthSheet(mWhere);
}


// SetSheetAndSelectCommand:
// Base class for other commands.  Will set selection and page to the state they were
// when command was created.
SetSheetAndSelectCommand::SetSheetAndSelectCommand(CalChartDoc& doc, const wxString cmdName)
: SetSheetCommand(doc, cmdName),
mPoints(doc.GetSelectionList())
{}

SetSheetAndSelectCommand::~SetSheetAndSelectCommand()
{}

void SetSheetAndSelectCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.SetSelection(mPoints);
}


ImportPrintContinuityCommand::ImportPrintContinuityCommand(CalChartDoc& show, const std::vector<std::string>& print_cont)
: SetSheetCommand(show, wxT("Importing Printable continuity command")),
mPrintCont(print_cont)
{}

ImportPrintContinuityCommand::~ImportPrintContinuityCommand()
{}

void ImportPrintContinuityCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.ImportPrintableContinuity(mPrintCont);
}


SetPrintContinuityCommand::SetPrintContinuityCommand(CalChartDoc& show, unsigned sheet, const std::string& number, const std::string& print_cont)
: SetSheetCommand(show, wxT("Setting Printable continuity command")),
mWhichSheet(sheet),
mNumber(number),
mPrintCont(print_cont)
{}

SetPrintContinuityCommand::~SetPrintContinuityCommand()
{}

void SetPrintContinuityCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mDoc.GetNthSheet(mWhichSheet)->SetPrintableContinuity(mNumber, mPrintCont);
}


// MovePointsOnSheetCommand:
// Move points around on a sheet.  mPosition is a mapping of which point to two positions,
// the original and new.  Do will move points to the new position, and undo will move them back.
// Fill out the mPosition in the constructor.
MovePointsOnSheetCommand::MovePointsOnSheetCommand(CalChartDoc& show, unsigned ref)
: SetSheetAndSelectCommand(show, wxT("Moving points")),
mRef(ref)
{}

MovePointsOnSheetCommand::~MovePointsOnSheetCommand()
{}

void MovePointsOnSheetCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	for (auto i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(mDoc.GetMode().ClipPosition(i->second), i->first, mRef);
	}
}

// RotatePointPositionsCommand
RotatePointPositionsCommand::RotatePointPositionsCommand(CalChartDoc& show, unsigned rotateAmount, unsigned ref)
: super(show, ref)
{
	// construct a vector of point indices in order
	std::vector<unsigned> pointIndices;
	std::copy(mPoints.begin(), mPoints.end(), std::back_inserter(pointIndices));

	// construct a vector of point positions, rotated by rotate amount
	std::vector<CC_coord> finalPositions;
	CC_show::const_CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	std::transform(mPoints.begin(), mPoints.end(), std::back_inserter(finalPositions), [=](unsigned i) { return sheet->GetPosition(i, mRef); });
	rotateAmount %= mPoints.size();
	std::rotate(finalPositions.begin(), finalPositions.begin() + rotateAmount, finalPositions.end());

	// put things into place.
	for (int index = pointIndices.size() - 1; index >= 0; index--) {
		mPositions[pointIndices[index]] = finalPositions[index];
	}
}

RotatePointPositionsCommand::~RotatePointPositionsCommand()
{}


// MovePointsCommand:
// Move points to position
MovePointsCommand::MovePointsCommand(CalChartDoc& show, const std::map<unsigned, CC_coord>& newPosition, unsigned ref)
: super(show, ref)
{
	mPositions = newPosition;
}

MovePointsCommand::~MovePointsCommand()
{}


// SetReferencePointToRef0 :
// Reset a reference point position to ref point 0.
SetReferencePointToRef0::SetReferencePointToRef0(CalChartDoc& show, unsigned ref) :
MovePointsOnSheetCommand(show, ref)
{
	CC_show::const_CC_sheet_iterator_t sheet = mDoc.GetNthSheet(mSheetNum);
	// for all the points, set the reference point
	const unsigned short numPoints = mDoc.GetNumPoints();
	for (unsigned short i = 0; i != numPoints; ++i)
	{
		mPositions[i] = sheet->GetPosition(i, 0);
	}
}

SetReferencePointToRef0::~SetReferencePointToRef0()
{}


// SetSymbolCommand:
// Sets the symbol for the selected points, creating one if it doesn't exist
SetSymbolCommand::SetSymbolCommand(CalChartDoc& show, SYMBOL_TYPE sym)
: SetSheetAndSelectCommand(show, wxT("Setting Continuity Symbol"))
{
	CC_show::const_CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	for (auto i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		// Only do work on points that have different symbols
		if (sym != sheet->GetPoint(*i).GetSymbol())
		{
			mSyms[*i] = sym;
		}
	}
}

SetSymbolCommand::~SetSymbolCommand()
{}

void SetSymbolCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
// possible optimization:  Don't save as map, since they all get moved to new symbol...
	for (auto i = mSyms.begin(); i != mSyms.end(); ++i)
	{
		sheet->GetPoint(i->first).SetSymbol(i->second);
	}
}


// SetContinuityTextCommand
// Sets the continuity text
SetContinuityTextCommand::SetContinuityTextCommand(CalChartDoc& show, SYMBOL_TYPE which, const wxString& text)
: SetSheetAndSelectCommand(show, wxT("Setting Continuity Text")),
mWhichCont(which),
mContinuity(text)
{}

SetContinuityTextCommand::~SetContinuityTextCommand()
{}

void SetContinuityTextCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	mDoc.GetCurrentSheet()->SetContinuityText(mWhichCont, mContinuity.ToStdString());
}


SetLabelCommand::SetLabelCommand(CalChartDoc& show)
: SetSheetAndSelectCommand(show, wxT("Setting Labels"))
{}

SetLabelCommand::~SetLabelCommand()
{}

void SetLabelCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	for (auto i = mLabelPos.begin(); i != mLabelPos.end(); ++i)
	{
		sheet->GetPoint(i->first).Flip(i->second);
	}
}


SetLabelRightCommand::SetLabelRightCommand(CalChartDoc& show, bool right)
: SetLabelCommand(show)
{
	for (auto i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = right;
	}
}

SetLabelRightCommand::~SetLabelRightCommand()
{}


SetLabelFlipCommand::SetLabelFlipCommand(CalChartDoc& show)
: SetLabelCommand(show)
{
	CC_show::const_CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	for (auto i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = !sheet->GetPoint(*i).GetFlip();
	}
}

SetLabelFlipCommand::~SetLabelFlipCommand()
{}


SetLabelVisibilityCommand::SetLabelVisibilityCommand(CalChartDoc& show) 
: super(show, wxT("Setting Label Visibility"))
{}

SetLabelVisibilityCommand::~SetLabelVisibilityCommand()
{}

void SetLabelVisibilityCommand::DoAction() {
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	for (auto iterator = mLabelVisibility.begin(); iterator != mLabelVisibility.end(); iterator++) {
		sheet->GetPoint(iterator->first).SetLabelVisibility(iterator->second);
	}
}

SetLabelVisibleCommand::SetLabelVisibleCommand(CalChartDoc& show, bool isVisible)
: super(show)
{
	for (auto iterator = mPoints.begin(); iterator != mPoints.end(); iterator++) {
		mLabelVisibility[*iterator] = isVisible;
	}
}

SetLabelVisibleCommand::~SetLabelVisibleCommand()
{}

ToggleLabelVisibilityCommand::ToggleLabelVisibilityCommand(CalChartDoc& show)
: super(show)
{
	CC_show::const_CC_sheet_iterator_t sheet = mDoc.GetCurrentSheet();
	for (auto iterator = mPoints.begin(); iterator != mPoints.end(); iterator++) {
		mLabelVisibility[*iterator] = !sheet->GetPoint(*iterator).LabelIsVisible();
	}
}

ToggleLabelVisibilityCommand::~ToggleLabelVisibilityCommand()
{}


OverwriteMusicScoreCommand::OverwriteMusicScoreCommand(CalChartDoc& show, MusicScoreDocComponent& newScore) 
: super(show, "Change Music Score"), mNewScore(newScore)
{}

void OverwriteMusicScoreCommand::DoAction() {
	mDoc.getMusicScore()->copyContentFrom(mNewScore);
}
