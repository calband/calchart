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
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<CC_coord,CC_coord> >::const_iterator i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(i->second.second, i->first, mRef);
	}
	mShow.Modify(true);
	return true;
}

bool MovePointsOnSheetCommand::Undo()
{
	mShow.UnselectAll();
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	for (std::map<unsigned, std::pair<CC_coord,CC_coord> >::const_iterator i = mPositions.begin(); i != mPositions.end(); ++i)
	{
		sheet->SetPosition(i->second.first, i->first, mRef);
	}
	mShow.Modify(true);
	return true;
}

TranslatePointsByDeltaCommand::TranslatePointsByDeltaCommand(CC_show& show, const CC_coord& delta, unsigned ref)
: MovePointsOnSheetCommand(show, ref)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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
{
}

// Move points into a line (old smart move)
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


SetContinuityIndexCommand::SetContinuityIndexCommand(CC_show& show, unsigned index)
: wxCommand(true, wxT("Setting Continuity Index")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList())
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<unsigned,unsigned> >::const_iterator i = mContinuity.begin(); i != mContinuity.end(); ++i)
	{
		sheet->GetPoint(i->first).cont = i->second.second;
	}
	mShow.Modify(true);
	return true;
}

bool SetContinuityIndexCommand::Undo()
{
	mShow.UnselectAll();
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<unsigned,unsigned> >::const_iterator i = mContinuity.begin(); i != mContinuity.end(); ++i)
	{
		sheet->GetPoint(i->first).cont = i->second.first;
	}
	mShow.Modify(true);
	return true;
}


SetSymbolAndContCommand::SetSymbolAndContCommand(CC_show& show, SYMBOL_TYPE sym)
: wxCommand(true, wxT("Setting Continuity Index")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList())
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

// possible optimization:  Don't save as map, since they all get moved to new symbol...
	for (std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> >::const_iterator i = mSymsAndCont.begin(); i != mSymsAndCont.end(); ++i)
	{
		sheet->GetPoint(i->first).sym = i->second.second.first;
		sheet->GetPoint(i->first).cont = sheet->GetStandardContinuity(i->second.second.first).GetNum();
	}
	mShow.Modify(true);
	return true;
}

bool SetSymbolAndContCommand::Undo()
{
	mShow.UnselectAll();
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

	for (std::map<unsigned, std::pair<sym_cont_t,sym_cont_t> >::const_iterator i = mSymsAndCont.begin(); i != mSymsAndCont.end(); ++i)
	{
		sheet->GetPoint(i->first).sym = i->second.first.first;
		sheet->GetPoint(i->first).cont = i->second.first.second;
	}
	sheet->animcont = mOrigAnimcont;
	mShow.Modify(true);
	return true;
}


SetContinuityTextCommand::SetContinuityTextCommand(CC_show& show, unsigned i, const wxString& text)
: wxCommand(true, wxT("Setting Continuity Text")),
mShow(show), mSheetNum(show.GetCurrentSheetNum()), mPoints(show.GetSelectionList()), mWhichCont(i)
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

	sheet->SetNthContinuity(mContinuity.second, mWhichCont);
	mShow.Modify(true);
	return true;
}

bool SetContinuityTextCommand::Undo()
{
//	mShow.UnselectAll();
//	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
//		mShow.Select(*i, true);
//
	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

	sheet->SetNthContinuity(mContinuity.first, mWhichCont);
	mShow.Modify(true);
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
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

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
	mShow.AddToSelection(mPoints);

	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();

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
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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
	CC_show::CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	for (CC_show::SelectionList::const_iterator i = mPoints.begin(); i != mPoints.end(); ++i)
	{
		mLabelPos[*i] = std::pair<bool,bool>(sheet->GetPoint(*i).GetFlip(), !sheet->GetPoint(*i).GetFlip());
	}
}

SetLabelFlipCommand::~SetLabelFlipCommand()
{
}

// For adding a single or container of sheets
AddSheetsCommand::AddSheetsCommand(CC_show& show, const CC_show::CC_sheet_container_t& sheets, unsigned where)
: wxCommand(true, wxT("Moving points")),
mShow(show), mSheets(sheets), mWhere(where)
{
}

AddSheetsCommand::~AddSheetsCommand()
{
}

bool AddSheetsCommand::Do()
{
	mShow.InsertSheetInternal(mSheets, mWhere);
	mShow.Modify(true);
	return true;
}

bool AddSheetsCommand::Undo()
{
	mShow.RemoveNthSheet(mWhere);
	mShow.Modify(true);
	return true;
}

// For removing a single
RemoveSheetsCommand::RemoveSheetsCommand(CC_show& show, unsigned where)
: wxCommand(true, wxT("Moving points")),
mShow(show), mWhere(where)
{
}

RemoveSheetsCommand::~RemoveSheetsCommand()
{
}

