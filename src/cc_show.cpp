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

#include <fstream>

#include "cc_show.h"

#include "cc_command.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "cc_point.h"
#include "show.h"

#include <wx/wfstream.h>
#include <list>

static const wxChar *nofile_str = wxT("Unable to open file");
static const wxChar *badcont_str = wxT("Error in continuity file");
static const wxChar *contnohead_str = wxT("Continuity file doesn't begin with header");


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


CC_FileException::CC_FileException(uint32_t nameID)
{
	uint8_t rawd[4];
	put_big_long(rawd, nameID);
	mError.Printf(wxT("Wrong ID read:  Read %c%c%c%c"), rawd[0], rawd[1], rawd[2], rawd[3]);
}

IMPLEMENT_DYNAMIC_CLASS(CC_show_modified, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CC_show_FlushAllViews, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CC_show_AllViewsGoToCont, wxObject)
IMPLEMENT_DYNAMIC_CLASS(CC_show_setup, wxObject)

IMPLEMENT_DYNAMIC_CLASS(CC_show, wxDocument);

// Create a new show
CC_show::CC_show()
:mOkay(true), numpoints(0),
print_landscape(false), print_do_cont(true),
print_do_cont_sheet(true),
mSheetNum(0),
mTimer(*this)
{
	mTimer.Start(GetConfiguration_AutosaveInterval()*1000);
	mode = *wxGetApp().GetModeList().Begin();
}

// When a file is opened, we first check to see if there is a temporary 
// file, and if there is, prompt the user to see if they would like use
// that file instead.
bool CC_show::OnOpenDocument(const wxString& filename)
{
	// first check to see if there is a recover file:
	wxString recoveryFile = TranslateNameToAutosaveName(filename);
	if (wxFileExists(recoveryFile))
	{
		// prompt the user to find out if they would like to use the recovery file
		int userchoice = wxMessageBox(
			wxT("CalChart has detected a recovery file (possibly from a previous crash).  ")
			wxT("Would you like to use the recovery file (Warning: choosing recover will ")
			wxT("destroy the original file)?"), wxT("Recovery File Detected"), wxYES_NO|wxCANCEL);
		if (userchoice == wxYES)
		{
			// move the recovery file to the filename, destroying the file and using the recovery
			wxCopyFile(recoveryFile, filename);
		}
		if (userchoice == wxNO)
		{
		}
		if (userchoice == wxCANCEL)
		{
			return false;
		}
	}
	bool success = wxDocument::OnOpenDocument(filename) && mOkay;
	if (success)
	{
		// at this point the recover file is no longer useful.
		if (wxFileExists(recoveryFile))
		{
			wxRemoveFile(recoveryFile);
		}
	}
	return success;
}

// If we close a file and decide not to save the changes, don't create a recovery
// file, it may confuse the user.
bool CC_show::OnCloseDocument()
{
	bool success = wxDocument::OnCloseDocument();
	// first check to see if there is a recover file:
	wxString recoveryFile = TranslateNameToAutosaveName(GetFilename());
	if (!IsModified() && wxFileExists(recoveryFile))
	{
		wxRemoveFile(recoveryFile);
	}
	return success;
}

bool CC_show::OnNewDocument()
{
	bool success = wxDocument::OnNewDocument();
	if (success)
	{
		// notify the views that we are a new document.  That should prompt a wizard to set up the show
		CC_show_setup show_setup;
		UpdateAllViews(NULL, &show_setup);
	}
	return success;
}

// When we save a file, the recovery file should be removed to prevent
// a false detection that the file writing failed.
bool CC_show::OnSaveDocument(const wxString& filename)
{
	bool result = wxDocument::OnSaveDocument(filename);
	wxString recoveryFile = TranslateNameToAutosaveName(filename);
	if (result && wxFileExists(recoveryFile))
	{
		wxRemoveFile(recoveryFile);
	}
	return true;
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

template <typename T>
void ReadLong(T& stream, uint32_t& d)
{
	uint8_t rawd[4];
	stream.Read(rawd, sizeof(rawd));
	d = get_big_long(&rawd);
}

template <>
void ReadLong<wxSTD istream>(wxSTD istream& stream, uint32_t& d)
{
	char rawd[4];
	stream.read(rawd, sizeof(rawd));
	d = get_big_long(&rawd);
}

// return false if you don't read the inname
template <typename T>
void ReadAndCheckID(T& stream, uint32_t inname)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
}

