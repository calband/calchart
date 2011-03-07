/* show.cpp
 * Member functions for show classes
 *
 * Modification history:
 * 4-16-95    Garrick Meeker              Created from previous CalPrint
 * 7-28-95    Garrick Meeker              Added continuity parser from
 *                                           previous CalPrint
 *
 */

/*
   Copyright (C) 1994-2008  Garrick Brian Meeker

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

#include <fstream>

#include "cc_show.h"

#include "undo.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "ingl.h"

static const wxChar *nomem_str = wxT("Out of memory!");
static const wxChar *nofile_str = wxT("Unable to open file");
static const wxChar *badcont_str = wxT("Error in continuity file");
static const wxChar *contnohead_str = wxT("Continuity file doesn't begin with header");
static const wxChar *nosheets_str = wxT("No sheets found");
static const wxChar *writeerr_str = wxT("Write error: check disk media");

const wxChar *contnames[] =
{
	wxT("Plain"),
	wxT("Sol"),
	wxT("Bksl"),
	wxT("Sl"),
	wxT("X"),
	wxT("Solbksl"),
	wxT("Solsl"),
	wxT("Solx")
};

class AutoSaveTimer: public wxTimer
{
	public:
		AutoSaveTimer(): untitled_number(1) {}
		void Notify()
		{
			for (std::list<CC_show*>::iterator i = showlist.begin(); i != showlist.end(); ++i)
			{
				CC_show *show = *i;
				if (show->Modified())
				{
					wxString s;
					if (!(s=show->Autosave()).empty())
					{
						(void)wxMessageBox(s, wxT("Autosave Error"));
					}
				}
			}
		}

		inline void AddShow(CC_show *show)
		{
			showlist.push_back(show);
		}
		inline void RemoveShow(CC_show *show)
		{
			showlist.remove(show);
		}
		inline int GetNumber() { return untitled_number++; }
	private:
		std::list<CC_show*> showlist;
		int untitled_number;
};

static AutoSaveTimer autosaveTimer;

void SetAutoSave(int secs)
{
	if (secs > 0)
		autosaveTimer.Start(secs*1000);
}

IMPLEMENT_DYNAMIC_CLASS(CC_show, wxDocument);

// Create a new show
CC_show::CC_show(unsigned npoints)
:okay(true), numpoints(npoints), numsheets(1), sheets(new CC_sheet(this, wxT("1"))),
pt_labels(npoints),
modified(false), print_landscape(false), print_do_cont(true),
print_do_cont_sheet(true)
{
	wxString tmpname;

	tmpname.Printf(wxT("noname%d.shw"), autosaveTimer.GetNumber());
	SetAutosaveName(tmpname);
	undolist = new ShowUndoList(this, undo_buffer_size);
	mode = *gTheApp->GetModeList().Begin();
	if (npoints)
	{
		for (unsigned int i = 0; i < npoints; i++)
		{
			pt_labels[i].Printf(wxT("%u"), i);
		}
		selections = new bool[npoints];
		if (selections == NULL)
		{
// Out of mem!
			AddError(nomem_str);
			return;
		}
		UnselectAll();
	}
	else
	{
		selections = NULL;
	}
	sheets->animcont.push_back(CC_continuity_ptr(new CC_continuity(contnames[0], 0)));

	autosaveTimer.AddShow(this);
}

#define Make4CharWord(a,b,c,d) (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#define INGL_INGL Make4CharWord('I','N','G','L')
#define INGL_GURK Make4CharWord('G','U','R','K')
#define INGL_SHOW Make4CharWord('S','H','O','W')
#define INGL_SHET Make4CharWord('S','H','E','T')
#define INGL_SIZE Make4CharWord('S','I','Z','E')
#define INGL_LABL Make4CharWord('L','A','B','L')
#define INGL_MODE Make4CharWord('M','O','D','E')
#define INGL_DESC Make4CharWord('D','E','S','C')
#define INGL_NAME Make4CharWord('N','A','M','E')
#define INGL_DURA Make4CharWord('D','U','R','A')
#define INGL_POS  Make4CharWord('P','O','S',' ')
#define INGL_SYMB Make4CharWord('S','Y','M','B')
#define INGL_TYPE Make4CharWord('T','Y','P','E')
#define INGL_REFP Make4CharWord('R','E','F','P')
#define INGL_CONT Make4CharWord('C','O','N','T')
#define INGL_PCNT Make4CharWord('P','C','N','T')
#define INGL_END  Make4CharWord('E','N','D',' ')

enum CONT_PARSE_MODE
{
	CONT_PARSE_NORMAL,
	CONT_PARSE_TAB,
	CONT_PARSE_BS,
	CONT_PARSE_PLAIN,
	CONT_PARSE_SOLID,
	CONT_PARSE_BOLD,
	CONT_PARSE_ITALIC
};

void PeekLong(wxInputStream& stream, uint32_t& d)
{
	uint8_t rawd[4];
	stream.Read(rawd, sizeof(rawd));
	d = get_big_long(&rawd);
	stream.Ungetch(rawd, sizeof(rawd));
}

void ReadLong(wxInputStream& stream, uint32_t& d)
{
	uint8_t rawd[4];
	stream.Read(rawd, sizeof(rawd));
	d = get_big_long(&rawd);
}

// return false if you don't read the inname
void ReadAndCheckID(wxInputStream& stream, uint32_t inname)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw INGL_exception(inname);
	}
}

// return false if you don't read the inname
void ReadCheckIDandSize(wxInputStream& stream, uint32_t inname, uint32_t& size)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw INGL_exception(inname);
	}
	ReadLong(stream, name);
	if (4 != name)
	{
		uint8_t rawd[4];
		put_big_long(rawd, inname);
		wxString s; s.Printf(wxT("Wrong size %d for name %c%c%c%c"), name, rawd[0], rawd[1], rawd[2], rawd[3]);
		throw INGL_exception(s);
	}
	ReadLong(stream, size);
}

// return false if you don't read the inname
void ReadCheckIDandFillData(wxInputStream& stream, uint32_t inname, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw INGL_exception(inname);
	}
	ReadLong(stream, name);
	data.resize(name);
	stream.Read(&data[0], name);
}

void WriteLong(wxOutputStream& stream, uint32_t d)
{
	uint8_t rawd[4];
	put_big_long(rawd, d);
	stream.Write(rawd, sizeof(rawd));
}

void WriteHeader(wxOutputStream& stream)
{
	WriteLong(stream, INGL_INGL);
}


void WriteGurk(wxOutputStream& stream, uint32_t name)
{
	WriteLong(stream, INGL_GURK);
	WriteLong(stream, name);
}


void WriteChunkHeader(wxOutputStream& stream, uint32_t name, uint32_t size)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
}


void WriteChunk(wxOutputStream& stream, uint32_t name, uint32_t size, const void *data)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
	if (size > 0)
		stream.Write(data, size);
}


void WriteChunkStr(wxOutputStream& stream, uint32_t name, const char *str)
{
	WriteChunk(stream, name, strlen(str)+1, (unsigned char *)str);
}


void WriteEnd(wxOutputStream& stream, uint32_t name)
{
	WriteLong(stream, INGL_END);
	WriteLong(stream, name);
}


void WriteStr(wxOutputStream& stream, const char *str)
{
	stream.Write(str, strlen(str)+1);
}


void Write(wxOutputStream& stream, const void *data, uint32_t size)
{
	stream.Write(data, size);
}


// Destroy a show
CC_show::~CC_show()
{
	CC_sheet *tmp;

	autosaveTimer.RemoveShow(this);
#if 0
	fprintf(stderr, "Deleting show...\n");
#endif
	if (undolist) delete undolist;
	while (sheets)
	{
		tmp = sheets->next;
		delete sheets;
		sheets = tmp;
	}
}


wxString CC_show::ImportContinuity(const wxString& file)
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
	FILE *fp;
	CC_sheet *curr_sheet;
	wxString tempbuf;
	CC_textline *line_text;
	unsigned pos;
	bool on_sheet, on_main, center, font_changed;
	enum CONT_PARSE_MODE parsemode;
	enum PSFONT_TYPE currfontnum, lastfontnum;
	wxString lineotext;
	char c;
	bool sheetmark;

	fp = CC_fopen(file.fn_str(), "r");
	if (fp)
	{
		curr_sheet = NULL;
		currfontnum = lastfontnum = PSFONT_NORM;
		line_text = NULL;
		while (true)
		{
			if (feof(fp)) break;
			ReadDOSline(fp, tempbuf);
			sheetmark = false;
			line_text = NULL;
			if (tempbuf.Length() >= 2)
			{
				if ((tempbuf.GetChar(0) == '%') && (tempbuf.GetChar(1) == '%'))
				{
					sheetmark = true;
				}
			}
			if (sheetmark)
			{
				if (curr_sheet) curr_sheet = curr_sheet->next;
				else curr_sheet = sheets;
				if (!curr_sheet) break;
				if (tempbuf.Length() > 2)
				{
					curr_sheet->SetNumber(tempbuf.Mid(2));
				}
			}
			else
			{
				if (curr_sheet == NULL)
				{
// Continuity doesn't begin with a sheet header
					return wxString(contnohead_str);
				}
				on_main = true;
				on_sheet = true;
				pos = 0;
				if (pos < tempbuf.Length())
				{
					if (tempbuf.GetChar(pos) == '<')
					{
						on_sheet = false;
						pos++;
					}
					else
					{
						if (tempbuf.GetChar(pos) == '>')
						{
							on_main = false;
							pos++;
						}
					}
				}
				center = false;
				if (pos < tempbuf.Length())
				{
					if (tempbuf.GetChar(pos) == '~')
					{
						center = true;
						pos++;
					}
				}
				parsemode = CONT_PARSE_NORMAL;
				do
				{
					font_changed = false;
					lineotext = wxT("");
					while ((pos < tempbuf.Length()) && !font_changed)
					{
						switch (parsemode)
						{
							case CONT_PARSE_NORMAL:
								c = tempbuf.GetChar(pos++);
								switch (c)
								{
									case '\\':
										parsemode = CONT_PARSE_BS;
										break;
									case '\t':
										parsemode = CONT_PARSE_TAB;
										font_changed = true;
										break;
									default:
										lineotext.Append(c);
										break;
								}
								break;
							case CONT_PARSE_TAB:
								parsemode = CONT_PARSE_NORMAL;
								currfontnum = PSFONT_TAB;
								font_changed = true;
								break;
							case CONT_PARSE_BS:
								c = tolower(tempbuf.GetChar(pos++));
								switch (c)
								{
									case 'p':
										parsemode = CONT_PARSE_PLAIN;
										font_changed = true;
										break;
									case 's':
										parsemode = CONT_PARSE_SOLID;
										font_changed = true;
										break;
									case 'b':
										parsemode = CONT_PARSE_BOLD;
										break;
									case 'i':
										parsemode = CONT_PARSE_ITALIC;
										break;
									default:
										parsemode = CONT_PARSE_NORMAL;
										lineotext.Append(c);
										break;
								}
								break;
							case CONT_PARSE_PLAIN:
								parsemode = CONT_PARSE_NORMAL;
								font_changed = true;
								currfontnum = PSFONT_SYMBOL;
								c = tolower(tempbuf.GetChar(pos++));
								switch (c)
								{
									case 'o':
										lineotext.Append('A');
										break;
									case 'b':
										lineotext.Append('C');
										break;
									case 's':
										lineotext.Append('D');
										break;
									case 'x':
										lineotext.Append('E');
										break;
									default:
// code not recognized
										return wxString(badcont_str);
								}
								break;
							case CONT_PARSE_SOLID:
								parsemode = CONT_PARSE_NORMAL;
								font_changed = true;
								currfontnum = PSFONT_SYMBOL;
								c = tolower(tempbuf.GetChar(pos++));
								switch (c)
								{
									case 'o':
										lineotext.Append('B');
										break;
									case 'b':
										lineotext.Append('F');
										break;
									case 's':
										lineotext.Append('G');
										break;
									case 'x':
										lineotext.Append('H');
										break;
									default:
// code not recognized
										return wxString(badcont_str);
								}
								break;
							case CONT_PARSE_BOLD:
								parsemode = CONT_PARSE_NORMAL;
								c = tolower(tempbuf.GetChar(pos++));
								switch (c)
								{
									case 's':
										switch (currfontnum)
										{
											case PSFONT_NORM:
												lastfontnum = PSFONT_BOLD;
												font_changed = true;
												break;
											case PSFONT_ITAL:
												lastfontnum = PSFONT_BOLDITAL;
												font_changed = true;
												break;
											default:
												break;
										}
										break;
									case 'e':
										switch (currfontnum)
										{
											case PSFONT_BOLD:
												lastfontnum = PSFONT_NORM;
												font_changed = true;
												break;
											case PSFONT_BOLDITAL:
												lastfontnum = PSFONT_ITAL;
												font_changed = true;
												break;
											default:
												break;
										}
										break;
									default:
// code not recognized
										return wxString(badcont_str);
								}
								break;
							case CONT_PARSE_ITALIC:
								parsemode = CONT_PARSE_NORMAL;
								c = tolower(tempbuf.GetChar(pos++));
								switch (c)
								{
									case 's':
										switch (currfontnum)
										{
											case PSFONT_NORM:
												lastfontnum = PSFONT_ITAL;
												font_changed = true;
												break;
											case PSFONT_BOLD:
												lastfontnum = PSFONT_BOLDITAL;
												font_changed = true;
												break;
											default:
												break;
										}
										break;
									case 'e':
										switch (currfontnum)
										{
											case PSFONT_ITAL:
												lastfontnum = PSFONT_NORM;
												font_changed = true;
												break;
											case PSFONT_BOLDITAL:
												lastfontnum = PSFONT_BOLD;
												font_changed = true;
												break;
											default:
												break;
										}
										break;
									default:
// code not recognized
										return wxString(badcont_str);
								}
								break;
						}
					}
// Add any remaining text
// Empty text only okay if line is blank
					if ((!lineotext.empty()) ||
						(currfontnum == PSFONT_TAB) ||
// Empty line
						((pos >= tempbuf.length()) && (line_text == NULL)))
					{
						CC_textchunk new_text;
						if (line_text == NULL)
						{
							curr_sheet->continuity.push_back(CC_textline());
							line_text = &curr_sheet->continuity.back();
							line_text->on_main = on_main;
							line_text->on_sheet = on_sheet;
							line_text->center = center;
						}
						new_text.font = currfontnum;
						new_text.text = lineotext;
						line_text->chunks.push_back(new_text);
					}
// restore to previous font (used for symbols and tabs)
					currfontnum = lastfontnum;
				} while (pos < tempbuf.Length());
			}
		}
		fclose(fp);
	}
	else
	{
		return wxString(nofile_str);
	}
	return wxT("");
}


void CC_show::Append(CC_show *shw)
{
	CC_sheet *sht;

	if (numpoints == shw->GetNumPoints())
	{
		if (sheets == NULL)
		{
			sheets = shw->sheets;
		}
		else
		{
			for (sht = sheets; sht->next != NULL; sht = sht->next);
			sht->next = shw->sheets;
		}
		numsheets += shw->numsheets;
		for (sht = shw->sheets; sht != NULL; sht = sht->next)
		{
			sht->show = this;
		}
		shw->sheets = NULL;
		shw->numsheets = 0;
		delete shw;
		gTheApp->GetWindowList().AppendSheets();
	}
}


void CC_show::Append(CC_sheet *newsheets)
{
	CC_sheet *sht;

	if (sheets == NULL)
	{
		sheets = newsheets;
	}
	else
	{
		for (sht = sheets; sht->next != NULL; sht = sht->next);
		sht->next = newsheets;
	}
	for (sht = newsheets; sht != NULL; sht = sht->next)
	{
		numsheets++;
	}
	gTheApp->GetWindowList().AppendSheets();
}


wxOutputStream& CC_show::SaveObject(wxOutputStream& stream)
{
	uint32_t id;
	unsigned i, j;
	Coord crd;
	unsigned char c;

	FlushAllTextWindows();

	WriteHeader(stream);
	WriteGurk(stream, INGL_SHOW);

// Handle show info
	i = GetNumPoints();
	put_big_long(&id, i);
	WriteChunk(stream, INGL_SIZE, 4, &id);

	id = 0;
	for (i = 0; i < GetNumPoints(); i++)
	{
		id += strlen(GetPointLabel(i).utf8_str())+1;
	}
	if (id > 0)
	{
		WriteChunkHeader(stream, INGL_LABL, id);
		for (i = 0; i < GetNumPoints(); i++)
		{
			WriteStr(stream, GetPointLabel(i).utf8_str());
		}
	}

//handl->WriteChunkStr(INGL_MODE, mode->Name());

// Description
	if (!UserGetDescr().empty())
	{
		WriteChunkStr(stream, INGL_DESC, UserGetDescr().utf8_str());
	}

// Handle sheets
	for (const CC_sheet *curr_sheet = GetSheet();
		curr_sheet != NULL;
		curr_sheet = curr_sheet->next)
	{
		WriteGurk(stream, INGL_SHET);
// Name
		WriteChunkStr(stream, INGL_NAME, curr_sheet->GetName().utf8_str());
// Beats
		put_big_long(&id, curr_sheet->GetBeats());
		WriteChunk(stream, INGL_DURA, 4, &id);

// Point positions
		WriteChunkHeader(stream, INGL_POS, GetNumPoints()*4);
		for (i = 0; i < GetNumPoints(); i++)
		{
			put_big_word(&crd, curr_sheet->GetPosition(i).x);
			Write(stream, &crd, 2);
			put_big_word(&crd, curr_sheet->GetPosition(i).y);
			Write(stream, &crd, 2);
		}
// Ref point positions
		for (j = 1; j <= NUM_REF_PNTS; j++)
		{
			for (i = 0; i < GetNumPoints(); i++)
			{
				if (curr_sheet->GetPosition(i) != curr_sheet->GetPosition(i, j))
				{
					WriteChunkHeader(stream, INGL_REFP, GetNumPoints()*4+2);
					put_big_word(&crd, j);
					Write(stream, &crd, 2);
					for (i = 0; i < GetNumPoints(); i++)
					{
						put_big_word(&crd, curr_sheet->GetPosition(i, j).x);
						Write(stream, &crd, 2);
						put_big_word(&crd, curr_sheet->GetPosition(i, j).y);
						Write(stream, &crd, 2);
					}
					break;
				}
			}
		}
// Point symbols
		for (i = 0; i < GetNumPoints(); i++)
		{
			if (curr_sheet->GetPoint(i).sym != 0)
			{
				WriteChunkHeader(stream, INGL_SYMB, GetNumPoints());
				for (i = 0; i < GetNumPoints(); i++)
				{
					Write(stream, &curr_sheet->GetPoint(i).sym, 1);
				}
				break;
			}
		}
// Point continuity types
		for (i = 0; i < GetNumPoints(); i++)
		{
			if (curr_sheet->GetPoint(i).cont != 0)
			{
				WriteChunkHeader(stream, INGL_TYPE, GetNumPoints());
				for (i = 0; i < GetNumPoints(); i++)
				{
					Write(stream, &curr_sheet->GetPoint(i).cont, 1);
				}
				break;
			}
		}
// Point labels (left or right)
		for (i = 0; i < GetNumPoints(); i++)
		{
			if (curr_sheet->GetPoint(i).GetFlip())
			{
				WriteChunkHeader(stream, INGL_LABL, GetNumPoints());
				for (i = 0; i < GetNumPoints(); i++)
				{
					if (curr_sheet->GetPoint(i).GetFlip())
					{
						c = true;
					}
					else
					{
						c = false;
					}
					Write(stream, &c, 1);
				}
				break;
			}
		}
// Continuity text
		for (CC_sheet::ContContainer::const_iterator curranimcont = curr_sheet->animcont.begin(); curranimcont != curr_sheet->animcont.end();
			++curranimcont)
		{
			WriteChunkHeader(stream, INGL_CONT,
				1+(*curranimcont)->GetName().Length()+1+
				(*curranimcont)->GetText().Length()+1);
			unsigned tnum = (*curranimcont)->GetNum();
			Write(stream, &tnum, 1);
			WriteStr(stream, (*curranimcont)->GetName().utf8_str());
			WriteStr(stream, (*curranimcont)->GetText().utf8_str());
		}
		WriteEnd(stream, INGL_SHET);
	}

	WriteEnd(stream, INGL_SHOW);
	
	// TODO: if an error occurred, notify user here:

	return stream;
}

wxInputStream& CC_show::LoadObject(wxInputStream& stream)
{
	uint32_t name;
	std::vector<uint8_t> data;

	try
	{
	ReadAndCheckID(stream, INGL_INGL);
	ReadAndCheckID(stream, INGL_GURK);
	ReadAndCheckID(stream, INGL_SHOW);

// Handle show info
	// read in the size:
	// <INGL_SIZE><4><# points>
	ReadCheckIDandSize(stream, INGL_SIZE, name);
	SetNumPointsInternal(name);

	PeekLong(stream, name);
	// Optional: read in the point labels
	// <INGL_LABL><SIZE>
	if (INGL_LABL == name)
	{
		ReadCheckIDandFillData(stream, INGL_LABL, data);
		const char *str = (const char*)&data[0];
		for (unsigned i = 0; i < GetNumPoints(); i++)
		{
			GetPointLabel(i) = wxString::FromUTF8(str);
			str += strlen(str)+1;
		}
		// peek for the next name
		PeekLong(stream, name);
	}

	// Optional: read in the point labels
	// <INGL_DESC><SIZE>
	if (INGL_DESC == name)
	{
		ReadCheckIDandFillData(stream, INGL_LABL, data);
		wxString s(wxString::FromUTF8((const char*)&data[0]));
		SetDescr(s);
		// peek for the next name
		PeekLong(stream, name);
	}

	// Read in sheets
	// <INGL_GURK><INGL_SHET>
	while (INGL_GURK == name)
	{
		ReadAndCheckID(stream, INGL_GURK);
		ReadAndCheckID(stream, INGL_SHET);

		CC_sheet *sheet = new CC_sheet(this);
		InsertSheetInternal(sheet, GetNumSheets());

		// Read in sheet name
		// <INGL_NAME><size><string + 1>
		ReadCheckIDandFillData(stream, INGL_NAME, data);
		sheet->SetName(wxString::FromUTF8((const char*)&data[0]));

		// read in the duration:
		// <INGL_DURA><4><duration>
		ReadCheckIDandSize(stream, INGL_DURA, name);
		sheet->SetBeats(name);

		// Point positions
		// <INGL_DURA><size><data>
		ReadCheckIDandFillData(stream, INGL_POS, data);
		if (data.size() != size_t(sheet->show->GetNumPoints()*4))
		{
			throw INGL_exception(wxT("bad POS chunk"));
		}
		{
			uint8_t *d;
			d = (uint8_t*)&data[0];
			for (unsigned i = 0; i < sheet->show->GetNumPoints(); ++i)
			{
				CC_coord c;
				c.x = get_big_word(d);
				d += 2;
				c.y = get_big_word(d);
				d += 2;
				sheet->SetAllPositions(c, i);
			}
		}

		PeekLong(stream, name);
		// read all the reference points
		while (INGL_REFP == name)
		{
			ReadCheckIDandFillData(stream, INGL_REFP, data);
			if (data.size() != (unsigned long)sheet->show->GetNumPoints()*4+2)
			{
				throw INGL_exception(wxT("Bad REFP chunk"));
			}
			uint8_t *d = (uint8_t*)&data[0];
			unsigned ref = get_big_word(d);
			d += 2;
			for (unsigned i = 0; i < sheet->show->GetNumPoints(); i++)
			{
				CC_coord c;
				c.x = get_big_word(d);
				d += 2;
				c.y = get_big_word(d);
				d += 2;
				sheet->SetPositionQuick(c, i, ref);		  // don't clip
			}
			PeekLong(stream, name);
		}
		// Point symbols
		while (INGL_SYMB == name)
		{
			ReadCheckIDandFillData(stream, INGL_SYMB, data);
			if (data.size() != (unsigned long)sheet->show->GetNumPoints())
			{
				throw INGL_exception(wxT("Bad SYMB chunk"));
			}
			uint8_t *d = (uint8_t *)&data[0];
			for (unsigned i = 0; i < sheet->show->GetNumPoints(); i++)
			{
				sheet->GetPoint(i).sym = (SYMBOL_TYPE)(*(d++));
			}
			PeekLong(stream, name);
		}
		// Point continuity types
		while (INGL_TYPE == name)
		{
			ReadCheckIDandFillData(stream, INGL_TYPE, data);
			if (data.size() != (unsigned long)sheet->show->GetNumPoints())
			{
				throw INGL_exception(wxT("Bad TYPE chunk"));
			}
			uint8_t *d = (uint8_t *)&data[0];
			for (unsigned i = 0; i < sheet->show->GetNumPoints(); i++)
			{
				sheet->GetPoint(i).cont = *(d++);
			}
			PeekLong(stream, name);
		}
		// Point labels (left or right)
		while (INGL_LABL == name)
		{
			ReadCheckIDandFillData(stream, INGL_LABL, data);
			if (data.size() != (unsigned long)sheet->show->GetNumPoints())
			{
				throw INGL_exception(wxT("Bad SYMB chunk"));
			}
			uint8_t *d = (uint8_t *)&data[0];
			for (unsigned i = 0; i < sheet->show->GetNumPoints(); i++)
			{
				if (*(d++))
				{
					sheet->GetPoint(i).Flip();
				}
			}
			PeekLong(stream, name);
		}
		// Continuity text
		while (INGL_CONT == name)
		{
			ReadCheckIDandFillData(stream, INGL_CONT, data);
			if (data.size() < 3)						  // one byte num + two nils minimum
			{
				throw INGL_exception(wxT("Bad cont chunk"));
			}
			const char *d = (const char *)&data[0];
			if (d[data.size()-1] != '\0')
			{
				throw INGL_exception(wxT("Bad cont chunk"));
			}

			const char* text = d + 1;
			size_t num = strlen(text);
			if (data.size() < num + 3)					  // check for room for text string
			{
				throw INGL_exception(wxT("Bad cont chunk"));
			}
			wxString namestr(wxString::FromUTF8(text));
			text = d + 2 + strlen(text);
			CC_continuity_ptr newcont(new CC_continuity(namestr, *((uint8_t *)&data[0])));
			wxString textstr(wxString::FromUTF8(text));
			newcont->SetText(textstr);
			sheet->AppendContinuity(newcont);

			PeekLong(stream, name);
		}
		ReadAndCheckID(stream, INGL_END);
		ReadAndCheckID(stream, INGL_SHET);
		// peek for the next name
		PeekLong(stream, name);
	}
	ReadAndCheckID(stream, INGL_END);
	ReadAndCheckID(stream, INGL_SHOW);
	}
	catch (INGL_exception& e) {
		AddError(e.WhatError());
	}
	return stream;
}

void CC_show::ClearAutosave() const
{
	wxRemoveFile(autosave_name);
}


void CC_show::FlushAllTextWindows() const
{
	gTheApp->GetWindowList().FlushDescr(); 
	gTheApp->GetWindowList().FlushContinuity();
}


void CC_show::UserSetName(const wxString& newname)
{
	SetName(newname);
	gTheApp->GetWindowList().ChangeName();
}


const wxString& CC_show::UserGetDescr() const
{
	gTheApp->GetWindowList().FlushDescr();
	return descr;
}


void CC_show::SetModified(bool b)
{
	modified = b;
	gTheApp->GetWindowList().UpdateStatusBar();
}


wxString CC_show::Autosave()
{
	if (!autosave_name.IsEmpty() && !OnSaveDocument(autosave_name))
	{
		return writeerr_str;
	}
	else
	{
		return wxT("");
	}
}

wxString CC_show::UserGetName() const
{
	if (name.empty()) return wxT("Untitled");
	else return wxFileNameFromPath(name);
}


void CC_show::SetName(const wxString& newname)
{
// make into a full path
	wxString path = FullPath(newname);
	name = path;
	SetAutosaveName(path);
}


void CC_show::SetAutosaveName(const wxString& realname)
{
	autosave_name = autosave_dir;
	autosave_name.Append(PATH_SEPARATOR);
	autosave_name.Append(wxFileNameFromPath(realname));
}


void CC_show::UserSetDescr(const wxString& newdescr, wxWindow *win)
{
// Create undo entry
	undolist->Add(new ShowUndoDescr(this));
	descr = newdescr;
	gTheApp->GetWindowList().SetDescr(win);
}


const CC_sheet *CC_show::GetNthSheet(unsigned n) const
{
	const CC_sheet *nsheet = sheets;
	while (n && nsheet)
	{
		n--;
		nsheet = nsheet->next;
	}
	return nsheet;
}


CC_sheet *CC_show::GetNthSheet(unsigned n)
{
	CC_sheet *nsheet = sheets;
	while (n && nsheet)
	{
		n--;
		nsheet = nsheet->next;
	}
	return nsheet;
}


unsigned CC_show::GetSheetPos(const CC_sheet *sheet) const
{
	const CC_sheet *nsheet = sheets;
	unsigned n = 0;
	while (nsheet!=sheet)
	{
		if (nsheet == NULL) return 0;
		nsheet = nsheet->next; n++;
	}
	return n;
}


CC_sheet *CC_show::RemoveNthSheet(unsigned sheetidx)
{
	CC_sheet *sht = sheets;
	CC_sheet *tmp;
	unsigned idx;

	if (sheetidx > 0)
	{
		idx = sheetidx;
		while (--idx)
		{
			sht = sht->next;
		}
		tmp = sht->next;
		sht->next = tmp->next;
		sht = tmp;
	}
	else
	{
		sheets = sheets->next;
	}
	numsheets--;
	sht->next = NULL;
	gTheApp->GetWindowList().DeleteSheet(sheetidx);
	return sht;
}


CC_sheet *CC_show::RemoveLastSheets(unsigned numtoremain)
{
	CC_sheet *sht = sheets;
	CC_sheet *tmp;
	unsigned idx;

	if (numtoremain > 0)
	{
		idx = numtoremain;
		while (--idx)
		{
			sht = sht->next;
		}
		tmp = sht->next;
		sht->next = NULL;
		sht = tmp;
	}
	else
	{
		sheets = NULL;
	}
	numsheets = numtoremain;
	gTheApp->GetWindowList().RemoveSheets(numtoremain);
	return sht;
}


void CC_show::DeleteNthSheet(unsigned sheetidx)
{
	delete RemoveNthSheet(sheetidx);
}


void CC_show::UserDeleteSheet(unsigned sheetidx)
{
	CC_sheet *sht = RemoveNthSheet(sheetidx);
	undolist->Add(new ShowUndoDelete(sheetidx, sht));
}


void CC_show::InsertSheetInternal(CC_sheet *nsheet, unsigned sheetidx)
{
	CC_sheet *sht = sheets;
	unsigned idx;

	if (sheetidx > 0)
	{
		idx = sheetidx;
		while (--idx)
		{
			sht = sht->next;
		}
		nsheet->next = sht->next;
		sht->next = nsheet;
	}
	else
	{
		nsheet->next = sheets;
		sheets = nsheet;
	}
	numsheets++;
}


void CC_show::InsertSheet(CC_sheet *nsheet, unsigned sheetidx)
{
	InsertSheetInternal(nsheet, sheetidx);
	gTheApp->GetWindowList().AddSheet(sheetidx);
}


void CC_show::UserInsertSheet(CC_sheet *sht, unsigned sheetidx)
{
	InsertSheet(sht, sheetidx);
	undolist->Add(new ShowUndoCopy(sheetidx));
}


void CC_show::SetNumPoints(unsigned num, unsigned columns)
{
	unsigned i, cpy;
	CC_sheet *sht;

	for (sht = sheets; sht != NULL; sht = sht->next)
	{
		sht->SetNumPoints(num, columns);
	}

	undolist->EraseAll();						  // Remove all previously avail undo

	delete selections;
	selections = new bool[num];
	std::vector<wxString> new_labels(num);
	for (i = 0; i < num; i++)
	{
		selections[i] = false;
	}
	cpy = MIN(numpoints, num);
	for (i = 0; i < cpy; i++)
	{
		new_labels[i] = pt_labels[i];
	}
	for (; i < num; i++)
	{
		new_labels[i] = wxT("");
	}
	pt_labels = new_labels;

	numpoints = num;

	SetModified(true);
}


void CC_show::SetNumPointsInternal(unsigned num)
{
	numpoints = num;
	pt_labels.assign(numpoints, wxString());
	selections = new bool[numpoints];
	UnselectAll();
}


bool CC_show::RelabelSheets(unsigned sht)
{
	CC_sheet *sheet;
	unsigned i,j;
	unsigned *table;
	bool *used_table;

	sheet = GetNthSheet(sht);
	if (sheet->next == NULL) return false;
	table = new unsigned[GetNumPoints()];
	used_table = new bool[GetNumPoints()];

	for (i = 0; i < GetNumPoints(); i++)
	{
		used_table[i] = false;
	}
	for (i = 0; i < GetNumPoints(); i++)
	{
		for (j = 0; j < GetNumPoints(); j++)
		{
			if (!used_table[j])
			{
				if (sheet->GetPosition(i) == sheet->next->GetPosition(j))
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
			delete [] table;
			delete [] used_table;
			return false;
		}
	}
	while ((sheet = sheet->next) != NULL)
	{
		sheet->RelabelSheet(table);
	}

	delete [] table;
	return true;
}


bool CC_show::UnselectAll()
{
	bool changed = false;
	for (unsigned i=0; i<numpoints; i++)
	{
		if (IsSelected(i))
		{
			Select(i, false);
			changed = true;
		}
	}
	return changed;
}


void CC_show::Select(unsigned i, bool val)
{
	selections[i] = val;
	if (val)
	{
		selectionList.push_back(i);
	}
	else
	{
		if (std::find(selectionList.begin(), selectionList.end(), i) != selectionList.end())
			selectionList.erase(std::find(selectionList.begin(), selectionList.end(), i));
	}
}

#include <iostream>
using std::cout;

struct ShowTestData
{
	bool Ok;
	
};
void UnitTests()
{
	boost::shared_ptr<CC_show> test(new CC_show);
	cout<<"ok "<<test->Ok()<<"\n";
//	assert(test.Ok() == true);
	cout<<"GetError "<<(wchar_t*)test->GetError().c_str()<<"\n";
//	assert(test.Ok() == true);
	cout<<"GetName "<<(wchar_t*)test->GetName().c_str()<<"\n";
	cout<<"UserGetName "<<(wchar_t*)test->UserGetName().c_str()<<"\n";
	cout<<"GetDescr "<<(wchar_t*)test->GetDescr().c_str()<<"\n";
	cout<<"UserGetDescr "<<(wchar_t*)test->UserGetDescr().c_str()<<"\n";
	cout<<"Modified "<<test->Modified()<<"\n";
	cout<<"GetNumSheets "<<test->GetNumSheets()<<"\n";
	cout<<"GetSheet "<<test->GetSheet()<<"\n";
	cout<<"GetSheetPos "<<test->GetSheetPos(NULL)<<"\n";
	cout<<"GetNumPoints "<<test->GetNumPoints()<<"\n";
	for (unsigned i = 0; i < test->GetNumSheets(); ++i)
	{
		cout<<"GetNthSheet "<<i<<" "<<test->GetNthSheet(i)<<"\n";
		cout<<"GetBoolLandscape "<<i<<" "<<test->GetBoolLandscape()<<"\n";
		cout<<"GetBoolDoCont "<<i<<" "<<test->GetBoolDoCont()<<"\n";
		cout<<"GetBoolDoContSheet "<<i<<" "<<test->GetBoolDoContSheet()<<"\n";
	}
	for (unsigned i = 0; i < test->GetNumPoints(); ++i)
	{
		cout<<"GetPointLabel "<<i<<" "<<(wchar_t*)test->GetPointLabel(i).c_str()<<"\n";
	}
	cout<<"GetMode "<<(void*)&test->GetMode()<<"\n";
}


