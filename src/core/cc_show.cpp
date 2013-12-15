/*
 * cc_show.cpp
 * Member functions for calchart show classes
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

#include "cc_show.h"

#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "cc_shapes.h"
#include "cc_fileformat.h"
#include "ccvers.h"

#include <sstream>

static const std::string k_nofile_str = "Unable to open file";
static const std::string k_badcont_str = "Error in continuity file";
static const std::string k_contnohead_str = "Continuity file doesn't begin with header";


// Create a new show
CC_show::CC_show() :
numpoints(0),
mSheetNum(0)
{
}


CC_show::CC_show(std::istream& stream) :
numpoints(0),
mSheetNum(0)
{
	uint32_t name;
	
	ReadAndCheckID(stream, INGL_INGL);
	uint32_t version = ReadGurkSymbolAndGetVersion(stream, INGL_GURK);
	if (version > ((CC_MAJOR_VERSION<<8) | (CC_MINOR_VERSION)))
	{
		// what do we do if we don't support it?
		;
	}
	ReadAndCheckID(stream, INGL_SHOW);
	
	// Handle show info
	// read in the size:
	// <INGL_SIZE><4><# points>
	name = ReadCheckIDandSize(stream, INGL_SIZE);
	numpoints = name;
	pt_labels.assign(numpoints, std::string());
	
	name = ReadLong(stream);
	// Optional: read in the point labels
	// <INGL_LABL><SIZE>
	if (INGL_LABL == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		std::vector<std::string> labels;
		const char *str = (const char*)&data[0];
		for (unsigned i = 0; i < GetNumPoints(); i++)
		{
			labels.push_back(str);
			str += strlen(str)+1;
		}
		SetPointLabel(labels);
		// peek for the next name
		name = ReadLong(stream);
	}
	else
	{
		// fail?
	}
	
	// Optional: read in the point labels
	// <INGL_DESC><SIZE>
	if (INGL_DESC == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		auto str = (const char*)&data[0];
		SetDescr(std::string(str, strlen(str)));
		// peek for the next name
		name = ReadLong(stream);
	}
	
	// Read in sheets
	// <INGL_GURK><INGL_SHET>
	while (INGL_GURK == name)
	{
		ReadAndCheckID(stream, INGL_SHET);
		
		CC_sheet sheet(this, GetNumPoints(), stream);
		InsertSheetInternal(sheet, GetNumSheets());
		
		//ReadAndCheckID(stream, INGL_END);
		ReadAndCheckID(stream, INGL_SHET);
		// peek for the next name
		name = ReadLong(stream);
	}
	//ReadAndCheckID(stream, INGL_END);
	ReadAndCheckID(stream, INGL_SHOW);

	// now set the show to sheet 0
	mSheetNum = 0;
}

// Destroy a show
CC_show::~CC_show()
{}


std::string CC_show::ImportContinuity(const std::vector<std::string>& lines)
{
/* This is the format for each sheet:
 * %%str      where str is the string printed for the stuntsheet number
 * normal ascii text possibly containing the following codes:
 * \bs \be \is \ie for bold start, bold end, italics start, italics end
 * \po plainman
 * \pb backslashman
 * \ps slashman
 * \px xman
 * \so solidman
 * \sb solidbackslashman
 * \ss solidslashman
 * \sx solidxman
 * a line may begin with these symbols in order: <>~
 * < don't print continuity on individual sheets
 * > don't print continuity on master sheet
 * ~ center this line
 * also, there are three tab stops set for standard continuity format
 */

	// check to make sure the first line starts with %%
	if (!lines.empty())
	{
		if ((lines.at(0).length() < 2) || !((lines.at(0).at(0) == '%') && (lines.at(0).at(1) == '%')))
		{
			// Continuity doesn't begin with a sheet header
			return k_contnohead_str;
		}
	}

	// first, split the lines into groups for each page
	std::vector<std::vector<std::string> > lines_data;
	for (auto line = lines.begin(); line != lines.end(); ++line)
	{
		if ((line->length() >= 2) && (line->at(0) == '%') && (line->at(1) == '%'))
		{
			lines_data.push_back(std::vector<std::string>());
		}
		lines_data.back().push_back(*line);
	}
	
	auto line_data = lines_data.begin();
	auto curr_sheet = GetSheetBegin();
	for (; line_data != lines_data.end() && curr_sheet != GetSheetEnd(); ++line_data, ++curr_sheet)
	{
		if (!curr_sheet->ImportPrintableContinuity(*line_data))
		{
			return k_badcont_str;
		}
	}
	return "";
}