// return false if you don't read the inname
template <typename T>
void ReadCheckIDandSize(T& stream, uint32_t inname, uint32_t& size)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	ReadLong(stream, name);
	if (4 != name)
	{
		uint8_t rawd[4];
		put_big_long(rawd, inname);
		wxString s; s.Printf(wxT("Wrong size %d for name %c%c%c%c"), name, rawd[0], rawd[1], rawd[2], rawd[3]);
		throw CC_FileException(s);
	}
	ReadLong(stream, size);
}

// return false if you don't read the inname
template <typename T>
void ReadCheckIDandFillData(T& stream, uint32_t inname, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	ReadLong(stream, name);
	data.resize(name);
	stream.Read(&data[0], name);
}

template <>
void ReadCheckIDandFillData<wxSTD istream>(wxSTD istream& stream, uint32_t inname, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	if (inname != name)
	{
		throw CC_FileException(inname);
	}
	ReadLong(stream, name);
	data.resize(name);
	stream.read(reinterpret_cast<char*>(&data[0]), name);
}

// Just fill the data
template <typename T>
void FillData(T& stream, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	data.resize(name);
	stream.Read(&data[0], name);
}

template <>
void FillData<wxSTD istream>(wxSTD istream& stream, std::vector<uint8_t>& data)
{
	uint32_t name;
	ReadLong(stream, name);
	data.resize(name);
	stream.read(reinterpret_cast<char*>(&data[0]), name);
}

template <typename T>
void WriteLong(T& stream, uint32_t d)
{
	uint8_t rawd[4];
	put_big_long(rawd, d);
	stream.Write(rawd, sizeof(rawd));
}

template <>
void WriteLong<wxSTD ostream>(wxSTD ostream& stream, uint32_t d)
{
	char rawd[4];
	put_big_long(rawd, d);
	stream.write(rawd, sizeof(rawd));
}

template <typename T>
void WriteHeader(T& stream)
{
	WriteLong(stream, INGL_INGL);
}


template <typename T>
void WriteGurk(T& stream, uint32_t name)
{
	WriteLong(stream, INGL_GURK);
	WriteLong(stream, name);
}


template <typename T>
void WriteChunkHeader(T& stream, uint32_t name, uint32_t size)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
}


template <typename T>
void WriteChunk(T& stream, uint32_t name, uint32_t size, const void *data)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
	if (size > 0)
		stream.Write(data, size);
}

template <>
void WriteChunk<wxSTD ostream>(wxSTD ostream& stream, uint32_t name, uint32_t size, const void *data)
{
	WriteLong(stream, name);
	WriteLong(stream, size);
	if (size > 0)
		stream.write(reinterpret_cast<const char*>(data), size);
}

template <typename T>
void WriteChunkStr(T& stream, uint32_t name, const char *str)
{
	WriteChunk(stream, name, strlen(str)+1, reinterpret_cast<const unsigned char *>(str));
}

template <typename T>
void WriteEnd(T& stream, uint32_t name)
{
	WriteLong(stream, INGL_END);
	WriteLong(stream, name);
}


template <typename T>
void WriteStr(T& stream, const char *str)
{
	stream.Write(str, strlen(str)+1);
}

template <>
void WriteStr<wxSTD ostream>(wxSTD ostream& stream, const char *str)
{
	stream.write(str, strlen(str)+1);
}

template <typename T>
void Write(T& stream, const void *data, uint32_t size)
{
	stream.Write(data, size);
}

template <>
void Write<wxSTD ostream>(wxSTD ostream& stream, const void *data, uint32_t size)
{
	stream.write(reinterpret_cast<const char*>(data), size);
}

