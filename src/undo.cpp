/*
 * undo.cpp
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

#include "undo.h"
#include "show.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include <wx/utils.h>


// BasicCalChartCommand
// Holds the show reference, and handles setting the modify-ness of the show.
BasicCalChartCommand::BasicCalChartCommand(CC_show& show, const wxString cmdName)
: wxCommand(true, cmdName),
mShow(show), mShowModified(show.IsModified())
{}

BasicCalChartCommand::~BasicCalChartCommand()
{}

// applying a command modifies the show
bool BasicCalChartCommand::Do()
{
	DoAction();
	mShow.Modify(true);
	return true;
}

// undoing a command puts the state back to original
bool BasicCalChartCommand::Undo()
{
	UndoAction();
	mShow.Modify(mShowModified);
	return true;
}


// SetDescriptionCommand
// Set the description of this show
SetDescriptionCommand::SetDescriptionCommand(CC_show& show, const wxString& newdescr)
: BasicCalChartCommand(show, wxT("Set Description"))
{
	mDescription = std::pair<wxString,wxString>(show.GetDescr(), newdescr);
}

SetDescriptionCommand::~SetDescriptionCommand()
{}


void SetDescriptionCommand::DoAction()
{
	mShow.SetDescr(mDescription.second);
}

void SetDescriptionCommand::UndoAction()
{
	mShow.SetDescr(mDescription.first);
}


// SetDescriptionCommand
// Set the show mode
SetModeCommand::SetModeCommand(CC_show& show, const wxString& newmode)
: BasicCalChartCommand(show, wxT("Set Mode"))
{
	mMode = std::pair<wxString,wxString>(show.GetMode().GetName(), newmode);
}

SetModeCommand::~SetModeCommand()
{}

void SetModeCommand::DoAction()
{
	ShowMode *newmode = wxGetApp().GetModeList().Find(mMode.second);
	if (newmode)
	{
		mShow.SetMode(newmode);
	}
}

void SetModeCommand::UndoAction()
{
	ShowMode *newmode = wxGetApp().GetModeList().Find(mMode.first);
	if (newmode)
	{
		mShow.SetMode(newmode);
	}
}


// SetShowInfoCommand
// Sets the shows number of points and labels 
SetShowInfoCommand::SetShowInfoCommand(CC_show& show, unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels)
: BasicCalChartCommand(show, wxT("Set show info")),
mNumPoints(numPoints), mNumColumns(numColumns)
{
	mOriginalNumPoints = mShow.GetNumPoints();
	mLabels = std::pair<std::vector<wxString>,std::vector<wxString> >(mShow.GetPointLabels(), labels);
	for (CC_show::CC_sheet_iterator_t sht = mShow.GetSheetBegin(); sht != mShow.GetSheetEnd(); ++sht)
	{
		mOriginalPoints.push_back(sht->GetPoints());
	}
}

SetShowInfoCommand::~SetShowInfoCommand()
{}

void SetShowInfoCommand::DoAction()
{
	mShow.SetNumPoints(mNumPoints, mNumColumns);
	mShow.SetPointLabel(mLabels.second);
}

void SetShowInfoCommand::UndoAction()
{
	size_t i = 0; 
	mShow.SetNumPoints(mOriginalNumPoints, 1);
	for (CC_show::CC_sheet_iterator_t sht = mShow.GetSheetBegin(); sht != mShow.GetSheetEnd(); ++sht, ++i)
	{
		sht->SetPoints(mOriginalPoints.at(i));
	}
	mShow.SetPointLabel(mLabels.first);
}


// SetSheetCommand:
// Base class for other commands.  Will set sheet to the state they were
// when command was created.
SetSheetCommand::SetSheetCommand(CC_show& show, const wxString cmdName)
: BasicCalChartCommand(show, cmdName),
mSheetNum(show.GetCurrentSheetNum())
{}

SetSheetCommand::~SetSheetCommand()
{}

void SetSheetCommand::DoAction()
{
	mShow.SetCurrentSheet(mSheetNum);
}

void SetSheetCommand::UndoAction()
{
	mShow.SetCurrentSheet(mSheetNum);
}


// SetSheetTitleCommand
// Set the description of the current sheet
SetSheetTitleCommand::SetSheetTitleCommand(CC_show& show, const wxString& newname)
: SetSheetCommand(show, wxT("Set title"))
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	mDescription = std::pair<wxString,wxString>(sheet->GetName(), newname);
}

SetSheetTitleCommand::~SetSheetTitleCommand()
{}

void SetSheetTitleCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->SetName(mDescription.second);
}

void SetSheetTitleCommand::UndoAction()
{
	SetSheetCommand::UndoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->SetName(mDescription.first);
}


// SetSheetBeatsCommand
// Set the beats of the current sheet
SetSheetBeatsCommand::SetSheetBeatsCommand(CC_show& show, unsigned short beats)
: SetSheetCommand(show, wxT("Set beats"))
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	mBeats = std::pair<unsigned short,unsigned short>(sheet->GetBeats(), beats);
}

SetSheetBeatsCommand::~SetSheetBeatsCommand()
{}

void SetSheetBeatsCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->SetBeats(mBeats.second);
	mShow.Modify(true);
}

void SetSheetBeatsCommand::UndoAction()
{
	SetSheetCommand::UndoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->SetBeats(mBeats.first);
	mShow.Modify(true);
}


// AddSheetsCommand
// For adding a container of sheets
AddSheetsCommand::AddSheetsCommand(CC_show& show, const CC_show::CC_sheet_container_t& sheets, unsigned where)
: SetSheetCommand(show, wxT("Moving points")),
mSheets(sheets), mWhere(where)
{}

AddSheetsCommand::~AddSheetsCommand()
{}

void AddSheetsCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mShow.InsertSheetInternal(mSheets, mWhere);
}

void AddSheetsCommand::UndoAction()
{
	mShow.RemoveNthSheet(mWhere);
	// intentionally do this afterwards
	SetSheetCommand::UndoAction(); // sets page
}


// RemoveSheetsCommand
// For removing a sheets
RemoveSheetsCommand::RemoveSheetsCommand(CC_show& show, unsigned where)
: SetSheetCommand(show, wxT("Moving points")),
mWhere(where)
{}

RemoveSheetsCommand::~RemoveSheetsCommand()
{}

void RemoveSheetsCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mSheets = mShow.RemoveNthSheet(mWhere);
}

void RemoveSheetsCommand::UndoAction()
{
	mShow.InsertSheetInternal(mSheets, mWhere);
	// intentionally do this afterwards
	SetSheetCommand::UndoAction(); // sets page
}


// SetSheetAndSelectCommand:
// Base class for other commands.  Will set selection and page to the state they were
// when command was created.
SetSheetAndSelectCommand::SetSheetAndSelectCommand(CC_show& show, const wxString cmdName)
: SetSheetCommand(show, cmdName),
mPoints(show.GetSelectionList())
{}

SetSheetAndSelectCommand::~SetSheetAndSelectCommand()
{}

void SetSheetAndSelectCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	mShow.SetSelection(mPoints);
}

void SetSheetAndSelectCommand::UndoAction()
{
	SetSheetCommand::UndoAction(); // sets page
	mShow.SetSelection(mPoints);
}


// Added or remove continuity base class
AddRemoveContinuityCommand::AddRemoveContinuityCommand(CC_show& show)
: SetSheetCommand(show, wxT("Adding or removing continuity"))
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	mOrigAnimcont = sheet->animcont;
}

AddRemoveContinuityCommand::~AddRemoveContinuityCommand()
{}

void AddRemoveContinuityCommand::UndoAction()
{
	SetSheetCommand::UndoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->animcont = mOrigAnimcont;
}

AddContinuityCommand::AddContinuityCommand(CC_show& show, const wxString& text)
: AddRemoveContinuityCommand(show),
mContName(text)
{}

AddContinuityCommand::~AddContinuityCommand()
{}

void AddContinuityCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	CC_continuity newcont(mContName, sheet->NextUnusedContinuityNum());
	sheet->AppendContinuity(newcont);
}


RemoveContinuityCommand::RemoveContinuityCommand(CC_show& show, unsigned index)
: AddRemoveContinuityCommand(show),
mIndexToRemove(index)
{}

RemoveContinuityCommand::~RemoveContinuityCommand()
{}

void RemoveContinuityCommand::DoAction()
{
	SetSheetCommand::DoAction(); // sets page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->RemoveNthContinuity(mIndexToRemove);
}

// MovePointsOnSheetCommand:
// Move points around on a sheet.  mPosition is a mapping of which point to two positions,
// the original and new.  Do will move points to the new position, and undo will move them back.
// Fill out the mPosition in the constructor.
MovePointsOnSheetCommand::MovePointsOnSheetCommand(CC_show& show, unsigned ref)
: SetSheetAndSelectCommand(show, wxT("Moving points")),
mRef(ref)
{}

MovePointsOnSheetCommand::~MovePointsOnSheetCommand()
{}

void MovePointsOnSheetCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<CC_coord,CC_coord> >::const_iterator i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(i->second.second, i->first, mRef);
	}
}

void MovePointsOnSheetCommand::UndoAction()
{
	SetSheetAndSelectCommand::UndoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<CC_coord,CC_coord> >::const_iterator i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(i->second.first, i->first, mRef);
	}
}


// TranslatePointsByDeltaCommand:
// Move the selected points by a fixed delta
TranslatePointsByDeltaCommand::TranslatePointsByDeltaCommand(CC_show& show, const CC_coord& delta, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mPositions[*i] = std::pair<CC_coord,CC_coord>(sheet->GetPosition(*i, mRef), sheet->GetPosition(*i, mRef) + delta);
	}
}

TranslatePointsByDeltaCommand::~TranslatePointsByDeltaCommand()
{}


// TransformPointsCommand:
// Move the selected points by a matrix function
TransformPointsCommand::TransformPointsCommand(CC_show& show, const Matrix& transmat, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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
{}


// TransformPointsInALineCommand:
// Move the selected points by a line function
TransformPointsInALineCommand::TransformPointsInALineCommand(CC_show& show, const CC_coord& start, const CC_coord& second, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	CC_coord curr_pos = start;
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i, curr_pos += second - start)
	{
		mPositions[*i] = std::pair<CC_coord,CC_coord>(sheet->GetPosition(*i, mRef), curr_pos);
	}
}

TransformPointsInALineCommand::~TransformPointsInALineCommand()
{
}


// SetContinuityIndexCommand:
// Sets the continuity index of the selected points.
SetContinuityIndexCommand::SetContinuityIndexCommand(CC_show& show, unsigned index)
: SetSheetAndSelectCommand(show, wxT("Setting Continuity Index"))
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mContinuity[*i] = std::pair<unsigned,unsigned>(sheet->GetPoint(*i).cont, index);
	}
}

SetContinuityIndexCommand::~SetContinuityIndexCommand()
{}

void SetContinuityIndexCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<unsigned,unsigned> >::const_iterator i = mContinuity.begin(); i != mContinuity.end(); ++i)
	{
		sheet->GetPoint(i->first).cont = i->second.second;
	}
}

void SetContinuityIndexCommand::UndoAction()
{
	SetSheetAndSelectCommand::UndoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<unsigned,unsigned> >::const_iterator i = mContinuity.begin(); i != mContinuity.end(); ++i)
	{
		sheet->GetPoint(i->first).cont = i->second.first;
	}
}


// SetSymbolAndContCommand:
// Sets the symbol for the selected points, creating one if it doesn't exist
SetSymbolAndContCommand::SetSymbolAndContCommand(CC_show& show, SYMBOL_TYPE sym)
: SetSheetAndSelectCommand(show, wxT("Setting Continuity Symbol"))
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
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
{}

void SetSymbolAndContCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
// possible optimization:  Don't save as map, since they all get moved to new symbol...
	for (std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> >::const_iterator i = mSymsAndCont.begin(); i != mSymsAndCont.end(); ++i)
	{
		sheet->GetPoint(i->first).sym = i->second.second.first;
		sheet->GetPoint(i->first).cont = sheet->GetStandardContinuity(i->second.second.first).GetNum();
	}
}

void SetSymbolAndContCommand::UndoAction()
{
	SetSheetAndSelectCommand::UndoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> >::const_iterator i = mSymsAndCont.begin(); i != mSymsAndCont.end(); ++i)
	{
		sheet->GetPoint(i->first).sym = i->second.first.first;
		sheet->GetPoint(i->first).cont = i->second.first.second;
	}
	sheet->animcont = mOrigAnimcont;
}


// SetContinuityTextCommand
// Sets the continuity text
SetContinuityTextCommand::SetContinuityTextCommand(CC_show& show, unsigned i, const wxString& text)
: SetSheetAndSelectCommand(show, wxT("Setting Continuity Text")),
mWhichCont(i)
{
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	mContinuity = std::pair<wxString,wxString>(sheet->GetNthContinuity(mWhichCont).GetText(), text);
}

SetContinuityTextCommand::~SetContinuityTextCommand()
{}

void SetContinuityTextCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->SetNthContinuity(mContinuity.second, mWhichCont);
}

void SetContinuityTextCommand::UndoAction()
{
	SetSheetAndSelectCommand::UndoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->SetNthContinuity(mContinuity.first, mWhichCont);
}


SetLabelCommand::SetLabelCommand(CC_show& show)
: SetSheetAndSelectCommand(show, wxT("Setting Labels"))
{}

SetLabelCommand::~SetLabelCommand()
{}

void SetLabelCommand::DoAction()
{
	SetSheetAndSelectCommand::DoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<bool,bool> >::const_iterator i = mLabelPos.begin(); i != mLabelPos.end(); ++i)
	{
		sheet->GetPoint(i->first).Flip(i->second.second);
	}
}

void SetLabelCommand::UndoAction()
{
	SetSheetAndSelectCommand::UndoAction(); // sets selected and page
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<bool,bool> >::const_iterator i = mLabelPos.begin(); i != mLabelPos.end(); ++i)
	{
		sheet->GetPoint(i->first).Flip(i->second.first);
	}
}


SetLabelRightCommand::SetLabelRightCommand(CC_show& show, bool right)
: SetLabelCommand(show)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = std::pair<bool,bool>(sheet->GetPoint(*i).GetFlip(), right);
	}
}

SetLabelRightCommand::~SetLabelRightCommand()
{}


SetLabelFlipCommand::SetLabelFlipCommand(CC_show& show)
: SetLabelCommand(show)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = std::pair<bool,bool>(sheet->GetPoint(*i).GetFlip(), !sheet->GetPoint(*i).GetFlip());
	}
}

SetLabelFlipCommand::~SetLabelFlipCommand()
{}

