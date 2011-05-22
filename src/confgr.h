/*
 * config.h
 * Functions for manipulating configuration Settings
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

#ifndef _CONFGR_H_
#define _CONFGR_H_

#include <wx/string.h>
#include <wx/gdicmn.h>

// forward declare
class wxPalette;
class wxPen;
class wxBrush;
class wxPathList;

#define MAX_SPR_LINES 5
#define MAX_YARD_LINES 53

enum CalChartColors
{
	COLOR_FIELD,
	COLOR_FIELD_DETAIL,
	COLOR_FIELD_TEXT,
	COLOR_POINT,
	COLOR_POINT_TEXT,
	COLOR_POINT_HILIT,
	COLOR_POINT_HILIT_TEXT,
	COLOR_REF_POINT,
	COLOR_REF_POINT_TEXT,
	COLOR_REF_POINT_HILIT,
	COLOR_REF_POINT_HILIT_TEXT,
	COLOR_POINT_ANIM_FRONT,
	COLOR_POINT_ANIM_BACK,
	COLOR_POINT_ANIM_SIDE,
	COLOR_POINT_ANIM_HILIT_FRONT,
	COLOR_POINT_ANIM_HILIT_BACK,
	COLOR_POINT_ANIM_HILIT_SIDE,
	COLOR_POINT_ANIM_COLLISION,
	COLOR_SHAPES,
	COLOR_NUM
};

extern wxPalette *CalChartPalette;

extern const wxPen *CalChartPens[COLOR_NUM];
extern const wxBrush *CalChartBrushes[COLOR_NUM];
extern const wxString ColorNames[COLOR_NUM];
extern const wxString DefaultColors[COLOR_NUM];
extern const int DefaultPenWidth[COLOR_NUM];

// directory and file setup:
extern wxString program_dir;
extern wxString shows_dir;
extern wxString autosave_dir;
extern wxString autosave_dirname;
extern unsigned int autosave_interval;

// page setup and zoom
wxSize GetDefaultSize();
void SetDefaultSize(const wxSize& size);

long GetDefaultZoom();
void SetDefaultZoom(long zoom);

// printing
wxString GetPrintFile();
void SetPrintFile(const wxString& str);
void ClearPrintFile();
wxString GetPrintCmd();
void SetPrintCmd(const wxString& str);
void ClearPrintCmd();
wxString GetPrintOpts();
void SetPrintOpts(const wxString& str);
void ClearPrintOpts();
wxString GetPrintViewCmd();
void SetPrintViewCmd(const wxString& str);
void ClearPrintViewCmd();
wxString GetPrintViewOpts();
void SetPrintViewOpts(const wxString& str);
void ClearPrintViewOpts();

float GetPageWidth();
void SetPageWidth(float f);
void ClearPageWidth();
float GetPageHeight();
void SetPageHeight(float f);
void ClearPageHeight();
float GetPageOffsetX();
void SetPageOffsetX(float f);
void ClearPageOffsetX();
float GetPageOffsetY();
void SetPageOffsetY(float f);
void ClearPageOffsetY();

extern wxString head_font;
extern wxString main_font;
extern wxString number_font;
extern wxString cont_font;
extern wxString bold_font;
extern wxString ital_font;
extern wxString bold_ital_font;
extern float paper_length;
extern float header_size;
extern float yards_size;
extern float text_size;
extern float dot_ratio;
extern float num_ratio;
extern float pline_ratio;
extern float sline_ratio;
extern float cont_ratio;
extern wxString yard_text[MAX_YARD_LINES];
extern wxString spr_line_text[MAX_SPR_LINES];

extern void ReadConfig();
extern wxString ReadConfig(const wxString& path);

void SetConfigColor(size_t selection);
void ClearConfigColor(size_t selection);

extern FILE *OpenFileInDir(const wxString& name, const wxString& modes,
const wxPathList *list = NULL);
extern wxString FullPath(const wxString& path);
extern int ReadDOSline(FILE *fp, wxString& str);

// DEBUG printing facilities
#ifndef NDEBUG
#define DEBUG_LOG(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#else
#define DEBUG_LOG(format, ...)
#endif
#endif