std::vector<uint8_t>
CC_show::SerializeShow() const
{
	std::ostringstream stream;
	uint32_t id;
	unsigned i;

	WriteHeader(stream);
	WriteGurk(stream, INGL_SHOW);

// Handle show info
	i = GetNumPoints();
	put_big_long(&id, i);
	WriteChunk(stream, INGL_SIZE, 4, &id);

	id = 0;
	for (i = 0; i < GetNumPoints(); i++)
	{
		id += strlen(GetPointLabel(i).c_str())+1;
	}
	if (id > 0)
	{
		WriteChunkHeader(stream, INGL_LABL, id);
		for (i = 0; i < GetNumPoints(); i++)
		{
			WriteStr(stream, GetPointLabel(i).c_str());
		}
	}

//handl->WriteChunkStr(INGL_MODE, mode->Name());

// Description
	if (!GetDescr().empty())
	{
		WriteChunkStr(stream, INGL_DESC, GetDescr().c_str());
	}

// Handle sheets
	for (const_CC_sheet_iterator_t curr_sheet = GetSheetBegin();
		curr_sheet != GetSheetEnd();
		++curr_sheet)
	{
		auto data = curr_sheet->SerializeSheet();
		Write(stream, &data[0], data.size());
	}

	WriteEnd(stream, INGL_SHOW);
	auto sdata = stream.str();
	std::vector<uint8_t> data;
	std::copy(sdata.begin(), sdata.end(), std::back_inserter(data));
	return data;
}


const std::string& CC_show::GetDescr() const
{
	return descr;
}


void CC_show::SetDescr(const std::string& newdescr)
{
	descr = newdescr;
}


size_t
CC_show::GetNumSheets() const
{
	return sheets.size();
}

CC_show::CC_sheet_iterator_t
CC_show::GetSheetBegin()
{
	return sheets.begin();
}

CC_show::const_CC_sheet_iterator_t
CC_show::GetSheetBegin() const
{
	return sheets.begin();
}

CC_show::CC_sheet_iterator_t
CC_show::GetSheetEnd()
{
	return sheets.end();
}

CC_show::const_CC_sheet_iterator_t
CC_show::GetSheetEnd() const
{
	return sheets.end();
}

CC_show::const_CC_sheet_iterator_t
CC_show::GetCurrentSheet() const
{
	return GetNthSheet(mSheetNum);
}

CC_show::CC_sheet_iterator_t
CC_show::GetCurrentSheet()
{
	return GetNthSheet(mSheetNum);
}

unsigned CC_show::GetCurrentSheetNum() const
{
	return mSheetNum;
}

CC_show::const_CC_sheet_iterator_t CC_show::GetNthSheet(unsigned n) const
{
	return GetSheetBegin() + n;
}


CC_show::CC_sheet_iterator_t CC_show::GetNthSheet(unsigned n)
{
	return GetSheetBegin() + n;
}


CC_show::CC_sheet_container_t CC_show::RemoveNthSheet(unsigned sheetidx)
{
	CC_sheet_iterator_t i = GetNthSheet(sheetidx);
	CC_sheet_container_t shts(1, *i);
	sheets.erase(i);

	if (sheetidx < GetCurrentSheetNum())
	{
		SetCurrentSheet(GetCurrentSheetNum()-1);
	}
	if (GetCurrentSheetNum() >=
		GetNumSheets())
	{
		SetCurrentSheet(GetNumSheets()-1);
	}

	return shts;
}


void CC_show::SetCurrentSheet(unsigned n)
{
	mSheetNum = n;
}


void
CC_show::SetupNewShow()
{
	InsertSheetInternal(CC_sheet(this, "1"), 0);
	SetCurrentSheet(0);
}

