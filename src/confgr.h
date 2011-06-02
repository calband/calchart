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

enum CalChartShowModes
{
	STANDARD,
	FULL_FIELD,
	TUNNEL,
	OLD_FIELD,
	PRO_FIELD,
	SHOWMODE_NUM
};

extern const wxString kShowModeStrings[SHOWMODE_NUM];

enum CalChartSpringShowModes
{
	ZELLERBACH,
	SPRINGSHOWMODE_NUM
};

extern const wxString kSpringShowModeStrings[SPRINGSHOWMODE_NUM];

// macro for declaring configuration settings
#define DECLARE_CONFIGURATION_FUNCTIONS( Key, Type ) \
Type GetConfiguration_ ## Key () ; \
void SetConfiguration_ ## Key (const Type& v) ; \
void ClearConfiguration_ ## Key () ;

DECLARE_CONFIGURATION_FUNCTIONS( AutosaveDir, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( AutosaveInterval, long);

// page setup and zoom
DECLARE_CONFIGURATION_FUNCTIONS( MainFrameZoom, long);
DECLARE_CONFIGURATION_FUNCTIONS( MainFrameWidth, long);
DECLARE_CONFIGURATION_FUNCTIONS( MainFrameHeight, long);

// printing configurations
DECLARE_CONFIGURATION_FUNCTIONS( PrintFile, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( PrintCmd, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( PrintOpts, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( PrintViewCmd, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( PrintViewOpts, wxString);

DECLARE_CONFIGURATION_FUNCTIONS( PageWidth, float);
DECLARE_CONFIGURATION_FUNCTIONS( PageHeight, float);
DECLARE_CONFIGURATION_FUNCTIONS( PageOffsetX, float);
DECLARE_CONFIGURATION_FUNCTIONS( PageOffsetY, float);
DECLARE_CONFIGURATION_FUNCTIONS( PaperLength, float);

DECLARE_CONFIGURATION_FUNCTIONS( HeadFont, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( MainFont, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( NumberFont, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( ContFont, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( BoldFont, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( ItalFont, wxString);
DECLARE_CONFIGURATION_FUNCTIONS( BoldItalFont, wxString);

DECLARE_CONFIGURATION_FUNCTIONS( HeaderSize, float);
DECLARE_CONFIGURATION_FUNCTIONS( YardsSize, float);
DECLARE_CONFIGURATION_FUNCTIONS( TextSize, float);
DECLARE_CONFIGURATION_FUNCTIONS( DotRatio, float);
DECLARE_CONFIGURATION_FUNCTIONS( NumRatio, float);
DECLARE_CONFIGURATION_FUNCTIONS( PLineRatio, float);
DECLARE_CONFIGURATION_FUNCTIONS( SLineRatio, float);
DECLARE_CONFIGURATION_FUNCTIONS( ContRatio, float);

extern wxString yard_text[MAX_YARD_LINES];
extern const wxString yard_text_index[MAX_YARD_LINES];
extern wxString spr_line_text[MAX_SPR_LINES];
extern const wxString spr_line_text_index[MAX_SPR_LINES];

extern void ReadConfig();

const size_t kShowModeValues = 10;
void GetConfigurationShowMode(size_t which, long values[kShowModeValues]);
void SetConfigurationShowMode(size_t which, const long values[kShowModeValues]);
void ClearConfigurationShowMode(size_t which);

const size_t kSpringShowModeValues = 21;
void GetConfigurationSpringShowMode(size_t which, long values[kSpringShowModeValues]);
void SetConfigurationSpringShowMode(size_t which, const long values[kSpringShowModeValues]);
void ClearConfigurationSpringShowMode(size_t which);

void SetConfigColor(size_t selection);
void ClearConfigColor(size_t selection);

// TODO: Setup linenumbers
void SetConfigShowYardline();
void ClearConfigShowYardline();
void SetConfigSpringShowYardline();
void ClearConfigSpringShowYardline();

extern int ReadDOSline(FILE *fp, wxString& str);

// DEBUG printing facilities
#ifndef NDEBUG
#define DEBUG_LOG(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#else
#define DEBUG_LOG(format, ...)
#endif
#endif
