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
#include <functional>

static const std::string k_nofile_str = "Unable to open file";
static const std::string k_badcont_str = "Error in continuity file";
static const std::string k_contnohead_str = "Continuity file doesn't begin with header";


// you can create a show in two ways, from scratch, or from an input stream
std::unique_ptr<CC_show>
CC_show::Create_CC_show()
{
	return std::unique_ptr<CC_show>(new CC_show());
}

std::unique_ptr<CC_show>
CC_show::Create_CC_show(std::istream& stream, uint32_t version) {
	if (version > 0x303) {
		return Create_CC_show_From_Stream_Fragment(std::istream_iterator<uint8_t>(stream), std::istream_iterator<uint8_t>(), version);
	}
	return std::unique_ptr<CC_show>(new CC_show(stream, Version_3_3_and_earlier()));
}

std::unique_ptr<CC_show>
CC_show::Create_CC_show_From_Stream_Fragment(std::istream_iterator<uint8_t> streamStart, std::istream_iterator<uint8_t> streamEnd, uint32_t version)
{
	if (version <= 0x303) {
		return std::unique_ptr<CC_show>();
	}
	std::vector<uint8_t> data(streamStart, streamEnd);
	// debug purposes, you can uncomment this line to have the show dumped
	//	DoRecursiveParsing("", data.data(), data.data() + data.size());
	return std::unique_ptr<CC_show>(new CC_show(data.data(), data.size(), Current_version_and_later()));
}

// Create a new show
CC_show::CC_show() :
numpoints(0),
mSheetNum(0)
{
}