void CC_show::InsertSheetInternal(const CC_sheet& sheet, unsigned sheetidx)
{
	sheets.insert(sheets.begin() + sheetidx, sheet);
	if (sheetidx <= GetCurrentSheetNum())
		SetCurrentSheet(GetCurrentSheetNum()+1);
}


void CC_show::InsertSheetInternal(const CC_sheet_container_t& sheet, unsigned sheetidx)
{
	sheets.insert(sheets.begin() + sheetidx, sheet.begin(), sheet.end());
	if (sheetidx <= GetCurrentSheetNum())
		SetCurrentSheet(GetCurrentSheetNum()+1);
}


void CC_show::InsertSheet(const CC_sheet& nsheet, unsigned sheetidx)
{
	InsertSheetInternal(nsheet, sheetidx);
}


// warning, the labels might not match up
void CC_show::SetNumPoints(unsigned num, unsigned columns, const CC_coord& new_march_position)
{
	for (CC_sheet_iterator_t sht = GetSheetBegin(); sht != GetSheetEnd(); ++sht)
	{
		sht->SetNumPoints(num, columns, new_march_position);
	}
	numpoints = num;
}


bool CC_show::RelabelSheets(unsigned sht)
{
	unsigned i,j;

	CC_sheet_iterator_t sheet = GetNthSheet(sht);
	CC_sheet_iterator_t sheet_next = GetNthSheet(sht+1);
	if (sheet_next == GetSheetEnd()) return false;
	std::vector<size_t> table(GetNumPoints());
	std::vector<unsigned> used_table(GetNumPoints());

	for (i = 0; i < GetNumPoints(); i++)
	{
		for (j = 0; j < GetNumPoints(); j++)
		{
			if (!used_table[j])
			{
				if (sheet->GetPosition(i) == sheet_next->GetPosition(j))
				{
					table[i] = j;
					used_table[j] = true;
					break;
				}
			}
		}
		if (j == GetNumPoints())
		{
// didn't find a match
			return false;
		}
	}
	while ((++sheet) != GetSheetEnd())
	{
		sheet->RelabelSheet(table);
	}

	return true;
}


std::string CC_show::GetPointLabel(unsigned i) const
{
	if (i >= pt_labels.size())
		return "";
	return pt_labels.at(i);
}


bool CC_show::SelectAll()
{
	bool changed = selectionList.size() != numpoints;
	for (size_t i = 0; i < numpoints; ++i)
		selectionList.insert(i);
	return changed;
}


bool CC_show::UnselectAll()
{
	bool changed = selectionList.size();
	selectionList.clear();
//	UpdateAllViews();
	return changed;
}


void CC_show::SetSelection(const SelectionList& sl)
{
	selectionList = sl;
//	UpdateAllViews();
}


void CC_show::AddToSelection(const SelectionList& sl)
{
	selectionList.insert(sl.begin(), sl.end());
//	UpdateAllViews();
}

void CC_show::RemoveFromSelection(const SelectionList& sl)
{
	selectionList.erase(sl.begin(), sl.end());
//	UpdateAllViews();
}

void CC_show::ToggleSelection(const SelectionList& sl)
{
	for (SelectionList::const_iterator i = sl.begin(); i != sl.end(); ++i)
	{
		if (selectionList.count(*i))
		{
			selectionList.erase(*i);
		}
		else
		{
			selectionList.insert(*i);
		}
		
	}
//	UpdateAllViews();
}

// toggle selection means toggle it as selected to unselected
// otherwise, always select it
void CC_show::SelectWithLasso(const CC_lasso& lasso, bool toggleSelected, unsigned ref)
{
	if (!lasso.FirstPoint())
	{
		return;
	}
	
	SelectionList sl;
	CC_show::const_CC_sheet_iterator_t sheet = GetCurrentSheet();
	for (unsigned i = 0; i < GetNumPoints(); i++)
	{
		if (lasso.Inside(sheet->GetPosition(i, ref)))
		{
			sl.insert(i);
		}
	}
	if (toggleSelected)
	{
		ToggleSelection(sl);
	}
	else
	{
		AddToSelection(sl);
	}
}

