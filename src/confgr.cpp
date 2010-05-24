/* confgr.cpp
 * Basic configuration initialization for all systems
 * Config directory is pointed to by environment variable CALCHART_RT
 * This directory contains a file called "config" that sets basic options
 *
 * Modification history:
 * 1-7-95     Garrick Meeker              Created
 * 7-10-95    Garrick Meeker              Converted to C++
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

#include <wx/utils.h>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/palette.h>
#include <wx/pen.h>
#include "show.h"
#include <ctype.h>
#include <string>

#include "confgr.h"
#include "modes.h"

extern ShowModeList *modelist;

const wxString ColorNames[COLOR_NUM] =
{
	wxT("FIELD"),
	wxT("FIELD DETAIL"),
	wxT("FIELD TEXT"),
	wxT("POINT"),
	wxT("POINT TEXT"),
	wxT("HILIT POINT"),
	wxT("HILIT POINT TEXT"),
	wxT("REF POINT"),
	wxT("REF POINT TEXT"),
	wxT("HILIT REF POINT"),
	wxT("HILIT REF POINT TEXT"),
	wxT("ANIM FRONT"),
	wxT("ANIM BACK"),
	wxT("ANIM SIDE"),
	wxT("HILIT ANIM FRONT"),
	wxT("HILIT ANIM BACK"),
	wxT("HILIT ANIM SIDE"),
	wxT("ANIM COLLISION"),
	wxT("SHAPES")
};

const wxString DefaultColors[COLOR_NUM] =
{
	wxT("FOREST GREEN"),
	wxT("WHITE"),
	wxT("BLACK"),
	wxT("WHITE"),
	wxT("BLACK"),
	wxT("YELLOW"),
	wxT("BLACK"),
	wxT("PURPLE"),
	wxT("BLACK"),
	wxT("PURPLE"),
	wxT("BLACK"),
	wxT("WHITE"),
	wxT("YELLOW"),
	wxT("SKY BLUE"),
	wxT("RED"),
	wxT("RED"),
	wxT("RED"),
	wxT("PURPLE"),
	wxT("RED")
};

const int DefaultPenWidth[COLOR_NUM] =
{
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	1,
};

wxPalette *CalChartPalette;
const wxPen *CalChartPens[COLOR_NUM];
const wxBrush *CalChartBrushes[COLOR_NUM];

wxString program_dir;
wxString shows_dir;
wxString autosave_dir;
wxString autosave_dirname(AUTOSAVE_VAR);
unsigned int window_default_width = 600;
unsigned int window_default_height = 450;
unsigned int undo_buffer_size = 50000;
unsigned int autosave_interval = 300;
unsigned int default_zoom = 5;
wxString print_file(wxT("LPT1"));
wxString print_cmd(wxT("lpr"));
wxString print_opts(wxT(""));
wxString print_view_cmd(wxT("ghostview"));
wxString print_view_opts(wxT(""));
wxString head_font(wxT("Helvetica-Bold"));
wxString main_font(wxT("Helvetica"));
wxString number_font(wxT("Helvetica"));
wxString cont_font(wxT("Courier"));
wxString bold_font(wxT("Courier-Bold"));
wxString ital_font(wxT("Courier-Italic"));
wxString bold_ital_font(wxT("Courier-BoldItalic"));
float page_width = 7.5;
float page_height = 10.0;
float paper_length = 11.0;
float page_offset_x = 0.5;
float page_offset_y = 0.5;
float header_size = 3.0;
float yards_size = 1.5;
float text_size = 10.0;
float dot_ratio = 1.0;
float num_ratio = 1.2;
float pline_ratio = 1.0;
float sline_ratio = 1.0;
float cont_ratio = 0.25;
wxString yard_text[MAX_YARD_LINES] =
{
	wxT("N"),
	wxT("M"),
	wxT("L"),
	wxT("K"),
	wxT("J"),
	wxT("I"),
	wxT("H"),
	wxT("G"),
	wxT("F"),
	wxT("E"),
	wxT("D"),
	wxT("C"),
	wxT("B"),
	wxT("A"),
	wxT("-10"),
	wxT("-5"),
	wxT("0"),
	wxT("5"),
	wxT("10"),
	wxT("15"),
	wxT("20"),
	wxT("25"),
	wxT("30"),
	wxT("35"),
	wxT("40"),
	wxT("45"),
	wxT("50"),
	wxT("45"),
	wxT("40"),
	wxT("35"),
	wxT("30"),
	wxT("25"),
	wxT("20"),
	wxT("15"),
	wxT("10"),
	wxT("5"),
	wxT("0"),
	wxT("-5"),
	wxT("-10"),
	wxT("A"),
	wxT("B"),
	wxT("C"),
	wxT("D"),
	wxT("E"),
	wxT("F"),
	wxT("G"),
	wxT("H"),
	wxT("I"),
	wxT("J"),
	wxT("K"),
	wxT("L"),
	wxT("M"),
	wxT("N")
};
wxString spr_line_text[MAX_SPR_LINES] =
{
	wxT("A"),
	wxT("B"),
	wxT("C"),
	wxT("D"),
	wxT("E"),
};

static wxPathList configdirs;
static wxString runtime_dir;

wxString ReadConfig(const wxString& path)
{
	FILE *fp;
	int i;
	wxString com_buf;
	CC_coord bord1(INT2COORD(8),INT2COORD(8)), bord2(INT2COORD(8),INT2COORD(8));
	CC_coord siz, off;
	wxString retmsg;
	unsigned short whash, ehash;
	short mode_steps_x, mode_steps_y, mode_steps_w, mode_steps_h;
	short eps_stage_x, eps_stage_y, eps_stage_w, eps_stage_h;
	short eps_field_x, eps_field_y, eps_field_w, eps_field_h;
	short eps_text_left, eps_text_right, eps_text_top, eps_text_bottom;
	unsigned char which_spr_yards;
	wxPathList tmp_configdirs;

	program_dir = wxGetCwd();

// Get default path
	runtime_dir = FullPath(path);

// Set search path for files
	tmp_configdirs.AddEnvList(wxT("CALCHART_RT"));
	tmp_configdirs.Add(runtime_dir);

	fp = OpenFileInDir(wxT("config"), wxT("r"), &tmp_configdirs);
	if (fp == NULL)
	{
		retmsg = wxT("Unable to open config file.  Using default values.\n");
	}
	else
	{
		while (!feof(fp))
		{
			ReadDOSline(fp, com_buf);

			if (com_buf.empty()) continue;
/* check for comment */
			if (com_buf[0] == '#') continue;
			if (com_buf == wxT("PROGRAM_DIR"))
			{
				ReadDOSline(fp, program_dir);
				continue;
			}
			if (com_buf == wxT("SHOWS_DIR"))
			{
				ReadDOSline(fp, shows_dir);
				continue;
			}
			if (com_buf == wxT("AUTOSAVE_DIR"))
			{
				ReadDOSline(fp, autosave_dirname);
				continue;
			}
			if (com_buf == wxT("RUNTIME_DIR"))
			{
				ReadDOSline(fp, runtime_dir);
				continue;
			}
			if (com_buf == wxT("WINDOW_WIDTH"))
			{
				fscanf(fp, " %d \n", &window_default_width);
				continue;
			}
			if (com_buf == wxT("WINDOW_HEIGHT"))
			{
				fscanf(fp, " %d \n", &window_default_height);
				continue;
			}
			if (com_buf == wxT("DEFAULT_ZOOM"))
			{
				fscanf(fp, " %d \n", &default_zoom);
				continue;
			}
			if (com_buf == wxT("UNDO_BUF_SIZE"))
			{
				fscanf(fp, " %d \n", &undo_buffer_size);
				continue;
			}
			if (com_buf == wxT("AUTOSAVE_INTERVAL"))
			{
				fscanf(fp, " %d \n", &autosave_interval);
				continue;
			}
			if (com_buf == wxT("PRINT_FILE"))
			{
				ReadDOSline(fp, print_file);
				continue;
			}
			if (com_buf == wxT("PRINT_CMD"))
			{
				ReadDOSline(fp, print_cmd);
				continue;
			}
			if (com_buf == wxT("PRINT_OPTS"))
			{
				ReadDOSline(fp, print_opts);
				continue;
			}
			if (com_buf == wxT("PRINT_VIEW_CMD"))
			{
				ReadDOSline(fp, print_view_cmd);
				continue;
			}
			if (com_buf == wxT("PRINT_VIEW_OPTS"))
			{
				ReadDOSline(fp, print_view_opts);
				continue;
			}
			if (com_buf == wxT("DEFINE_STANDARD_MODE"))
			{
				ReadDOSline(fp, com_buf);
				fscanf(fp, " %hu %hu \n", &whash, &ehash);
				fscanf(fp, " %hu %hu %hu %hu \n",
					&bord1.x, &bord1.y, &bord2.x, &bord2.y);
				bord1.x = INT2COORD(bord1.x);
				bord1.y = INT2COORD(bord1.y);
				bord2.x = INT2COORD(bord2.x);
				bord2.y = INT2COORD(bord2.y);
				fscanf(fp, " %hd %hd %hd %hd \n", &mode_steps_x, &mode_steps_y,
					&mode_steps_w, &mode_steps_h);
				siz.x = INT2COORD(mode_steps_w);
				siz.y = INT2COORD(mode_steps_h);
				off.x = INT2COORD(-mode_steps_x);
				off.y = INT2COORD(-mode_steps_y);
				modelist->Add(new ShowModeStandard(com_buf, bord1, bord2,
					siz, off, whash, ehash));
				continue;
			}
			if (com_buf == wxT("DEFINE_SPRSHOW_MODE"))
			{
				wxString mode_name;
				ReadDOSline(fp, mode_name);
				ReadDOSline(fp, com_buf);
				fscanf(fp, " %c \n", &which_spr_yards);
				if (which_spr_yards <= '9')
				{
					which_spr_yards -= '0';
				}
				else
				{
					which_spr_yards = toupper(which_spr_yards) - 'A' + 10;
				}
				fscanf(fp, " %hu %hu %hu %hu \n",
					&bord1.x, &bord1.y, &bord2.x, &bord2.y);
				bord1.x = INT2COORD(bord1.x);
				bord1.y = INT2COORD(bord1.y);
				bord2.x = INT2COORD(bord2.x);
				bord2.y = INT2COORD(bord2.y);
				fscanf(fp, " %hd %hd %hd %hd \n", &mode_steps_x, &mode_steps_y,
					&mode_steps_w, &mode_steps_h);
				fscanf(fp, " %hd %hd %hd %hd \n", &eps_stage_x, &eps_stage_y,
					&eps_stage_w, &eps_stage_h);
				fscanf(fp, " %hd %hd %hd %hd \n", &eps_field_x, &eps_field_y,
					&eps_field_w, &eps_field_h);
				fscanf(fp, " %hd %hd %hd %hd \n", &eps_text_left, &eps_text_right,
					&eps_text_top, &eps_text_bottom);
				modelist->Add(new ShowModeSprShow(mode_name, bord1, bord2,
					which_spr_yards, com_buf,
					mode_steps_x, mode_steps_y,
					mode_steps_w, mode_steps_h,
					eps_stage_x, eps_stage_y,
					eps_stage_w, eps_stage_h,
					eps_field_x, eps_field_y,
					eps_field_w, eps_field_h,
					eps_text_left, eps_text_right,
					eps_text_top, eps_text_bottom));
				continue;
			}
			if (com_buf == wxT("PRINT_PAGE_WIDTH"))
			{
				fscanf(fp, " %f \n", &page_width);
				continue;
			}
			if (com_buf == wxT("PRINT_PAGE_HEIGHT"))
			{
				fscanf(fp, " %f \n", &page_height);
				continue;
			}
			if (com_buf == wxT("PRINT_PAPER_LENGTH"))
			{
				fscanf(fp, " %f \n", &paper_length);
				continue;
			}
			if (com_buf == wxT("PRINT_LEFT_MARGIN"))
			{
				fscanf(fp, " %f \n", &page_offset_x);
				continue;
			}
			if (com_buf == wxT("PRINT_TOP_MARGIN"))
			{
				fscanf(fp, " %f \n", &page_offset_y);
				continue;
			}
			if (com_buf == wxT("PRINT_HEADER_SIZE"))
			{
				fscanf(fp, " %f \n", &header_size);
				continue;
			}
			if (com_buf == wxT("PRINT_YARDS_SIZE"))
			{
				fscanf(fp, " %f \n", &yards_size);
				continue;
			}
			if (com_buf == wxT("PRINT_TEXT_SIZE"))
			{
				fscanf(fp, " %f \n", &text_size);
				continue;
			}
			if (com_buf == wxT("PRINT_DOT_RATIO"))
			{
				fscanf(fp, " %f \n", &dot_ratio);
				continue;
			}
			if (com_buf == wxT("PRINT_NUM_RATIO"))
			{
				fscanf(fp, " %f \n", &num_ratio);
				continue;
			}
			if (com_buf == wxT("PRINT_PLAIN_LINE"))
			{
				fscanf(fp, " %f \n", &pline_ratio);
				continue;
			}
			if (com_buf == wxT("PRINT_SOLID_LINE"))
			{
				fscanf(fp, " %f \n", &sline_ratio);
				continue;
			}
			if (com_buf == wxT("PRINT_CONT_RATIO"))
			{
				fscanf(fp, " %f \n", &cont_ratio);
				continue;
			}
			if (com_buf == wxT("PRINT_HEADER_FONT"))
			{
				ReadDOSline(fp, head_font);
				continue;
			}
			if (com_buf == wxT("PRINT_NUMBER_FONT"))
			{
				ReadDOSline(fp, number_font);
				continue;
			}
			if (com_buf == wxT("PRINT_ANNO_FONT"))
			{
				ReadDOSline(fp, main_font);
				continue;
			}
			if (com_buf == wxT("PRINT_FONT"))
			{
				ReadDOSline(fp, cont_font);
				continue;
			}
			if (com_buf == wxT("PRINT_BOLD_FONT"))
			{
				ReadDOSline(fp, bold_font);
				continue;
			}
			if (com_buf == wxT("PRINT_ITALIC_FONT"))
			{
				ReadDOSline(fp, ital_font);
				continue;
			}
			if (com_buf == wxT("PRINT_BOLD_ITALIC_FONT"))
			{
				ReadDOSline(fp, bold_ital_font);
				continue;
			}
			if (com_buf == wxT("PRINT_YARDS"))
			{
				char yardbuf[16];
				for (i=0; i<MAX_YARD_LINES; i++)
				{
					fscanf(fp, " %s ", yardbuf);
					yard_text[i] = wxString::FromUTF8(yardbuf);
				}
				continue;
			}
			if (com_buf == wxT("PRINT_SPR_LINES"))
			{
				char yardbuf[16];
				for (i=0; i<MAX_SPR_LINES; i++)
				{
					fscanf(fp, " %s ", yardbuf);
					spr_line_text[i] = wxString::FromUTF8(yardbuf);
				}
				continue;
			}
			if (com_buf == wxT("COLOR"))
			{
				int mono, hollow, width, dotted;
				char coltype_buf[256];
				char colname_buf[256];

				fscanf(fp, " \"%[^\"]\" \"%[^\"]\" %d %d %d %d ",
					coltype_buf, colname_buf, &mono, &hollow, &width, &dotted);
				wxString coltype_str(wxString::FromUTF8(coltype_buf));
				for (i=0; i<COLOR_NUM; i++)
				{
					if (coltype_str == ColorNames[i])
					{
						unsigned r, g, b;
						wxColour c;

						if (sscanf(colname_buf, " %u %u %u ", &r, &g, &b) == 3)
						{
							c = wxColour(r, g, b);
						}
						else
						{
							c = wxColour(wxString::FromUTF8(colname_buf));
						}
						CalChartPens[i] = wxThePenList->FindOrCreatePen(c, DefaultPenWidth[i], wxSOLID);
						CalChartBrushes[i] = wxTheBrushList->FindOrCreateBrush(c,
							wxSOLID);
						break;
					}
				}
				if (i == COLOR_NUM)
				{
					wxString tmpbuf;
					tmpbuf.Printf(wxT("Warning: color '%hs' in config file is not recognized.\n"),
						coltype_buf);
					retmsg = tmpbuf;
				}
				continue;
			}
			if (!retmsg)
			{
				wxString tmpbuf;
				tmpbuf.Printf(wxT("Warning: '%s' is not recognized in config file.\n"),
					com_buf.c_str());
				retmsg = tmpbuf;
			}
		}
		fclose(fp);
	}

	if (autosave_dirname[0] == '$')
	{
		const char *d;
		std::string s(autosave_dirname.utf8_str());
		if ((d = getenv(s.c_str()+1)) != NULL)
		{
			autosave_dir = wxString::FromUTF8(d);
		}
		else
		{
			autosave_dir = AUTOSAVE_DIR;
		}
	}
	else
	{
		autosave_dir = autosave_dirname;
	}

	if (!modelist->Empty())
	{
// No modes were defined.  Add a default
		modelist->Add(new ShowModeStandard(wxT("Standard"), bord1, bord2,
			DEF_HASH_W, DEF_HASH_E));
	}

	{
		int j;
		int n = 0;
		const wxColour *c1, *c2;

		for (i=0; i<COLOR_NUM; i++)
		{
			unsigned r, g, b;
			wxColour c;

			if (CC_sscanf(DefaultColors[i], wxT(" %u %u %u "), &r, &g, &b) == 3)
			{
				c = wxColour(r, g, b);
			}
			else
			{
				c = wxColour(DefaultColors[i]);
			}
			if (CalChartPens[i] == NULL)
			{
				CalChartPens[i] = wxThePenList->FindOrCreatePen(c, DefaultPenWidth[i], wxSOLID);
			}
			if (CalChartBrushes[i] == NULL)
			{
				CalChartBrushes[i] = wxTheBrushList->FindOrCreateBrush(c, wxSOLID);
			}
		}
		unsigned char rd[COLOR_NUM], gd[COLOR_NUM], bd[COLOR_NUM];
		CalChartPalette = new wxPalette();
		for (i = 0; i < COLOR_NUM; i++)
		{
			for (j = 0; j < i; j++)
			{
				c1 = &CalChartPens[i]->GetColour();
				if (j >= 0)
				{
					c2 = &CalChartPens[j]->GetColour();
				}
				else
				{
					if (j < -1)
					{
						c2 = wxBLACK;
					}
					else
					{
						c2 = wxWHITE;
					}
				}
				if ((c1->Red() == c2->Red()) &&
					(c1->Green() == c2->Green()) &&
					(c1->Blue() == c2->Blue()))
				{
					break;
				}
			}
			if (i == j)
			{
				rd[n] = CalChartPens[i]->GetColour().Red();
				gd[n] = CalChartPens[i]->GetColour().Green();
				bd[n] = CalChartPens[i]->GetColour().Blue();
				n++;
			}
		}
		CalChartPalette->Create(n, rd, gd, bd);
	}
