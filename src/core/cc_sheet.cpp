/*
 * cc_sheet.cpp
 * Defintion for calchart sheet class
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

#include "cc_sheet.h"

#include "cc_show.h"
#include "cc_fileformat.h"

#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
#include <iostream>
#include <map>

const std::string contnames[MAX_NUM_SYMBOLS] =
{
	"Plain",
	"Sol",
	"Bksl",
	"Sl",
	"X",
	"Solbksl",
	"Solsl",
	"Solx"
};

SYMBOL_TYPE GetSymbolForName(const std::string& name)
{
	for (auto i = contnames; i != (contnames+sizeof(contnames)/sizeof(contnames[0])); ++i)
	{
		if (boost::iequals(name, *i))
		{
			return static_cast<SYMBOL_TYPE>(std::distance(contnames, i));
		}
	}
	// what do we do here?  give larger one for now...
	// This should probably throw
	return MAX_NUM_SYMBOLS;
}

std::string GetNameForSymbol(SYMBOL_TYPE which)
{
	if (which > MAX_NUM_SYMBOLS)
	{
		return "";
	}
	return contnames[which];
}

CC_sheet::CC_sheet(CC_show *show) :
mAnimationContinuity(MAX_NUM_SYMBOLS),
beats(1),
pts(show->GetNumPoints())
{
}


CC_sheet::CC_sheet(CC_show *show, const std::string& newname) :
mAnimationContinuity(MAX_NUM_SYMBOLS),
beats(1),
pts(show->GetNumPoints()),
mName(newname)
{
}

static void CheckInconsistancy(SYMBOL_TYPE symbol, uint8_t cont_index, 	std::map<SYMBOL_TYPE, uint8_t>& continity_for_symbol, std::map<uint8_t, SYMBOL_TYPE>& symbol_for_continuity, const std::string& sheet_name, uint32_t pointNum)
{
	// need to check for symbol inconsistency here.
	if (continity_for_symbol.count(symbol) == 0)
	{
		// we haven't seen this symbol->cont_index yet
		continity_for_symbol[symbol] = cont_index;
	}
	else
	{
		if (continity_for_symbol[symbol] != cont_index)
		{
			std::stringstream buf;
			buf<<"Error, symbol inconsistency on sheet \""<<sheet_name<<"\".\n";
			buf<<"Symbol "<<GetNameForSymbol(symbol)<<" previously used continuity "<<(uint32_t)continity_for_symbol[symbol]<<" but point "<<pointNum<<" on uses continuity "<<(uint32_t)cont_index<<", which is used by symbol "<<GetNameForSymbol(symbol_for_continuity[cont_index])<<".\n";
			buf<<"Try opening this file on CalChart v3.3.5 or earlier.\n";
			throw CC_FileException(buf.str());
		}
	}
	if (symbol_for_continuity.count(cont_index) == 0)
	{
		symbol_for_continuity[cont_index] = symbol;
	}
	else
	{
		if (symbol_for_continuity[cont_index] != symbol)
		{
			std::stringstream buf;
			buf<<"Error, symbol inconsistency on sheet \""<<sheet_name<<"\".\n";
			buf<<"Continuity index "<<(uint32_t)cont_index<<" previously used symbol "<<GetNameForSymbol(symbol_for_continuity[cont_index])<<"  but point "<<pointNum<<" on uses symbol "<<GetNameForSymbol(symbol)<<".\n";
			buf<<"Try opening this file on CalChart v3.3.5 or earlier.\n";
			throw CC_FileException(buf.str());
		}
	}
}

CC_sheet::CC_sheet(CC_show *show, size_t numPoints, std::istream& stream, Version_3_3_and_earlier) :
mAnimationContinuity(MAX_NUM_SYMBOLS),
pts(numPoints)
{
	// Read in sheet name
	// <INGL_NAME><size><string + 1>
	std::vector<uint8_t> data = ReadCheckIDandFillData(stream, INGL_NAME);
	mName = (const char*)&data[0];
	
	// read in the duration:
	// <INGL_DURA><4><duration>
	auto chunk = ReadCheckIDandSize(stream, INGL_DURA);
	beats = chunk;
	
	// Point positions
	// <INGL_DURA><size><data>
	data = ReadCheckIDandFillData(stream, INGL_POS);
	if (data.size() != size_t(pts.size()*4))
	{
		throw CC_FileException("bad POS chunk");
	}
	{
		uint8_t *d = &data[0];
		for (unsigned i = 0; i < pts.size(); ++i)
		{
			CC_coord c;
			c.x = get_big_word(d);
			d += 2;
			c.y = get_big_word(d);
			d += 2;
			for (unsigned j = 0; j <= CC_point::kNumRefPoints; j++)
			{
				pts[i].SetPos(c, j);
			}
		}
	}
	
	uint32_t name = ReadLong(stream);
	// read all the reference points
	while (INGL_REFP == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() != pts.size()*4+2)
		{
			throw CC_FileException("Bad REFP chunk");
		}
		uint8_t *d = &data[0];
		unsigned ref = get_big_word(d);
		d += 2;
		for (unsigned i = 0; i < pts.size(); i++)
		{
			CC_coord c;
			c.x = get_big_word(d);
			d += 2;
			c.y = get_big_word(d);
			d += 2;
			pts[i].SetPos(c, ref);
		}
		name = ReadLong(stream);
	}
	// Point symbols
	while (INGL_SYMB == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() != pts.size())
		{
			throw CC_FileException("Bad SYMB chunk");
		}
		uint8_t *d = &data[0];
		for (unsigned i = 0; i < pts.size(); i++)
		{
			pts.at(i).SetSymbol((SYMBOL_TYPE)(*(d++)));
		}
		name = ReadLong(stream);
	}
	std::map<SYMBOL_TYPE, uint8_t> continity_for_symbol;
	std::map<uint8_t, SYMBOL_TYPE> symbol_for_continuity;
	bool has_type = false;
	// Point continuity types
	while (INGL_TYPE == name)
	{
		has_type = true;
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() != pts.size())
		{
			throw CC_FileException("Bad TYPE chunk");
		}
		uint8_t *d = &data[0];
		for (unsigned i = 0; i < pts.size(); i++)
		{
			CheckInconsistancy(pts[i].GetSymbol(), *(d++), continity_for_symbol, symbol_for_continuity, mName, i);
		}
		name = ReadLong(stream);
	}
	// because older calchart files may omit the continuity index, need to check if it isn't used
	if (!has_type)
	{
		// when a point doesn't have a cont_index, it is assumed to be 0
		for (unsigned i = 0; i < pts.size(); i++)
		{
			CheckInconsistancy(pts[i].GetSymbol(), 0, continity_for_symbol, symbol_for_continuity, mName, i);
		}
	}
	// Point labels (left or right)
	while (INGL_LABL == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() != pts.size())
		{
			throw CC_FileException("Bad SYMB chunk");
		}
		uint8_t *d = &data[0];
		for (unsigned i = 0; i < pts.size(); i++)
		{
			if (*(d++))
			{
				pts.at(i).Flip();
			}
		}
		name = ReadLong(stream);
	}
	// Continuity text
	while (INGL_CONT == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() < 3)						  // one byte num + two nils minimum
		{
			throw CC_FileException("Bad cont chunk");
		}
		const char *d = (const char *)&data[0];
		if (d[data.size()-1] != '\0')
		{
			throw CC_FileException("Bad cont chunk");
		}
		
		const char* text = d + 1;
		size_t num = strlen(text);
		if (data.size() < num + 3)					  // check for room for text string
		{
			throw CC_FileException("Bad cont chunk");
		}
		std::string namestr(text);
		text = d + 2 + strlen(text);
		
		auto symbol_index = GetSymbolForName(namestr);
		if (symbol_index == MAX_NUM_SYMBOLS)
		{
			throw CC_FileException("No viable symbol for name");
		}
		if (continity_for_symbol.count(symbol_index))
		{
			// some point is using this symbol, check to see if it points to the same continuity
			if (continity_for_symbol[symbol_index] != (*d))
			{
				std::stringstream buf;
				buf<<"Error, continuity inconsistency on sheet "<<mName<<"\n";
				buf<<"Continuity index "<<(uint32_t)(*d)<<" is symbol "<<GetNameForSymbol(symbol_index)<<" but points using that symbol refer to continuity index "<<(uint32_t)continity_for_symbol[symbol_index]<<"\n";
				throw CC_FileException(buf.str());
			}
		}
		std::string textstr(text);
		GetContinuityBySymbol(symbol_index).SetText(textstr);
		
		name = ReadLong(stream);
	}
}

CC_sheet::CC_sheet(CC_show *show, size_t numPoints, std::istream& stream, Current_version_and_later) :
mAnimationContinuity(MAX_NUM_SYMBOLS),
pts(numPoints)
{
	// Read in sheet name
	// <INGL_NAME><size><string + 1>
	std::vector<uint8_t> data = ReadCheckIDandFillData(stream, INGL_NAME);
	mName = (const char*)&data[0];
	ReadAndCheckID(stream, INGL_END);
	ReadAndCheckID(stream, INGL_NAME);
	
	// read in the duration:
	// <INGL_DURA><4><duration>
	auto chunk = ReadCheckIDandSize(stream, INGL_DURA);
	beats = chunk;
	ReadAndCheckID(stream, INGL_END);
	ReadAndCheckID(stream, INGL_DURA);
	
	// Point positions
	ReadAndCheckID(stream, INGL_PNTS);
	auto pts_size = ReadLong(stream);
	for (auto i = 0; i < pts.size(); ++i)
	{
		auto size = ReadByte(stream);
		auto point_data = FillData(stream, size);
		pts[i] = CC_point(point_data);
	}
	ReadAndCheckID(stream, INGL_END);
	ReadAndCheckID(stream, INGL_PNTS);
	
	ReadAndCheckID(stream, INGL_CONT);
	uint32_t cont_size = ReadLong(stream);
	uint32_t name = ReadLong(stream);
	
	// Continuity text
	while (INGL_ECNT == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() < 2)						  // one byte num + 1 nil minimum
		{
			throw CC_FileException("Bad cont chunk");
		}
		const char *d = (const char *)&data[0];
		if (d[data.size()-1] != '\0')
		{
			throw CC_FileException("Bad cont chunk");
		}
		
		const char* text = d + 1;
		size_t num = strlen(text);
		if (data.size() < num + 2)					  // check for room for text string
		{
			throw CC_FileException("Bad cont chunk");
		}
		SYMBOL_TYPE symbol_index = static_cast<SYMBOL_TYPE>(*d);
		if (symbol_index >= MAX_NUM_SYMBOLS)
		{
			throw CC_FileException("No viable symbol for name");
		}
		
		std::string textstr(text);
		GetContinuityBySymbol(symbol_index).SetText(textstr);
		
		ReadAndCheckID(stream, INGL_END);
		ReadAndCheckID(stream, INGL_ECNT);
		name = ReadLong(stream);
	}
	if (INGL_END != name)
	{
		throw CC_FileException(INGL_CONT);
	}
	ReadAndCheckID(stream, INGL_CONT);

	name = ReadLong(stream);
	// Continuity text
	if (INGL_PCNT == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		const char *print_name = (const char *)&data[0];
		const char *print_cont = print_name + strlen(print_name) + 1;
		if ((strlen(print_name) + 1 + strlen(print_cont) + 1) != data.size())
		{
			throw CC_FileException("Bad Print cont chunk");
		}
		mPrintableContinuity = CC_print_continuity(print_name, print_cont);
		ReadAndCheckID(stream, INGL_END);
		ReadAndCheckID(stream, INGL_PCNT);
		name = ReadLong(stream);
	}
	if (INGL_END != name)
	{
		throw CC_FileException(INGL_END);
	}
	ReadAndCheckID(stream, INGL_SHET);
}

// SHEET              = INGL_SHET , BigEndianInt32(DataTill_SHEET_END) , SHEET_DATA , SHEET_END ;
// SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY, [ PRINT_CONTINUITY ] ;
// SHEET_END          = INGL_END , INGL_SHET ;
std::vector<uint8_t>
CC_sheet::SerializeAllPoints() const
{
	// ALL_POINTS         = INGL_PNTS , BigEndianInt32(DataTill_ALL_POINTS_END) , ALL_POINTS_DATA , ALL_POINTS_END ;
	// ALL_POINTS_DATA    = { EACH_POINT }* ;
	// ALL_POINTS_END     = INGL_END , INGL_PNTS ;
	// EACH_POINT         = INGL_PONT , BigEndianInt32(DataTill_EACH_POINT_END) , EACH_POINT_DATA , EACH_POINT_END ;
	// EACH_POINT_DATA    = { POSITION , [ REF_POSITION ] , [ POINT_SYMBOL ] , [ POINT_CONT_INDEX ] , [ POINT_LABEL_FLIP ] } ;
	// EACH_POINT_END     = INGL_END , INGL_PONT ;

	// for each of the points, serialize them and put them in the master list
	std::vector<uint8_t> all_points;
	for (const auto& i : pts)
	{
		std::vector<uint8_t> this_point = i.Serialize();
		all_points.insert(all_points.end(), this_point.begin(), this_point.end());
	}
	return all_points;
}

std::vector<uint8_t>
CC_sheet::SerializeContinuityData() const
{
	// CONTINUITY_DATA    = { EACH_CONTINUITY }* ;
	// EACH_CONTINUITY    = INGL_ECNT , BigEndianInt32(DataTill_EACH_CONTINUITY_END)) , EACH_CONTINUITY_DATA , EACH_CONTINUITY_END;
	// EACH_CONTINUITY_DATA = BigEndianInt8( symbol ) , Null-terminated char* ;
	// EACH_CONTINUITY_END  INGL_END , INGL_ECONT ;
	
	std::ostringstream stream("");
	for (auto current_symbol = SYMBOLS_START; current_symbol != MAX_NUM_SYMBOLS; ++current_symbol)
	{
		if (ContinuityInUse(current_symbol))
		{
			WriteChunkHeader(stream, INGL_ECNT,
						 1+GetContinuityBySymbol(current_symbol).GetText().length()+1);
			uint8_t tnum = static_cast<uint8_t>(current_symbol);
			Write(stream, &tnum, 1);
			WriteStr(stream, GetContinuityBySymbol(current_symbol).GetText().c_str());
			WriteEnd(stream, INGL_ECNT);
		}
	}

	auto sdata = stream.str();
	std::vector<uint8_t> data;
	std::copy(sdata.begin(), sdata.end(), std::back_inserter(data));
	return data;
}

std::vector<uint8_t>
CC_sheet::SerializePrintContinuityData() const
{
	// PRINT_CONTINUITY   = INGL_PCNT , BigEndianInt32(DataTill_PRINT_CONTINUITY_END)) , PRINT_CONTINUITY_DATA , PRINT_CONTINUITY_END;
	// PRINT_CONTINUITY_DATA = Null-terminated char* , Null-terminated char* ;
	// PRINT_CONTINUITY_END = INGL_END , INGL_PCNT ;
	
	std::ostringstream stream("");
	WriteStr(stream, mPrintableContinuity.GetPrintNumber().c_str());
	WriteStr(stream, mPrintableContinuity.GetOriginalLine().c_str());

	auto sdata = stream.str();
	std::vector<uint8_t> data;
	std::copy(sdata.begin(), sdata.end(), std::back_inserter(data));
	return data;
}

std::vector<uint8_t>
CC_sheet::SerializeSheetData() const
{
	// SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY, PRINT_CONTINUITY ;

	std::ostringstream stream("");
	// Write NAME
	WriteChunkStr(stream, INGL_NAME, GetName().c_str());
	WriteEnd(stream, INGL_NAME);
	
	// Write DURATION
	uint32_t id;
	put_big_long(&id, GetBeats());
	WriteChunk(stream, INGL_DURA, sizeof(id), &id);
	WriteEnd(stream, INGL_DURA);

	// Write ALL_POINTS
	auto all_points = SerializeAllPoints();
	WriteChunk(stream, INGL_PNTS, all_points.size(), &all_points[0]);
	WriteEnd(stream, INGL_PNTS);

	// Write Continuity
	auto all_continuity = SerializeContinuityData();
	WriteChunk(stream, INGL_CONT, all_continuity.size(), &all_continuity[0]);
	WriteEnd(stream, INGL_CONT);

	// Write Continuity
	auto print_continuity = SerializePrintContinuityData();
	WriteChunk(stream, INGL_PCNT, print_continuity.size(), &print_continuity[0]);
	WriteEnd(stream, INGL_PCNT);
	
	auto sdata = stream.str();
	std::vector<uint8_t> data;
	std::copy(sdata.begin(), sdata.end(), std::back_inserter(data));
	return data;
}

// SHEET              = INGL_SHET , BigEndianInt32(DataTill_SHEET_END) , SHEET_DATA , SHEET_END ;
// SHEET_DATA         = NAME , DURATION , ALL_POINTS , CONTINUITY, [ PRINT_CONTINUITY ] ;
// SHEET_END          = INGL_END , INGL_SHET ;
std::vector<uint8_t>
CC_sheet::SerializeSheet() const
{
	std::ostringstream stream("");
	auto sheet_data = SerializeSheetData();
	WriteChunk(stream, INGL_SHET, sheet_data.size(), &sheet_data[0]);
	WriteEnd(stream, INGL_SHET);
	
	auto sdata = stream.str();
	std::vector<uint8_t> data;
	std::copy(sdata.begin(), sdata.end(), std::back_inserter(data));
	return data;
}

CC_sheet::~CC_sheet()
{
}


// Find point at certain coords
int
CC_sheet::FindPoint(Coord x, Coord y, Coord searchBound, unsigned ref) const
{
	for (size_t i = 0; i < pts.size(); i++)
	{
		CC_coord c = GetPosition(i, ref);
		if (((x+searchBound) >= c.x) && ((x-searchBound) <= c.x) && ((y+searchBound) >= c.y) && ((y-searchBound) <= c.y))
		{
			return i;
		}
	}
	return -1;
}


std::set<unsigned>
CC_sheet::SelectPointsBySymbol(SYMBOL_TYPE i) const
{
	std::set<unsigned> select;
	for (size_t j = 0; j < pts.size(); j++)
	{
		if (pts.at(j).GetSymbol() == i)
		{
			select.insert(j);
		}
	}
	return select;
}


void CC_sheet::SetNumPoints(unsigned num, unsigned columns, const CC_coord& new_march_position)
{
	unsigned i, cpy, col;
	CC_coord c;

	std::vector<CC_point> newpts(num);
	cpy = std::min<unsigned>(pts.size(), num);
	for (i = 0; i < cpy; i++)
	{
		newpts[i] = pts[i];
	}
	for (c = new_march_position, col = 0; i < num; i++, col++, c.x += Int2Coord(2))
	{
		const CC_continuity& plaincont = GetContinuityBySymbol(SYMBOL_PLAIN);
		if (col >= columns)
		{
			c.x = new_march_position.x;
			c.y += Int2Coord(2);
			col = 0;
		}
		newpts[i] = CC_point(c);
	}
	pts = newpts;
}


void CC_sheet::RelabelSheet(const std::vector<size_t>& table)
{
	if (pts.size() != table.size())
	{
		throw std::runtime_error("wrong size for Relabel");
	}
	std::vector<CC_point> newpts(pts.size());
	for (size_t i = 0; i < newpts.size(); i++)
	{
		newpts[i] = pts[table[i]];
	}
	pts = newpts;
}


const CC_continuity& CC_sheet::GetContinuityBySymbol(SYMBOL_TYPE i) const
{
	return mAnimationContinuity.at(i);
}

CC_continuity& CC_sheet::GetContinuityBySymbol(SYMBOL_TYPE i)
{
	return mAnimationContinuity.at(i);
}


void CC_sheet::SetContinuityText(SYMBOL_TYPE which, const std::string& text)
{
	GetContinuityBySymbol(which).SetText(text);
}



bool CC_sheet::ContinuityInUse(SYMBOL_TYPE idx) const
{
	// is any point using this symbol?
	for (auto& point : pts)
	{
		if (point.GetSymbol() == idx)
		{
			return true;
		}
	}
	// otherwise, is the text set.
	return !GetContinuityBySymbol(idx).GetText().empty();
}


std::string CC_sheet::GetName() const
{
	return mName;
}

void CC_sheet::SetName(const std::string& newname)
{
	mName = newname;
}

std::string CC_sheet::GetNumber() const
{
	return mPrintableContinuity.GetPrintNumber();
}

std::string CC_sheet::GetRawPrintContinuity() const
{
	return mPrintableContinuity.GetOriginalLine();
}

unsigned short CC_sheet::GetBeats() const
{
	return beats;
}


void CC_sheet::SetBeats(unsigned short b)
{
	beats = b;
}


// Get position of point
CC_coord CC_sheet::GetPosition(unsigned i, unsigned ref) const
{
	return pts[i].GetPos(ref);
}


// Set position of point and all refs
void CC_sheet::SetAllPositions(const CC_coord& val, unsigned i)
{
	for (unsigned j = 0; j <= CC_point::kNumRefPoints; j++)
	{
		pts[i].SetPos(val, j);
	}
}


// Set position of point
void CC_sheet::SetPosition(const CC_coord& val, unsigned i, unsigned ref)
{
	unsigned j;
	if (ref == 0)
	{
		for (j = 1; j <= CC_point::kNumRefPoints; j++)
		{
			if (pts[i].GetPos(j) == pts[i].GetPos(0))
			{
				pts[i].SetPos(val, j);
			}
		}
		pts[i].SetPos(val);
	}
	else
	{
		pts[i].SetPos(val, ref);
	}
}

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

void
CC_sheet::SetPrintableContinuity(const std::string& name, const std::string& lines)
{
	mPrintableContinuity = CC_print_continuity(name, lines);
}

CC_textline_list
CC_sheet::GetPrintableContinuity() const
{
	return mPrintableContinuity.GetChunks();
}

const CC_point&
CC_sheet::GetPoint(unsigned i) const
{
	return pts[i];
}

CC_point&
CC_sheet::GetPoint(unsigned i)
{
	return pts[i];
}

std::vector<CC_point>
CC_sheet::GetPoints() const
{
	return pts;
}

void
CC_sheet::SetPoints(const std::vector<CC_point>& points)
{
	pts = points;
}

