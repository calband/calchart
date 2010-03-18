/* config.h
 * Definitions for the configuration routines
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

#ifndef _CONFGR_H_
#define _CONFGR_H_

#include <wx/string.h>

#define MAX_SPR_LINES 5
#define MAX_YARD_LINES 53

enum CalChartColors {
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

class wxPalette;
extern wxPalette *CalChartPalette;

class wxPen;
extern wxPen *CalChartPens[COLOR_NUM];
class wxBrush;
extern const wxBrush *CalChartBrushes[COLOR_NUM];

extern wxString program_dir;
extern wxString shows_dir;
extern wxString autosave_dir;
extern wxString autosave_dirname;
extern unsigned int window_default_width;
extern unsigned int window_default_height;
extern unsigned int default_zoom;
extern unsigned int undo_buffer_size;
extern unsigned int autosave_interval;
extern wxString print_file;
extern wxString print_cmd;
extern wxString print_opts;
extern wxString print_view_cmd;
extern wxString print_view_opts;
extern wxString head_font;
extern wxString main_font;
extern wxString number_font;
extern wxString cont_font;
extern wxString bold_font;
extern wxString ital_font;
extern wxString bold_ital_font;
extern float page_width;
extern float page_height;
extern float paper_length;
extern float page_offset_x;
extern float page_offset_y;
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

extern wxString ReadConfig(const wxString& path);
class wxPathList;
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
