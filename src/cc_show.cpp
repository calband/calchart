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

#include "cc_show.h"

#include "undo.h"
#include "confgr.h"
#include "calchartapp.h"
#include "cc_sheet.h"
#include "cc_continuity.h"
#include "ingl.h"

static const wxChar *nomem_str = wxT("Out of memory!");
static const wxChar *nofile_str = wxT("Unable to open file");
static const wxChar *badfile_mas_str = wxT("Error reading master file");
static const wxChar *badfile_pnt_str = wxT("Error reading points file");
static const wxChar *badfile_cnt_str = wxT("Error reading animation continuity file");
static const wxChar *badanimcont_str = wxT("Error in animation continuity file");
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

static void ChangeExtension(const wxString& in, wxString& out,
const wxString& ext)
{
	int i = in.Last('.');
	out = in.Mid(0, i);
	out.Append(ext);
}


class AutoSaveTimer: public wxTimer
{
	public:
		AutoSaveTimer(): untitled_number(1) {}
		void Notify()
		{
			const CC_show *show;

			for (std::list<const CC_show*>::const_iterator i = showlist.begin(); i != showlist.end(); ++i)
			{
				show = *i;
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

		inline void AddShow(const CC_show *show)
		{
			showlist.push_back(show);
		}
		inline void RemoveShow(const CC_show *show)
		{
			showlist.remove(show);
		}
		inline int GetNumber() { return untitled_number++; }
	private:
		std::list<const CC_show*> showlist;
		int untitled_number;
};

static AutoSaveTimer autosaveTimer;

void SetAutoSave(int secs)
{
	if (secs > 0)
		autosaveTimer.Start(secs*1000);
}


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


#define INGL_SHOW MakeINGLid('S','H','O','W')
#define INGL_SHET MakeINGLid('S','H','E','T')
#define INGL_SIZE MakeINGLid('S','I','Z','E')
#define INGL_LABL MakeINGLid('L','A','B','L')
#define INGL_MODE MakeINGLid('M','O','D','E')
#define INGL_DESC MakeINGLid('D','E','S','C')
#define INGL_NAME MakeINGLid('N','A','M','E')
#define INGL_DURA MakeINGLid('D','U','R','A')
#define INGL_POS  MakeINGLid('P','O','S',' ')
#define INGL_SYMB MakeINGLid('S','Y','M','B')
#define INGL_TYPE MakeINGLid('T','Y','P','E')
#define INGL_REFP MakeINGLid('R','E','F','P')
#define INGL_CONT MakeINGLid('C','O','N','T')
#define INGL_PCNT MakeINGLid('P','C','N','T')

// Only exists so SHOW chunk is recognized
static const char* load_show_SHOW(INGLchunk*)
{
	return NULL;
}


static const char* load_show_SHET(INGLchunk* chunk)
{
	CC_show *show = (CC_show*)chunk->prev->userdata;
	CC_sheet *sheet = new CC_sheet(show);

	show->InsertSheetInternal(sheet, show->GetNumSheets());
	chunk->userdata = sheet;

	return NULL;
}


static const char* load_show_SIZE(INGLchunk* chunk)
{
	CC_show *show = (CC_show*)chunk->prev->userdata;

	if ((show->GetNumPoints() > 0) || (show->GetSheet() != NULL))
	{
		return "Duplicate SIZE chunks in file";
	}
	if (chunk->size != 4)
	{
		return "Bad SIZE chunk";
	}
	show->SetNumPointsInternal(get_big_long(&chunk->data[0]));
	return NULL;
}


static const char* load_show_LABL(INGLchunk* chunk)
{
	unsigned i;
	const char *str = (const char*)&chunk->data[0];
	CC_show *show = (CC_show*)chunk->prev->userdata;
	for (i = 0; i < show->GetNumPoints(); i++)
	{
		show->GetPointLabel(i) = wxString::FromUTF8(str);
		str += strlen(str)+1;
	}
	return NULL;
}


static const char* load_show_MODE(INGLchunk* /*chunk*/) {
return NULL;
}


static const char* load_show_DESC(INGLchunk* chunk)
{
	CC_show *show = (CC_show*)chunk->prev->userdata;

	wxString s(wxString::FromUTF8((const char*)&chunk->data[0]));
	show->SetDescr(s);

	return NULL;
}


static const char* load_show_NAME(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;

	wxString s(wxString::FromUTF8((const char*)&chunk->data[0]));
	sheet->SetName(s);

	return NULL;
}


static const char* load_show_DURA(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;

	if (chunk->size != 4)
	{
		return "Bad DURA chunk";
	}
	sheet->SetBeats(get_big_long(&chunk->data[0]));

	return NULL;
}


static const char* load_show_POS(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
	unsigned i;
	uint8_t *data;
	CC_coord c;

	if (chunk->size != (unsigned long)sheet->show->GetNumPoints()*4)
	{
		return "Bad POS chunk";
	}
	data = (uint8_t*)&chunk->data[0];
	for (i = 0; i < sheet->show->GetNumPoints(); i++)
	{
		c.x = get_big_word(data);
		data += 2;
		c.y = get_big_word(data);
		data += 2;
		sheet->SetAllPositions(c, i);
	}

	return NULL;
}


static const char* load_show_SYMB(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
	unsigned i;
	uint8_t *data;

	if (chunk->size != sheet->show->GetNumPoints())
	{
		return "Bad SYMB chunk";
	}
	data = (uint8_t *)&chunk->data[0];
	for (i = 0; i < sheet->show->GetNumPoints(); i++)
	{
		sheet->GetPoint(i).sym = (SYMBOL_TYPE)(*(data++));
	}

	return NULL;
}


static const char* load_show_TYPE(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
	unsigned i;
	uint8_t *data;

	if (chunk->size != sheet->show->GetNumPoints())
	{
		return "Bad TYPE chunk";
	}
	data = (uint8_t *)&chunk->data[0];
	for (i = 0; i < sheet->show->GetNumPoints(); i++)
	{
		sheet->GetPoint(i).cont = *(data++);
	}

	return NULL;
}


static const char* load_show_REFP(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
	unsigned i, ref;
	uint8_t *data;
	CC_coord c;

	if (chunk->size != (unsigned long)sheet->show->GetNumPoints()*4+2)
	{
		return "Bad REFP chunk";
	}
	data = (uint8_t*)&chunk->data[0];
	ref = get_big_word(data);
	data += 2;
	for (i = 0; i < sheet->show->GetNumPoints(); i++)
	{
		c.x = get_big_word(data);
		data += 2;
		c.y = get_big_word(data);
		data += 2;
		sheet->SetPositionQuick(c, i, ref);		  // don't clip
	}

	return NULL;
}


static const char* load_show_SHET_LABL(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
	unsigned i;
	uint8_t *data;

	if (chunk->size != sheet->show->GetNumPoints())
	{
		return "Bad LABL chunk";
	}
	data = (uint8_t *)&chunk->data[0];
	for (i = 0; i < sheet->show->GetNumPoints(); i++)
	{
		if (*(data++))
		{
			sheet->GetPoint(i).Flip();
		}
	}

	return NULL;
}


static const char* badcontchunk = "Bad CONT chunk";
static const char* load_show_CONT(INGLchunk* chunk)
{
	CC_sheet *sheet = (CC_sheet*)chunk->prev->userdata;
	unsigned num;
	const char *name;
	const char *text;

	if (chunk->size < 3)						  // one byte num + two nils minimum
	{
		return badcontchunk;
	}
												  // make sure we have a nil
	if (((const char*)&chunk->data[0])[chunk->size-1] != '\0')
	{
		return badcontchunk;
	}
	name = (const char *)&chunk->data[0] + 1;
	num = strlen(name);
	if (chunk->size < num + 3)					  // check for room for text string
	{
		return badcontchunk;
	}
	text = (const char *)&chunk->data[0] + 2 + strlen(name);

	wxString namestr(wxString::FromUTF8(name));
	CC_continuity_ptr newcont(new CC_continuity(namestr, *((uint8_t *)&chunk->data[0])));
	wxString textstr(wxString::FromUTF8(text));
	newcont->SetText(textstr);
	sheet->AppendContinuity(newcont);

	return NULL;
}


static const char* load_show_PCNT(INGLchunk* /*chunk*/) {
return NULL;
}


static INGLhandler load_show_handlers[] =
{
	{ INGL_SHOW, 0, load_show_SHOW },
	{ INGL_SHET, INGL_SHOW, load_show_SHET },
	{ INGL_SIZE, INGL_SHOW, load_show_SIZE },
	{ INGL_LABL, INGL_SHOW, load_show_LABL },
	{ INGL_MODE, INGL_SHOW, load_show_MODE },
	{ INGL_DESC, INGL_SHOW, load_show_DESC },
	{ INGL_NAME, INGL_SHET, load_show_NAME },
	{ INGL_DURA, INGL_SHET, load_show_DURA },
	{ INGL_POS , INGL_SHET, load_show_POS  },
	{ INGL_SYMB, INGL_SHET, load_show_SYMB },
	{ INGL_TYPE, INGL_SHET, load_show_TYPE },
	{ INGL_REFP, INGL_SHET, load_show_REFP },
	{ INGL_LABL, INGL_SHET, load_show_SHET_LABL },
	{ INGL_CONT, INGL_SHET, load_show_CONT },
	{ INGL_PCNT, INGL_SHET, load_show_PCNT }
};

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

// Load a show
CC_show::CC_show(const wxString& filestr) :
okay(true),
numpoints(0),
numsheets(0),
sheets(NULL),
selections(NULL),
modified(false),
print_landscape(false),
print_do_cont(true),
print_do_cont_sheet(true)
{

	undolist = new ShowUndoList(this, undo_buffer_size);
	mode = *gTheApp->GetModeList().Begin();

	wxString ext = filestr.AfterLast('.');
	if (!ext.CmpNoCase(wxT("mas")))
	{
		bool old_format_uppercase = false;
		if (!ext.Cmp(wxT("MAS")))
		{
			old_format_uppercase = true;
		}

		FILE *fp = fopen(filestr.fn_str(), "r");
		if (!fp)
		{
			AddError(nofile_str);
			return;
		}
		wxString tempbuf;
		if (ReadDOSline(fp, tempbuf) <= 0)
		{
			AddError(badfile_mas_str);
			return;
		}
		uint32_t keyvalue;
		if (CC_sscanf(tempbuf.c_str(), wxT(" %u "), &keyvalue) != 1)
		{
			AddError(badfile_mas_str);
			return;
		}
		if (keyvalue != 1024)
		{
			AddError(badfile_mas_str);
			return;
		}

		if (ReadDOSline(fp, tempbuf) <= 0)
		{
			AddError(badfile_mas_str);
			return;
		}
		if (CC_sscanf(tempbuf.c_str(), wxT(" %hu , %hu "), &numsheets, &numpoints) != 2)
		{
			AddError(badfile_mas_str);
			return;
		}
		pt_labels.assign(numpoints, wxString());
		selections = new bool[numpoints];
		UnselectAll();

		CC_sheet *curr_sheet = NULL;
		for (uint32_t i = 0; i < numsheets; ++i)
		{
			if (ReadDOSline(fp, tempbuf) <= 0)
			{
				AddError(badfile_mas_str);
				return;
			}
			char sheetnamebuf[16];
			uint32_t j;
			uint32_t off;
			if (CC_sscanf(tempbuf.c_str(), wxT(" \"%[^\"]\" , %u , %u\n"),
				sheetnamebuf, &j, &off) != 3)
			{
				AddError(badfile_mas_str);
				return;
			}
			if (j == 0) j=1;
			off = strlen(sheetnamebuf);
			while (off > 0)
			{
				off--;
				if (sheetnamebuf[off] != ' ')
				{
					sheetnamebuf[off+1] = 0;
					break;
				}
			}

// For old format, use ISO-8859-1 not UTF8
			wxString sheetnamestr(wxString::From8BitData(sheetnamebuf));
			if (curr_sheet == NULL)
			{
				sheets = new CC_sheet(this, sheetnamestr);
				if (!sheets)
				{
					AddError(nomem_str);
					return;
				}
				curr_sheet = sheets;
			}
			else
			{
				curr_sheet->next = new CC_sheet(this, sheetnamestr);
				if (!sheets)
				{
					AddError(nomem_str);
					return;
				}
				curr_sheet = curr_sheet->next;
			}
			curr_sheet->SetBeats(j);
		}
		fclose(fp);

		std::vector<cc_oldpoint> diskpts(numpoints);
		curr_sheet = sheets;
		for (size_t k = 1;
			k <= numsheets;
			k++, curr_sheet = curr_sheet->next)
		{
			ext.Printf(wxT("%c%u"), old_format_uppercase ? 'S':'s', k);
			wxString namebuf; namebuf = filestr.BeforeLast('.');
			namebuf.Append('.');
			namebuf.Append(ext);
			fp = fopen(namebuf.fn_str(), "rb");
			if (!fp)
			{
				AddError(nofile_str);
				return;
			}
			bool reallyoldpnts = false;
			int record_len = fread(&diskpts[0], numpoints, sizeof(cc_oldpoint), fp);
			if (record_len == sizeof(cc_oldpoint))
			{
				reallyoldpnts = false;
			}
			else if (record_len == sizeof(cc_reallyoldpoint))
			{
				reallyoldpnts = true;
			}
			else
			{
				AddError(badfile_pnt_str);
				return;
			}
			fclose(fp);

			for (size_t i = 0; i < numpoints; i++)
			{
				if (reallyoldpnts)
				{
					short refidx;
					unsigned short refloc;

					cc_oldpoint conv_diskpt;
					conv_diskpt.sym = ((cc_reallyoldpoint*)&diskpts[0])[i].sym;
					conv_diskpt.flags = ((cc_reallyoldpoint*)&diskpts[0])[i].flags;
					conv_diskpt.pos = ((cc_reallyoldpoint*)&diskpts[0])[i].pos;
					conv_diskpt.color = ((cc_reallyoldpoint*)&diskpts[0])[i].color;
					conv_diskpt.code[0] = ((cc_reallyoldpoint*)&diskpts[0])[i].code[0];
					conv_diskpt.code[1] = ((cc_reallyoldpoint*)&diskpts[0])[i].code[1];
					conv_diskpt.cont = ((cc_reallyoldpoint*)&diskpts[0])[i].cont;
					refidx = get_lil_word(&((cc_reallyoldpoint*)&diskpts[0])[i].refnum);
					if (refidx >= 0)
					{
						refloc =
							get_lil_word(&((cc_reallyoldpoint*)&diskpts[0])[refidx].pos.x) +
							(short)get_lil_word(&((cc_reallyoldpoint*)&diskpts[0])[i].ref.x);
						put_lil_word(&conv_diskpt.ref[0].x, refloc);

						refloc =
							get_lil_word(&((cc_reallyoldpoint*)&diskpts[0])[refidx].pos.y) +
							(short)get_lil_word(&((cc_reallyoldpoint*)&diskpts[0])[i].ref.y);
						put_lil_word(&conv_diskpt.ref[0].y, refloc);
					}
					else
					{
						conv_diskpt.ref[0].x = 0xFFFF;
						conv_diskpt.ref[0].x = 0xFFFF;
					}
					conv_diskpt.ref[1].x = 0xFFFF;
					conv_diskpt.ref[1].x = 0xFFFF;
					conv_diskpt.ref[2].x = 0xFFFF;
					conv_diskpt.ref[2].x = 0xFFFF;
					curr_sheet->SetPoint(conv_diskpt, i);
				}
				else
				{
					curr_sheet->SetPoint(diskpts[i], i);
				}

				if (k == 1)
				{
// Build table for point labels
					unsigned int label_idx = 0;
					char pt_buf[4];
					for (size_t j = 0; j < 2; j++)
					{
// Convert to ascii
						if ((diskpts[i].code[j] >= '0') && (diskpts[i].code[j] != '?'))
						{
							if (diskpts[i].code[j] <= 'Z')
							{
// normal ascii
								pt_buf[label_idx++] = diskpts[i].code[j];
							}
							else
							{
								if (diskpts[i].code[j] > 100)
								{
// digit (0-9)
									pt_buf[label_idx++] = diskpts[i].code[j] - 101 + '0';
								}
								else
								{
// '10' to '19' range
									sprintf(&pt_buf[label_idx], "%u",
										diskpts[i].code[j] - 81);
									label_idx = strlen(pt_buf);
								}
							}
						}
					}
					pt_buf[label_idx] = 0;
					pt_labels[i] = wxString::FromAscii(pt_buf);
				}
			}
// Now load animation continuity
// We need to use fgets and sscanf so blank lines aren't skipped
			ext.Printf(wxT("%c%u"), old_format_uppercase ? 'F':'f', k);
			namebuf = filestr.BeforeLast('.');
			namebuf.Append('.');
			namebuf.Append(ext);
			fp = fopen(namebuf.fn_str(), "r");
			if (!fp)
			{
				AddError(nofile_str);
				return;
			}
			if (ReadDOSline(fp, tempbuf) <= 0)
			{
				AddError(badfile_cnt_str);
				return;
			}
			if (tempbuf != wxT("'NEW"))
			{
				AddError(badanimcont_str);
				return;
			}
			if (ReadDOSline(fp, tempbuf) <= 0)
			{
				AddError(badanimcont_str);
				return;
			}
			unsigned numanimcont;
			if (CC_sscanf(tempbuf.c_str(), wxT(" %u"), &numanimcont) != 1)
			{
				AddError(badanimcont_str);
				return;
			}
			for (size_t i = 0; i < numanimcont; i++)
			{
// Skip blank line
				if (feof(fp))
				{
					AddError(badanimcont_str);
					return;
				}
				ReadDOSline(fp, tempbuf);
				if (ReadDOSline(fp, tempbuf) <= 0)
				{
					AddError(badanimcont_str);
					return;
				}
				char sheetnamebuf[16];
				uint32_t j;
				unsigned tnum;
				if (CC_sscanf(tempbuf.c_str(), wxT(" \"%[^\"]\" , %u , %u\n"),
					sheetnamebuf, &tnum, &j) != 3)
				{
					AddError(badanimcont_str);
					return;
				}
				wxString sheetnamestr(wxString::From8BitData(sheetnamebuf));
				CC_continuity_ptr newanimcont(new CC_continuity(sheetnamestr, tnum));
				while (j > 0)
				{
					j--;
					if (feof(fp))
					{
						AddError(badanimcont_str);
					}
					ReadDOSline(fp, tempbuf);
					newanimcont->AppendText(tempbuf.Strip() + wxT("\n"));
				}
				curr_sheet->AppendContinuity(newanimcont);
			}
			fclose(fp);
		}

// now load continuity file if it exists
		ext = old_format_uppercase ? wxT("TXT") : wxT("txt");
		wxString namebuf;
		namebuf = filestr.BeforeLast('.');
		namebuf.Append('.');
		namebuf.Append(ext);
		wxString namestr(namebuf.fn_str(), *wxConvFileName);
		if (wxFileExists(namestr))
		{
			wxString conterr = ImportContinuity(namestr);
			if (!conterr.empty())
			{
				AddError(conterr);
			}
		}

		wxString shwname;
		ChangeExtension(namestr, shwname, wxT("shw"));
		SetName(shwname);
	}
	else
	{
		INGLread readhnd(filestr.fn_str());
		if (!readhnd.Okay())
		{
			AddError(nofile_str);
		}
		else
		{
			const char *parseerror = NULL;
			readhnd.ParseFile(load_show_handlers,
				sizeof(load_show_handlers)/sizeof(INGLhandler),
				&parseerror, this);
			if (parseerror != NULL)
				AddError(wxString(parseerror, wxConvUTF8));
			SetName(filestr);
		}
	}
	if (okay && (sheets == NULL))
	{
		AddError(nosheets_str);
		return;
	}
	autosaveTimer.AddShow(this);
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


wxString CC_show::SaveInternal(const wxString& filename) const
{
	wxString bakfile;
	if (wxFileExists(filename))
	{
		ChangeExtension(filename, bakfile, wxT("bak"));
		if (!wxCopyFile(filename, bakfile))
			return nofile_str;
	}

	INGLwrite *handl = new INGLwrite(filename.fn_str());
	INGLid id;
	unsigned i, j;
	Coord crd;
	unsigned char c;

	FlushAllTextWindows();

	if (!handl->Okay())
	{
		delete handl;
		return nofile_str;
	}
	if (!handl->WriteHeader())
	{
		return writeerr_str;
	}
	if (!handl->WriteGurk(INGL_SHOW))
	{
		return writeerr_str;
	}

// Handle show info
	i = GetNumPoints();
	put_big_long(&id, i);
	if (!handl->WriteChunk(INGL_SIZE, 4, &id))
	{
		return writeerr_str;
	}

	id = 0;
	for (i = 0; i < GetNumPoints(); i++)
	{
		id += strlen(GetPointLabel(i).utf8_str())+1;
	}
	if (id > 0)
	{
		if (!handl->WriteChunkHeader(INGL_LABL, id))
		{
			return writeerr_str;
		}
		for (i = 0; i < GetNumPoints(); i++)
		{
			if (!handl->WriteStr(GetPointLabel(i).utf8_str()))
			{
				return writeerr_str;
			}
		}
	}

//handl->WriteChunkStr(INGL_MODE, mode->Name());

// Description
	if (!UserGetDescr().empty())
	{
		if (!handl->WriteChunkStr(INGL_DESC, UserGetDescr().utf8_str()))
		{
			return writeerr_str;
		}
	}

// Handle sheets
	for (const CC_sheet *curr_sheet = GetSheet();
		curr_sheet != NULL;
		curr_sheet = curr_sheet->next)
	{
		if (!handl->WriteGurk(INGL_SHET))
		{
			return writeerr_str;
		}
// Name
		if (!handl->WriteChunkStr(INGL_NAME, curr_sheet->GetName().utf8_str()))
		{
			return writeerr_str;
		}
// Beats
		put_big_long(&id, curr_sheet->GetBeats());
		if (!handl->WriteChunk(INGL_DURA, 4, &id))
		{
			return writeerr_str;
		}

// Point positions
		if (!handl->WriteChunkHeader(INGL_POS, GetNumPoints()*4))
		{
			return writeerr_str;
		}
		for (i = 0; i < GetNumPoints(); i++)
		{
			put_big_word(&crd, curr_sheet->GetPosition(i).x);
			if (!handl->Write(&crd, 2))
			{
				return writeerr_str;
			}
			put_big_word(&crd, curr_sheet->GetPosition(i).y);
			if (!handl->Write(&crd, 2))
			{
				return writeerr_str;
			}
		}
// Ref point positions
		for (j = 1; j <= NUM_REF_PNTS; j++)
		{
			for (i = 0; i < GetNumPoints(); i++)
			{
				if (curr_sheet->GetPosition(i) != curr_sheet->GetPosition(i, j))
				{
					if (!handl->WriteChunkHeader(INGL_REFP, GetNumPoints()*4+2))
					{
						return writeerr_str;
					}
					put_big_word(&crd, j);
					if (!handl->Write(&crd, 2))
					{
						return writeerr_str;
					}
					for (i = 0; i < GetNumPoints(); i++)
					{
						put_big_word(&crd, curr_sheet->GetPosition(i, j).x);
						if (!handl->Write(&crd, 2))
						{
							return writeerr_str;
						}
						put_big_word(&crd, curr_sheet->GetPosition(i, j).y);
						if (!handl->Write(&crd, 2))
						{
							return writeerr_str;
						}
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
				if (!handl->WriteChunkHeader(INGL_SYMB, GetNumPoints()))
				{
					return writeerr_str;
				}
				for (i = 0; i < GetNumPoints(); i++)
				{
					if (!handl->Write(&curr_sheet->GetPoint(i).sym, 1))
					{
						return writeerr_str;
					}
				}
				break;
			}
		}
// Point continuity types
		for (i = 0; i < GetNumPoints(); i++)
		{
			if (curr_sheet->GetPoint(i).cont != 0)
			{
				if (!handl->WriteChunkHeader(INGL_TYPE, GetNumPoints()))
				{
					return writeerr_str;
				}
				for (i = 0; i < GetNumPoints(); i++)
				{
					if (!handl->Write(&curr_sheet->GetPoint(i).cont, 1))
					{
						return writeerr_str;
					}
				}
				break;
			}
		}
// Point labels (left or right)
		for (i = 0; i < GetNumPoints(); i++)
		{
			if (curr_sheet->GetPoint(i).GetFlip())
			{
				if (!handl->WriteChunkHeader(INGL_LABL, GetNumPoints()))
				{
					return writeerr_str;
				}
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
					if (!handl->Write(&c, 1))
					{
						return writeerr_str;
					}
				}
				break;
			}
		}
// Continuity text
		for (CC_sheet::ContContainer::const_iterator curranimcont = curr_sheet->animcont.begin(); curranimcont != curr_sheet->animcont.end();
			++curranimcont)
		{
			if (!handl->WriteChunkHeader(INGL_CONT,
				1+(*curranimcont)->GetName().Length()+1+
				(*curranimcont)->GetText().Length()+1))
			{
				return writeerr_str;
			}
			unsigned tnum = (*curranimcont)->GetNum();
			if (!handl->Write(&tnum, 1))
			{
				return writeerr_str;
			}
			if (!handl->WriteStr((*curranimcont)->GetName().utf8_str()))
			{
				return writeerr_str;
			}
			if (!handl->WriteStr((*curranimcont)->GetText().utf8_str()))
			{
				return writeerr_str;
			}
		}
		if (!handl->WriteEnd(INGL_SHET))
		{
			return writeerr_str;
		}
	}

	if (!handl->WriteEnd(INGL_SHOW))
	{
		return writeerr_str;
	}

	delete handl;

	if (!bakfile.IsEmpty())
	{
		wxRemoveFile(bakfile);
	}
	return wxT("");
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


wxString CC_show::Autosave() const
{
	if (!autosave_name.IsEmpty())
	{
		return SaveInternal(autosave_name);
	}
	else
	{
		return wxT("");
	}
}


wxString CC_show::Save(const wxString& filename)
{
	wxString s = SaveInternal(filename);
	if (s.empty())
	{
		if (autosave_name)
		{
			ClearAutosave();
		}
		SetModified(false);
		if (name.CompareTo(filename) != 0)
		{
			UserSetName(filename);
		}
	}
	return s;
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