bool RemoveSheetsCommand::Do()
{
	mSheets = mShow.RemoveNthSheet(mWhere);
	mShow.Modify(true);
	return true;
}

bool RemoveSheetsCommand::Undo()
{
	mShow.InsertSheetInternal(mSheets, mWhere);
	mShow.Modify(true);
	return true;
}

// Added or remove continuity base class
AddRemoveContinuityCommand::AddRemoveContinuityCommand(CC_show& show)
: wxCommand(true, wxT("Adding or removing continuity")),
mShow(show), mSheetNum(show.GetCurrentSheetNum())
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	mOrigAnimcont = sheet->animcont;
}

AddRemoveContinuityCommand::~AddRemoveContinuityCommand()
{
}

bool AddRemoveContinuityCommand::Undo()
{
	mShow.SetCurrentSheet(mSheetNum);
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
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
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
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
	CC_show::CC_sheet_iterator_t sheet = mShow.GetCurrentSheet();
	sheet->RemoveNthContinuity(mIndexToRemove);
	mShow.Modify(true);
	return true;
}


SetSheetTitleCommand::SetSheetTitleCommand(CC_show& show, const wxString& newname)
: wxCommand(true, wxT("Moving points")),
mShow(show), mSheetNum(show.GetCurrentSheetNum())
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	mDescription = std::pair<wxString,wxString>(sheet->GetName(), newname);
}

SetSheetTitleCommand::~SetSheetTitleCommand()
{
}


bool SetSheetTitleCommand::Do()
{
	CC_show::CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetName(mDescription.second);
	mShow.Modify(true);
	return true;
}

bool SetSheetTitleCommand::Undo()
{
	CC_show::CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetName(mDescription.first);
	mShow.Modify(true);
	return true;
}

SetSheetBeatsCommand::SetSheetBeatsCommand(CC_show& show, unsigned short beats)
: wxCommand(true, wxT("setting beats")),
mShow(show), mSheetNum(show.GetCurrentSheetNum())
{
	CC_show::const_CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	mBeats = std::pair<unsigned short,unsigned short>(sheet->GetBeats(), beats);
}

SetSheetBeatsCommand::~SetSheetBeatsCommand()
{
}


bool SetSheetBeatsCommand::Do()
{
	CC_show::CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
	sheet->SetBeats(mBeats.second);
	mShow.Modify(true);
	return true;
}

bool SetSheetBeatsCommand::Undo()
{
	CC_show::CC_sheet_iterator_t sheet = mShow.GetNthSheet(mSheetNum);
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

SetModeCommand::SetModeCommand(CC_show& show, const wxString& newmode)
: wxCommand(true, wxT("Set Mode")),
mShow(show)
{
	mMode = std::pair<wxString,wxString>(show.GetMode().GetName(), newmode);
}

SetModeCommand::~SetModeCommand()
{
}


bool SetModeCommand::Do()
{
	ShowMode *newmode = wxGetApp().GetModeList().Find(mMode.second);
	if (newmode)
	{
		mShow.SetMode(newmode);
	}
	mShow.Modify(true);
	return true;
}

bool SetModeCommand::Undo()
{
	ShowMode *newmode = wxGetApp().GetModeList().Find(mMode.first);
	if (newmode)
	{
		mShow.SetMode(newmode);
	}
	mShow.Modify(true);
	return true;
}

SetShowInfoCommand::SetShowInfoCommand(CC_show& show, unsigned numPoints, unsigned numColumns, const std::vector<wxString>& labels)
: wxCommand(true, wxT("Set show info")),
mShow(show), mNumPoints(numPoints), mNumColumns(numColumns)
{
	for (CC_show::CC_sheet_iterator_t sht = mShow.GetSheetBegin(); sht != mShow.GetSheetEnd(); ++sht)
	{
		mOriginalPoints.push_back(sht->GetPoints());
	}
	mOriginalNumPoints = mShow.GetNumPoints();
	mLabels = std::pair<std::vector<wxString>,std::vector<wxString> >(mShow.GetPointLabels(), labels);
}

SetShowInfoCommand::~SetShowInfoCommand()
{
}


bool SetShowInfoCommand::Do()
{
	mShow.SetNumPoints(mNumPoints, mNumColumns);
	mShow.SetPointLabel(mLabels.second);
	mShow.Modify(true);
	return true;
}

bool SetShowInfoCommand::Undo()
{
	size_t i = 0; 
	mShow.SetNumPoints(mOriginalNumPoints, 1);
	for (CC_show::CC_sheet_iterator_t sht = mShow.GetSheetBegin(); sht != mShow.GetSheetEnd(); ++sht, ++i)
	{
		sht->SetPoints(mOriginalPoints.at(i));
	}
	mShow.SetPointLabel(mLabels.first);
	mShow.Modify(true);
	return true;
}
