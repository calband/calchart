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
#include "cc_command.h"
#include "modes.h"
#include "confgr.h"
#include "draw.h"
#include "cc_fileformat.h"
#include <boost/algorithm/string/predicate.hpp>
#include <sstream>
#include <iostream>

const std::string contnames[] =
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


CC_sheet::CC_sheet(CC_show *shw) :
show(shw),
beats(1),
pts(show->GetNumPoints())
{
}


CC_sheet::CC_sheet(CC_show *shw, const std::string& newname) :
show(shw),
beats(1),
pts(show->GetNumPoints()),
mName(newname)
{
}


CC_sheet::CC_sheet(CC_show *shw, size_t numPoints, std::istream& stream) :
show(shw),
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
	// Point continuity types
	while (INGL_TYPE == name)
	{
		std::vector<uint8_t> data = FillData(stream);
		if (data.size() != pts.size())
		{
			throw CC_FileException("Bad TYPE chunk");
		}
		uint8_t *d = &data[0];
		for (unsigned i = 0; i < pts.size(); i++)
		{
			pts.at(i).SetContinuityIndex(*(d++));
		}
		name = ReadLong(stream);
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
		CC_continuity newcont(namestr, *((uint8_t *)&data[0]));
		std::string textstr(text);
		newcont.SetText(textstr);
		animcont.push_back(newcont);
		
		name = ReadLong(stream);
	}
}