// Destroy a show
CC_show::~CC_show()
{}


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
		CC_sheet_iterator_t curr_sheet = CC_sheet_iterator_t(NULL);
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
				if (curr_sheet != CC_sheet_iterator_t(NULL)) ++curr_sheet;
				else curr_sheet = GetSheetBegin();
				if (curr_sheet == GetSheetEnd()) break;
				if (tempbuf.Length() > 2)
				{
					curr_sheet->SetNumber(tempbuf.Mid(2));
				}
			}
			else
			{
				if (curr_sheet == CC_sheet_iterator_t(NULL))
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


template <typename T>
T& CC_show::SaveObjectGeneric(T& stream)
{
	// flush out the text before we save a file.
	FlushAllTextWindows();
	return SaveObjectInternal(stream);
}

#if wxUSE_STD_IOSTREAM
wxSTD ostream& CC_show::SaveObject(wxSTD ostream& stream)
{
	return SaveObjectGeneric<wxSTD ostream>(stream);
}
#else
wxOutputStream& CC_show::SaveObject(wxOutputStream& stream)
{
	return SaveObjectGeneric<wxOutputStream>(stream);
}
#endif

template <typename T>
T& CC_show::SaveObjectInternal(T& stream)
{
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
	if (!GetDescr().empty())
	{
		WriteChunkStr(stream, INGL_DESC, GetDescr().utf8_str());
	}

// Handle sheets
	for (const_CC_sheet_iterator_t curr_sheet = GetSheetBegin();
		curr_sheet != GetSheetEnd();
		++curr_sheet)
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
			Coord crd;
			put_big_word(&crd, curr_sheet->GetPosition(i).x);
			Write(stream, &crd, 2);
			put_big_word(&crd, curr_sheet->GetPosition(i).y);
			Write(stream, &crd, 2);
		}
// Ref point positions
		for (size_t j = 1; j <= CC_point::kNumRefPoints; j++)
		{
			for (i = 0; i < GetNumPoints(); i++)
			{
				if (curr_sheet->GetPosition(i) != curr_sheet->GetPosition(i, j))
				{
					Coord crd;
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
					char c = (curr_sheet->GetPoint(i).GetFlip()) ? true : false;
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
				1+curranimcont->GetName().Length()+1+
				curranimcont->GetText().Length()+1);
			unsigned tnum = curranimcont->GetNum();
			Write(stream, &tnum, 1);
			WriteStr(stream, curranimcont->GetName().utf8_str());
			WriteStr(stream, curranimcont->GetText().utf8_str());
		}
		WriteEnd(stream, INGL_SHET);
	}

	WriteEnd(stream, INGL_SHOW);
	
	return stream;
}