// -=-=-=-=-=- LEGACY CODE -=-=-=-=-=-
// Recommend that you don't touch this unless you know what you are doing.
// Constructor for shows 3.3 and ealier.
CC_show::CC_show(std::istream& stream, Version_3_3_and_earlier ver) :
numpoints(0),
mSheetNum(0)
{
	// caller should have stripped off INGL and GURK headers
	/*
	ReadAndCheckID(stream, INGL_INGL);
	uint32_t version = ReadGurkSymbolAndGetVersion(stream, INGL_GURK);
	*/
	ReadAndCheckID(stream, INGL_SHOW);
	
	// Handle show info
	// read in the size:
	// <INGL_SIZE><4><# points>
	numpoints = ReadCheckIDandSize(stream, INGL_SIZE);
	pt_labels.assign(numpoints, std::string());
	
	uint32_t name = ReadLong(stream);
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
		
		CC_sheet sheet(GetNumPoints(), stream, ver);
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
// -=-=-=-=-=- LEGACY CODE </end>-=-=-=-=-=-

CC_show::CC_show(const uint8_t* ptr, size_t size, Current_version_and_later ver) :
numpoints(0),
mSheetNum(0)
{
	// caller should have stripped off INGL and GURK headers

	// construct the parser handlers
	// TODO: Why can't I capture this here?
	auto parse_INGL_SIZE = [](CC_show* show, const uint8_t* ptr, size_t size)
	{
		if (4 != size)
		{
			throw CC_FileException("Incorrect size", INGL_SIZE);
		}
		show->numpoints = get_big_long(ptr);
		show->pt_labels.assign(show->numpoints, std::string());
	};
	auto parse_INGL_LABL = [](CC_show* show, const uint8_t* ptr, size_t size)
	{
		std::vector<std::string> labels;
		auto str = (const char*)ptr;
		for (unsigned i = 0; i < show->GetNumPoints(); i++)
		{
			labels.push_back(str);
			auto length = strlen(str) + 1;
			if (length > size)
			{
				throw CC_FileException("Label too large", INGL_LABL);
			}
			str += length;
			size -= length;
		}
		if (size != 0)
		{
			throw CC_FileException("Label the wrong size", INGL_LABL);
		}
		show->SetPointLabel(labels);
	};
	auto parse_INGL_DESC = [](CC_show* show, const uint8_t* ptr, size_t size)
	{
		auto str = (const char*)ptr;
		if (size != (strlen(str) + 1))
		{
			throw CC_FileException("Description the wrong size", INGL_DESC);
		}
		show->SetDescr(std::string(str, strlen(str)));
	};
	auto parse_INGL_SHET = [](CC_show* show, const uint8_t* ptr, size_t size)
	{
		CC_sheet sheet(show->GetNumPoints(), ptr, size, Current_version_and_later());
		show->InsertSheetInternal(sheet, show->GetNumSheets());
	};
	// [=] needed here to pull in the parse functions
	auto parse_INGL_SHOW = [=](CC_show* show, const uint8_t* ptr, size_t size)
	{
		static const std::map<uint32_t, std::function<void(CC_show* show, const uint8_t*, size_t)>> parser = {
			{ INGL_SIZE, parse_INGL_SIZE },
			{ INGL_LABL, parse_INGL_LABL },
			{ INGL_DESC, parse_INGL_DESC },
			{ INGL_SHET, parse_INGL_SHET },
		};
		auto table = CalChart::Parser::ParseOutLabels(ptr, ptr + size);
		for (auto& i : table)
		{
			auto the_parser = parser.find(std::get<0>(i));
			if (the_parser != parser.end())
			{
				the_parser->second(show, std::get<1>(i), std::get<2>(i));
			}
		}
	};
	parse_INGL_SHOW(this, ptr, size);
	// now set the show to sheet 0
	mSheetNum = 0;
}

// Destroy a show
CC_show::~CC_show()
{}

std::vector<uint8_t>
CC_show::SerializeShow() const
{
	using CalChart::Parser::Append;
	using CalChart::Parser::AppendAndNullTerminate;
	using CalChart::Parser::Construct_block;
	std::vector<uint8_t> result;
	// SHOW_DATA          = NUM_MARCH , LABEL , [ DESCRIPTION ] , { SHEET }* ;
	// Write NUM_MARCH
	Append(result, Construct_block(INGL_SIZE, uint32_t{ GetNumPoints() }));

	// Write LABEL
	std::vector<char> labels;
	for (auto& i : pt_labels)
	{
		AppendAndNullTerminate(labels, i);
	}
	Append(result, Construct_block(INGL_LABL, labels));

	// write Description
	if (!GetDescr().empty())
	{
		std::vector<char> descr;
		AppendAndNullTerminate(descr, GetDescr());
		Append(result, Construct_block(INGL_DESC, descr));
	}

	// Handle sheets
	for (auto& sheet : sheets)
	{
		Append(result, sheet.SerializeSheet());
	}
	return result;
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
	InsertSheetInternal(CC_sheet(GetNumPoints(), "1"), 0);
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
	pt_labels.resize(numpoints);
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


bool CC_show::AlreadyHasPrintContinuity() const
{
	for (auto& i : sheets)
	{
		if (i.GetPrintableContinuity().size() > 0)
		{
			return true;
		}
	}
	return false;
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


// -=-=-=-=-=-=- Unit Tests -=-=-=-=-=-=-=-
#include <assert.h>
using namespace CalChart::Parser;

static std::vector<char> Construct_show_zero_points_zero_labels_zero_description()
{
	std::vector<char> show_data;
	Append(show_data, Construct_block(INGL_SIZE, std::vector<char>(4)));
	Append(show_data, Construct_block(INGL_LABL, std::vector<char>{}));
	Append(show_data, Construct_block(INGL_DESC, std::vector<char>(1)));
	return Construct_block(INGL_SHOW, show_data);
}

static std::vector<char> Construct_show_zero_points_zero_labels()
{
	std::vector<char> show_data;
	Append(show_data, Construct_block(INGL_SIZE, std::vector<char>(4)));
	Append(show_data, Construct_block(INGL_LABL, std::vector<char>{}));
	return Construct_block(INGL_SHOW, show_data);
}

static std::vector<char> Construct_show_zero_points_zero_labels_and_random()
{
	std::vector<char> show_data;
	Append(show_data, Construct_block(0x12345678, std::vector<char>(4)));
	Append(show_data, Construct_block(INGL_SIZE, std::vector<char>(4)));
	Append(show_data, Construct_block(0x87654321, std::vector<char>(13)));
	Append(show_data, Construct_block(INGL_LABL, std::vector<char>{}));
	Append(show_data, Construct_block(0xDEADBEEF, std::vector<char>(1)));
	return Construct_block(INGL_SHOW, show_data);
}

/**
void CC_show::CC_show_round_trip_test()
{
	auto blank_show = CC_show::Create_CC_show();
	auto blank_show_data = blank_show->SerializeShow();
	std::vector<char> char_data{blank_show_data.begin(), blank_show_data.end()};
	std::istringstream is(std::string{char_data.data(), char_data.size()});
	auto re_read_show = CC_show::Create_CC_show(is);
	auto re_read_show_data = re_read_show->SerializeShow();
	bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), re_read_show_data.begin());
	assert(is_equal);
}
**/

void CC_show::CC_show_round_trip_test_with_number_label_description()
{
	std::vector<char> point_data;
	Append(point_data, uint32_t{1});
	std::vector<char> data;
	Append(data, Construct_block(INGL_SIZE, point_data));
	Append(data, Construct_block(INGL_LABL, std::vector<char>{'p','o','i','n','t','\0'}));
	Append(data, Construct_block(INGL_DESC, std::vector<char>{'d','e','s','c','r','i','p','t','i','o','n','\0'}));
	auto show_data = Construct_block(INGL_SHOW, data);

	CC_show show1((const uint8_t*)show_data.data(), show_data.size(), Current_version_and_later());
	auto show1_data = show1.SerializeShow();
	// eat header
	show1_data.erase(show1_data.begin(), show1_data.begin()+8);
	auto is_equal = show1_data.size() == show_data.size() && std::equal(show1_data.begin(), show1_data.end(), show_data.begin());
	assert(is_equal);

	// now check that things loaded correctly
	assert(show1.GetNumPoints() == 1);
	assert(show1.GetNumSheets() == 0);
	assert(show1.GetPointLabel(0) == "point");
	assert(show1.GetDescr() == "description");
}

void CC_show::CC_show_blank_desc_test()
{
	auto show_zero_points_zero_labels_zero_description = Construct_show_zero_points_zero_labels_zero_description();
	CC_show show1((const uint8_t*)show_zero_points_zero_labels_zero_description.data(), show_zero_points_zero_labels_zero_description.size(), Current_version_and_later());
	auto show1_data = show1.SerializeShow();
	// eat header
	show1_data.erase(show1_data.begin(), show1_data.begin()+8);
	bool is_equal = show1_data.size() == show_zero_points_zero_labels_zero_description.size() && std::equal(show1_data.begin(), show1_data.end(), show_zero_points_zero_labels_zero_description.begin());
	assert(!is_equal);

	// now remove the description and they should be equal
	auto show_zero_points_zero_labels = Construct_show_zero_points_zero_labels();
	CC_show show2((const uint8_t*)show_zero_points_zero_labels.data(), show_zero_points_zero_labels.size(), Current_version_and_later());
	auto show2_data = show2.SerializeShow();
	show2_data.erase(show2_data.begin(), show2_data.begin()+8);
	is_equal = show2_data.size() == show_zero_points_zero_labels.size() && std::equal(show2_data.begin(), show2_data.end(), show_zero_points_zero_labels.begin());
	assert(is_equal);
}
/**
// confirm we try to handle shows from the future
void CC_show::CC_show_future_show_test()
{
	// how?  By creating a show from scratch, then modifying the version; make sure that we load it, and it looks the same
	// except the data gets reverted
	auto blank_show = CC_show::Create_CC_show();
	auto blank_show_data = blank_show->SerializeShow();
	std::vector<char> char_data{blank_show_data.begin(), blank_show_data.end()};
	assert(char_data.at(6)-'0' == CC_MAJOR_VERSION && char_data.at(7)-'0' == CC_MINOR_VERSION);
	++char_data.at(6);
	++char_data.at(7);
	std::istringstream is(std::string{char_data.data(), char_data.size()});
	auto re_read_show = CC_show::Create_CC_show(is);
	auto re_read_show_data = blank_show->SerializeShow();
	--char_data.at(6);
	--char_data.at(7);
	bool is_equal = blank_show_data.size() == re_read_show_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), re_read_show_data.begin());
	assert(is_equal);
}
**/

void CC_show::CC_show_wrong_size_throws_exception()
{
	auto points_3(Construct_block(INGL_SIZE, std::vector<char>(3)));
	auto show_data = Construct_block(INGL_SHOW, points_3);
	bool hit_exception = false;
	try {
		CC_show show1((const uint8_t*)show_data.data(), show_data.size(), Current_version_and_later());
	}
	catch (const CC_FileException&)
	{
		hit_exception = true;
	}
	assert(hit_exception);
}

// too large, and too small
void CC_show::CC_show_wrong_size_number_labels_throws()
{
	{
		std::vector<char> point_data(4);
		put_big_long(point_data.data(), 1);
		auto points(Construct_block(INGL_SIZE, point_data));
		auto no_labels(Construct_block(INGL_LABL, std::vector<char>{}));
		auto t_show_data = points;
		t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
		auto show_data = Construct_block(INGL_SHOW, t_show_data);
		bool hit_exception = false;
		try {
			CC_show show1((const uint8_t*)show_data.data(), show_data.size(), Current_version_and_later());
		}
		catch (const CC_FileException&)
		{
			hit_exception = true;
		}
		assert(hit_exception);
	}
	{
		std::vector<char> point_data(4);
		put_big_long(point_data.data(), 1);
		auto points(Construct_block(INGL_SIZE, point_data));
		auto labels(Construct_block(INGL_LABL, std::vector<char>{'a','\0','b','\0'}));
		auto t_show_data = points;
		t_show_data.insert(t_show_data.end(), labels.begin(), labels.end());
		auto show_data = Construct_block(INGL_SHOW, t_show_data);
		bool hit_exception = false;
		try {
			CC_show show1((const uint8_t*)show_data.data(), show_data.size(), Current_version_and_later());
		}
		catch (const CC_FileException&)
		{
			hit_exception = true;
		}
		assert(hit_exception);
	}
}

// too large, and too small
void CC_show::CC_show_wrong_size_description()
{
	{
		auto no_points(Construct_block(INGL_SIZE, std::vector<char>(4)));
		auto no_labels(Construct_block(INGL_LABL, std::vector<char>{}));
		auto descr(Construct_block(INGL_DESC, std::vector<char>{'a','b','c','\0'}));
		descr.at(9) = '\0';
		auto t_show_data = no_points;
		t_show_data.insert(t_show_data.end(), no_labels.begin(), no_labels.end());
		t_show_data.insert(t_show_data.end(), descr.begin(), descr.end());
		auto show_data = Construct_block(INGL_SHOW, t_show_data);
		bool hit_exception = false;
		try {
			CC_show show1((const uint8_t*)show_data.data(), show_data.size(), Current_version_and_later());
		}
		catch (const CC_FileException&)
		{
			hit_exception = true;
		}
		assert(hit_exception);
	}
}


// extra cruft ok
void CC_show::CC_show_extra_cruft_ok()
{
	// now remove the description and they should be equal
	auto extra_cruft = Construct_show_zero_points_zero_labels_and_random();
	CC_show show1((const uint8_t*)extra_cruft.data(), extra_cruft.size(), Current_version_and_later());
	auto show1_data = show1.SerializeShow();

	auto blank_show = CC_show::Create_CC_show();
	auto blank_show_data = blank_show->SerializeShow();
	auto is_equal = blank_show_data.size() == show1_data.size() && std::equal(blank_show_data.begin(), blank_show_data.end(), show1_data.begin());
	assert(is_equal);
}

// show with nothing should fail:
void CC_show::CC_show_with_nothing_throws()
{
	std::vector<char> empty{};
	bool hit_exception = false;
	try {
		CC_show show1((const uint8_t*)empty.data(), empty.size(), Current_version_and_later());
	}
	catch (const CC_FileException&)
	{
		hit_exception = true;
	}
	assert(hit_exception);
}

void CC_show_UnitTests()
{
	CC_show::CC_show_round_trip_test();
	CC_show::CC_show_round_trip_test_with_number_label_description();
	CC_show::CC_show_blank_desc_test();
	CC_show::CC_show_future_show_test();
	CC_show::CC_show_wrong_size_throws_exception();
	CC_show::CC_show_wrong_size_number_labels_throws();
	CC_show::CC_show_wrong_size_description();
	CC_show::CC_show_extra_cruft_ok();
	CC_show::CC_show_with_nothing_throws();
}