std::vector<uint8_t>
CC_sheet::WriteSheet() const
{
	std::ostringstream stream("");
	WriteGurk(stream, INGL_SHET);
	// Name
	WriteChunkStr(stream, INGL_NAME, GetName().c_str());
	// Beats
	uint32_t id;
	put_big_long(&id, GetBeats());
	WriteChunk(stream, INGL_DURA, 4, &id);
	
	// Point positions
	WriteChunkHeader(stream, INGL_POS, pts.size()*4);
	for (size_t i = 0; i < pts.size(); ++i)
	{
		Coord crd;
		put_big_word(&crd, GetPosition(i).x);
		Write(stream, &crd, 2);
		put_big_word(&crd, GetPosition(i).y);
		Write(stream, &crd, 2);
	}
	// Ref point positions
	for (size_t j = 1; j <= CC_point::kNumRefPoints; j++)
	{
		for (size_t i = 0; i < pts.size(); i++)
		{
			if (GetPosition(i) != GetPosition(i, j))
			{
				Coord crd;
				WriteChunkHeader(stream, INGL_REFP, pts.size()*4+2);
				put_big_word(&crd, j);
				Write(stream, &crd, 2);
				for (i = 0; i < pts.size(); i++)
				{
					put_big_word(&crd, GetPosition(i, j).x);
					Write(stream, &crd, 2);
					put_big_word(&crd, GetPosition(i, j).y);
					Write(stream, &crd, 2);
				}
				break;
			}
		}
	}
	// Point symbols
	for (size_t i = 0; i < pts.size(); ++i)
	{
		if (GetPoint(i).GetSymbol() != 0)
		{
			WriteChunkHeader(stream, INGL_SYMB, pts.size());
			for (i = 0; i < pts.size(); i++)
			{
				SYMBOL_TYPE tmp = GetPoint(i).GetSymbol();
				Write(stream, &tmp, 1);
			}
			break;
		}
	}
	// Point continuity types
	for (size_t i = 0; i < pts.size(); ++i)
	{
		if (GetPoint(i).GetContinuityIndex() != 0)
		{
			WriteChunkHeader(stream, INGL_TYPE, pts.size());
			for (i = 0; i < pts.size(); i++)
			{
				unsigned char tmp = GetPoint(i).GetContinuityIndex();
				Write(stream, &tmp, 1);
			}
			break;
		}
	}
	// Point labels (left or right)
	for (size_t i = 0; i < pts.size(); ++i)
	{
		if (GetPoint(i).GetFlip())
		{
			WriteChunkHeader(stream, INGL_LABL, pts.size());
			for (i = 0; i < pts.size(); i++)
			{
				char c = (GetPoint(i).GetFlip()) ? true : false;
				Write(stream, &c, 1);
			}
			break;
		}
	}
	// Continuity text
	for (CC_sheet::ContContainer::const_iterator curranimcont = animcont.begin(); curranimcont != animcont.end();
		 ++curranimcont)
	{
		WriteChunkHeader(stream, INGL_CONT,
						 1+curranimcont->GetName().length()+1+
						 curranimcont->GetText().length()+1);
		unsigned tnum = curranimcont->GetNum();
		Write(stream, &tnum, 1);
		WriteStr(stream, curranimcont->GetName().c_str());
		WriteStr(stream, curranimcont->GetText().c_str());
	}
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
int CC_sheet::FindPoint(Coord x, Coord y, unsigned ref) const
{
	Coord w = Float2Coord(GetConfiguration_DotRatio());
	for (size_t i = 0; i < pts.size(); i++)
	{
		CC_coord c = GetPosition(i, ref);
		if (((x+w) >= c.x) && ((x-w) <= c.x) && ((y+w) >= c.y) && ((y-w) <= c.y))
		{
			return i;
		}
	}
	return -1;
}


std::set<unsigned>
CC_sheet::SelectPointsOfContinuity(unsigned i) const
{
	std::set<unsigned> select;
	for (size_t j = 0; j < pts.size(); j++)
	{
		if (pts.at(j).GetContinuityIndex() == i)
		{
			select.insert(j);
		}
	}
	return select;
}


void CC_sheet::SetNumPoints(unsigned num, unsigned columns)
{
	unsigned i, cpy, col;
	CC_coord c, coff(show->GetMode().FieldOffset());

	std::vector<CC_point> newpts(num);
	cpy = std::min<unsigned>(show->GetNumPoints(), num);
	for (i = 0; i < cpy; i++)
	{
		newpts[i] = pts[i];
	}
	for (c = coff, col = 0; i < num; i++, col++, c.x += Int2Coord(2))
	{
		const CC_continuity& plaincont = GetStandardContinuity(SYMBOL_PLAIN);
		if (col >= columns)
		{
			c.x = coff.x;
			c.y += Int2Coord(2);
			col = 0;
		}
		newpts[i] = CC_point(plaincont.GetNum(), c);
	}
	pts = newpts;
}


void CC_sheet::RelabelSheet(unsigned *table)
{
	std::vector<CC_point> newpts(show->GetNumPoints());
	for (size_t i = 0; i < newpts.size(); i++)
	{
		newpts[i] = pts[table[i]];
	}
	pts = newpts;
}


const CC_continuity& CC_sheet::GetNthContinuity(unsigned i) const
{
	return animcont.at(i);
}


void CC_sheet::SetNthContinuity(const std::string& text, unsigned i)
{
	animcont.at(i).SetText(text);
}


CC_continuity CC_sheet::RemoveNthContinuity(unsigned i)
{
	CC_continuity cont = animcont.at(i);
	animcont.erase(animcont.begin()+i);
	return cont;
}


void CC_sheet::InsertContinuity(const CC_continuity& newcont, unsigned i)
{
	animcont.insert(animcont.begin() + i, newcont);
}


void CC_sheet::AppendContinuity(const CC_continuity& newcont)
{
	animcont.push_back(newcont);
}


unsigned CC_sheet::NextUnusedContinuityNum()
{
	unsigned i = 0;
	bool found;

	do
	{
		found = false;
		for (ContContainer::const_iterator c = animcont.begin(); c != animcont.end(); ++c)
		{
			if (c->GetNum() == i)
			{
				found = true;
				i++;
				break;
			}
		}
	} while (found);
	return i;
}


// not undoable
const CC_continuity& CC_sheet::GetStandardContinuity(SYMBOL_TYPE sym)
{

	for (ContContainer::const_iterator c = animcont.begin(); c != animcont.end(); ++c)
	{
		if (boost::iequals(c->GetName(), contnames[sym]))
		{
			return *c;
		}
	}

	unsigned i,idx;
	i = (unsigned)sym;
	idx = 0;
// Put in correct postion
	while (i > 0)
	{
		idx = FindContinuityByName(contnames[--i]);
		if (idx != 0) break;
	}
	InsertContinuity(CC_continuity(contnames[sym], NextUnusedContinuityNum()), idx);
	return GetNthContinuity(idx);
}


unsigned CC_sheet::FindContinuityByName(const std::string& name) const
{
	unsigned idx;
	ContContainer::const_iterator c = animcont.begin();

	for (idx = 1; c != animcont.end(); idx++, ++c)
	{
		if (boost::iequals(c->GetName(), mName))
		{
			break;
		}
	}
	if (c == animcont.end())
	{
		idx = 0;
	}
	return idx;
}


bool CC_sheet::ContinuityInUse(unsigned idx) const
{
	unsigned i;
	const CC_continuity& c = GetNthContinuity(idx);

	for (i = 0; i < pts.size(); i++)
	{
		if (pts[i].GetContinuityIndex() == c.GetNum()) return true;
	}
	return false;
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
	return number;
}

void CC_sheet::SetNumber(const std::string& num)
{
	number = num;
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
	CC_coord clippedval = show->GetMode().ClipPosition(val);
	if (ref == 0)
	{
		for (j = 1; j <= CC_point::kNumRefPoints; j++)
		{
			if (pts[i].GetPos(j) == pts[i].GetPos(0))
			{
				pts[i].SetPos(clippedval, j);
			}
		}
		pts[i].SetPos(clippedval);
	}
	else
	{
		pts[i].SetPos(clippedval, ref);
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

bool
CC_sheet::ImportContinuity(const std::vector<std::string>& line_data)
{
	enum PSFONT_TYPE currfontnum;
	currfontnum = PSFONT_NORM;
	for (auto line_iter = line_data.begin(); line_iter != line_data.end(); ++line_iter)
	{
		if ((line_iter->length() >= 2) && (line_iter->at(0) == '%') && (line_iter->at(1) == '%'))
		{
			SetNumber(std::string(*line_iter, 2));
			continue;
		}
		// make a copy of the line
		std::string line = *line_iter;
		CC_textline line_text;
		// peel off the '<>~'
		if (!line.empty() && line.at(0) == '<')
		{
			line_text.on_sheet = false;
			line.erase(0, 1);
		}
		if (!line.empty() && line.at(0) == '>')
		{
			line_text.on_main = false;
			line.erase(0, 1);
		}
		if (!line.empty() && line.at(0) == '~')
		{
			line_text.center = true;
			line.erase(0, 1);
		}
		// break the line into substrings
		while (!line.empty())
		{
			// first take care of any tabs
			if (line.at(0) == '\t')
			{
				if (line.length() > 1)
				{
					CC_textchunk new_text;
					new_text.font = PSFONT_TAB;
					line_text.chunks.push_back(new_text);
				}
				line.erase(0, 1);
				continue;
			}
			// now check to see if we have any special person marks
			if ((line.length() >= 3) && (line.at(0) == '\\') && ((tolower(line.at(1)) == 'p') || (tolower(line.at(1)) == 's')))
			{
				CC_textchunk new_text;
				new_text.font = PSFONT_SYMBOL;
				if (tolower(line.at(1)) == 'p')
				{
					switch (tolower(line.at(2)))
					{
						case 'o':
							new_text.text.push_back('A'); break;
						case 'b':
							new_text.text.push_back('C'); break;
						case 's':
							new_text.text.push_back('D'); break;
						case 'x':
							new_text.text.push_back('E'); break;
						default:
							// code not recognized
							return false;
					}
				}
				if (tolower(line.at(1)) == 's')
				{
					switch (tolower(line.at(2)))
					{
						case 'o':
							new_text.text.push_back('B'); break;
						case 'b':
							new_text.text.push_back('F'); break;
						case 's':
							new_text.text.push_back('G'); break;
						case 'x':
							new_text.text.push_back('H'); break;
						default:
							// code not recognized
							return false;
					}
				}
				line_text.chunks.push_back(new_text);
				line.erase(0, 3);
				continue;
			}
			// now check to see if we have any font
			if ((line.length() >= 3) && (line.at(0) == '\\') && ((tolower(line.at(1)) == 'b') || (tolower(line.at(1)) == 'i')))
			{
				if (tolower(line.at(2)) == 'e')
				{
					currfontnum = PSFONT_NORM;
				}
				if (tolower(line.at(2)) == 's')
				{
					currfontnum = (tolower(line.at(1)) == 'b') ? PSFONT_BOLD : PSFONT_ITAL;
				}
				line.erase(0, 3);
				continue;
			}
			int pos = line.find_first_of("\\\t", 1);

			CC_textchunk new_text;
			new_text.font = currfontnum;
			new_text.text = line.substr(0, pos);
			line_text.chunks.push_back(new_text);
			line.erase(0, pos);
		}
		continuity.push_back(line_text);
	}
	return true;
}