// Set search path for files
	configdirs.AddEnvList(wxT("CALCHART_RT"));
	configdirs.Add(runtime_dir);

	return retmsg;
}


// Open a file in the specified dir.
FILE *OpenFileInDir(const wxString& name, const wxString& modes, const wxPathList *list)
{
	FILE *fp;
	wxString fullpath;

	if (!list) list = &configdirs;
	fullpath = list->FindValidPath(name);
	if (fullpath.empty()) return NULL;

	fp = CC_fopen(fullpath.fn_str(), modes.fn_str());
	return fp;
}


wxString FullPath(const wxString& path)
{
	if (wxIsAbsolutePath(path))
	{
		return path;
	}
	else
	{
// make into a full path
		wxString newpath(wxGetCwd());
		newpath.Append(PATH_SEPARATOR);
		newpath.Append(path);
		return newpath;
	}
}


int ReadDOSline(FILE *fp, wxString& str)
{
	char buf[1024];
	if (fgets(buf, sizeof(buf), fp) == NULL)
		return 0;
	int c = strlen(buf);
// chomp like, keep removing \r and \n from the end of a line
	while(c && ((buf[c-1] == '\r') || (buf[c-1] == '\n')))
	{
		buf[c-1] = '\0';
		c--;
	}
	str = wxString::FromUTF8(buf);
	return c;
}