template <typename T>
T& CC_show::LoadObjectGeneric(T& stream)
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
	numpoints = name;
	pt_labels.assign(numpoints, wxString());

	ReadLong(stream, name);
	// Optional: read in the point labels
	// <INGL_LABL><SIZE>
	if (INGL_LABL == name)
	{
		FillData(stream, data);
		std::vector<wxString> labels;
		const char *str = (const char*)&data[0];
		for (unsigned i = 0; i < GetNumPoints(); i++)
		{
			wxString buf = wxString::FromUTF8(str);
			labels.push_back(buf);
			str += strlen(str)+1;
		}
		SetPointLabel(labels);
		// peek for the next name
		ReadLong(stream, name);
	}

	// Optional: read in the point labels
	// <INGL_DESC><SIZE>
	if (INGL_DESC == name)
	{
		FillData(stream, data);
		wxString s(wxString::FromUTF8((const char*)&data[0]));
		SetDescr(s);
		// peek for the next name
		ReadLong(stream, name);
	}

	// Read in sheets
	// <INGL_GURK><INGL_SHET>
	while (INGL_GURK == name)
	{
		ReadAndCheckID(stream, INGL_SHET);

		CC_sheet sheet(this);

		// Read in sheet name
		// <INGL_NAME><size><string + 1>
		ReadCheckIDandFillData(stream, INGL_NAME, data);
		sheet.SetName(wxString::FromUTF8((const char*)&data[0]));

		// read in the duration:
		// <INGL_DURA><4><duration>
		ReadCheckIDandSize(stream, INGL_DURA, name);
		sheet.SetBeats(name);

		// Point positions
		// <INGL_DURA><size><data>
		ReadCheckIDandFillData(stream, INGL_POS, data);
		if (data.size() != size_t(GetNumPoints()*4))
		{
			throw CC_FileException(wxT("bad POS chunk"));
		}
		{
			uint8_t *d;
			d = (uint8_t*)&data[0];
			for (unsigned i = 0; i < GetNumPoints(); ++i)
			{
				CC_coord c;
				c.x = get_big_word(d);
				d += 2;
				c.y = get_big_word(d);
				d += 2;
				sheet.SetAllPositions(c, i);
			}
		}

		ReadLong(stream, name);
		// read all the reference points
		while (INGL_REFP == name)
		{
			FillData(stream, data);
			if (data.size() != (unsigned long)GetNumPoints()*4+2)
			{
				throw CC_FileException(wxT("Bad REFP chunk"));
			}
			uint8_t *d = (uint8_t*)&data[0];
			unsigned ref = get_big_word(d);
			d += 2;
			for (unsigned i = 0; i < GetNumPoints(); i++)
			{
				CC_coord c;
				c.x = get_big_word(d);
				d += 2;
				c.y = get_big_word(d);
				d += 2;
				sheet.SetPosition(c, i, ref);		  // don't clip
			}
			ReadLong(stream, name);
		}
		// Point symbols
		while (INGL_SYMB == name)
		{
			FillData(stream, data);
			if (data.size() != (unsigned long)GetNumPoints())
			{
				throw CC_FileException(wxT("Bad SYMB chunk"));
			}
			uint8_t *d = (uint8_t *)&data[0];
			for (unsigned i = 0; i < GetNumPoints(); i++)
			{
				sheet.GetPoint(i).sym = (SYMBOL_TYPE)(*(d++));
			}
			ReadLong(stream, name);
		}
		// Point continuity types
		while (INGL_TYPE == name)
		{
			FillData(stream, data);
			if (data.size() != (unsigned long)GetNumPoints())
			{
				throw CC_FileException(wxT("Bad TYPE chunk"));
			}
			uint8_t *d = (uint8_t *)&data[0];
			for (unsigned i = 0; i < GetNumPoints(); i++)
			{
				sheet.GetPoint(i).cont = *(d++);
			}
			ReadLong(stream, name);
		}
		// Point labels (left or right)
		while (INGL_LABL == name)
		{
			FillData(stream, data);
			if (data.size() != (unsigned long)GetNumPoints())
			{
				throw CC_FileException(wxT("Bad SYMB chunk"));
			}
			uint8_t *d = (uint8_t *)&data[0];
			for (unsigned i = 0; i < GetNumPoints(); i++)
			{
				if (*(d++))
				{
					sheet.GetPoint(i).Flip();
				}
			}
			ReadLong(stream, name);
		}
		// Continuity text
		while (INGL_CONT == name)
		{
			FillData(stream, data);
			if (data.size() < 3)						  // one byte num + two nils minimum
			{
				throw CC_FileException(wxT("Bad cont chunk"));
			}
			const char *d = (const char *)&data[0];
			if (d[data.size()-1] != '\0')
			{
				throw CC_FileException(wxT("Bad cont chunk"));
			}

			const char* text = d + 1;
			size_t num = strlen(text);
			if (data.size() < num + 3)					  // check for room for text string
			{
				throw CC_FileException(wxT("Bad cont chunk"));
			}
			wxString namestr(wxString::FromUTF8(text));
			text = d + 2 + strlen(text);
			CC_continuity newcont(namestr, *((uint8_t *)&data[0]));
			wxString textstr(wxString::FromUTF8(text));
			newcont.SetText(textstr);
			sheet.AppendContinuity(newcont);

			ReadLong(stream, name);
		}
		InsertSheetInternal(sheet, GetNumSheets());

		//ReadAndCheckID(stream, INGL_END);
		ReadAndCheckID(stream, INGL_SHET);
		// peek for the next name
		ReadLong(stream, name);
	}
	//ReadAndCheckID(stream, INGL_END);
	ReadAndCheckID(stream, INGL_SHOW);
	mSheetNum = 0;
	}
	catch (CC_FileException& e) {
		wxString message = wxT("Error encountered:\n");
		message += e.WhatError();
		wxMessageBox(message, wxT("Error!"));
		mOkay = false;
	}
	return stream;
}

#if wxUSE_STD_IOSTREAM
wxSTD istream& CC_show::LoadObject(wxSTD istream& stream)
{
	return LoadObjectGeneric<wxSTD istream>(stream);
}
#else
wxInputStream& CC_show::LoadObject(wxInputStream& stream)
{
	return LoadObjectGeneric<wxInputStream>(stream);
}
#endif

void CC_show::FlushAllTextWindows()
{
	CC_show_FlushAllViews flushMod;
	UpdateAllViews(NULL, &flushMod);
}


const wxString& CC_show::GetDescr() const
{
	return descr;
}


void CC_show::Modify(bool b)
{
	wxDocument::Modify(b);
	CC_show_modified showMod;
	UpdateAllViews(NULL, &showMod);
}


void CC_show::AutoSaveTimer::Notify()
{
	mShow.Autosave();
}

wxString CC_show::TranslateNameToAutosaveName(const wxString& name)
{
	return name + wxT("~");
}

// When the timer goes off, and if the show has a name and is modified,
// we will write the file to a version of the file that the same
// but with the extension .shw~, to indicate that there is a recovery
// file at that location.
void CC_show::Autosave()
{
	if (GetFilename() != wxT("") && IsModified())
	{
		wxFFileOutputStream outputStream(TranslateNameToAutosaveName(GetFilename()));
		if (outputStream.IsOk())
		{
			SaveObjectInternal(outputStream);
		}
		if (!outputStream.IsOk())
		{
			wxMessageBox(wxT("Error creating recovery file.  Take heed, save often!"), wxT("Recovery Error"));
		}
	}
}

void CC_show::SetDescr(const wxString& newdescr)
{
	descr = newdescr;
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
	UpdateAllViews();
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
void CC_show::SetNumPoints(unsigned num, unsigned columns)
{
	for (CC_sheet_iterator_t sht = GetSheetBegin(); sht != GetSheetEnd(); ++sht)
	{
		sht->SetNumPoints(num, columns);
	}
	numpoints = num;
}


bool CC_show::RelabelSheets(unsigned sht)
{
	unsigned i,j;

	CC_sheet_iterator_t sheet = GetNthSheet(sht);
	CC_sheet_iterator_t sheet_next = GetNthSheet(sht+1);
	if (sheet_next == GetSheetEnd()) return false;
	std::vector<unsigned> table(GetNumPoints());
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
		sheet->RelabelSheet(&table[0]);
	}

	return true;
}


wxString CC_show::GetPointLabel(unsigned i) const
{
	if (i >= pt_labels.size())
		return wxT("");
	return pt_labels.at(i);
}


bool CC_show::SelectAll()
{
	bool changed = selectionList.size() != numpoints;
	for (size_t i = 0; i < numpoints; ++i)
		selectionList.insert(i);
	UpdateAllViews();
	return changed;
}


bool CC_show::UnselectAll()
{
	bool changed = selectionList.size();
	selectionList.clear();
	UpdateAllViews();
	return changed;
}


void CC_show::SetSelection(const SelectionList& sl)
{
	selectionList = sl;
	UpdateAllViews();
}


void CC_show::AddToSelection(const SelectionList& sl)
{
	selectionList.insert(sl.begin(), sl.end());
	UpdateAllViews();
}

void CC_show::RemoveFromSelection(const SelectionList& sl)
{
	selectionList.erase(sl.begin(), sl.end());
	UpdateAllViews();
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
	UpdateAllViews();
}

void CC_show::AllViewGoToCont(unsigned contnum, int line, int col)
{
	CC_show_AllViewsGoToCont goToMod(contnum, line, col);
	UpdateAllViews(NULL, &goToMod);
}


#include <iostream>
using std::cout;

struct ShowTestData
{
	bool Ok;
	
};
void UnitTests()
{
	CC_show test;
//	cout<<"ok "<<test.mOkay<<"\n";
//	assert(test.Ok() == true);
//	cout<<"GetError "<<(wchar_t*)test.GetError().c_str()<<"\n";
//	assert(test.Ok() == true);
	cout<<"GetTitle "<<test.GetTitle().c_str()<<"\n";
	cout<<"GetDescr "<<test.GetDescr().c_str()<<"\n";
	cout<<"Modified "<<test.IsModified()<<"\n";
	cout<<"GetNumSheets "<<test.GetNumSheets()<<"\n";
	cout<<"GetNumPoints "<<test.GetNumPoints()<<"\n";
	for (unsigned i = 0; i < test.GetNumSheets(); ++i)
	{
		cout<<"GetBoolLandscape "<<i<<" "<<test.GetBoolLandscape()<<"\n";
		cout<<"GetBoolDoCont "<<i<<" "<<test.GetBoolDoCont()<<"\n";
		cout<<"GetBoolDoContSheet "<<i<<" "<<test.GetBoolDoContSheet()<<"\n";
	}
	for (unsigned i = 0; i < test.GetNumPoints(); ++i)
	{
		cout<<"GetPointLabel "<<i<<" "<<test.GetPointLabel(i).c_str()<<"\n";
	}
	cout<<"GetMode "<<(void*)&test.GetMode()<<"\n";
}


