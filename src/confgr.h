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
#include <vector>
#include <functional>
#include <map>
#include <array>

// forward declare
class wxPen;
class wxBrush;
class wxPathList;
class wxColour;
class ShowMode;

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
	COLOR_GHOST_POINT,
	COLOR_GHOST_POINT_TEXT,
	COLOR_GHOST_POINT_HLIT,
	COLOR_GHOST_POINT_HLIT_TEXT,
	COLOR_POINT_ANIM_FRONT,
	COLOR_POINT_ANIM_BACK,
	COLOR_POINT_ANIM_SIDE,
	COLOR_POINT_ANIM_HILIT_FRONT,
	COLOR_POINT_ANIM_HILIT_BACK,
	COLOR_POINT_ANIM_HILIT_SIDE,
	COLOR_POINT_ANIM_COLLISION,
	COLOR_POINT_ANIM_COLLISION_WARNING,
	COLOR_SHAPES,
	COLOR_PATHS,
	COLOR_NUM
};

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

// CalChartConfiguration interfaces with the system config and acts as a "cache" for the values.
// On Get, it reads the values from system config, and caches a copy.
// On Set (and clear), it updates it's cache, and puts the command into a write-queue.
// The write-queue needs to be explicitly flushed or the values will be lost
//
// To use a config value, first get the Global config, and then Get_ the value from it:
// auto save_interval = CalChartConfiguration::GetGlobalConfig().Get_AutosaveInterval();
//
// To add a new config value:
// Add DECLARE_CONFIGURATION_FUNCTIONS in the class declaration of the right type; this will make the Get_, Set_ and Clear_
// functions available.  Then in the implementation file, declare IMPLEMENT_CONFIGURATION_FUNCTIONS with the default.
class CalChartConfiguration
{
public:
	static CalChartConfiguration& GetGlobalConfig();
	static void AssignConfig(const CalChartConfiguration& config);

	// explicit flush
	void FlushWriteQueue() const;

private:
	mutable std::map<wxString, std::function<void()>> mWriteQueue;

// macro for declaring configuration Get_, Set_, and Clear_
#define DECLARE_CONFIGURATION_FUNCTIONS( Key, Type ) \
public: \
	Type Get_ ## Key () const ; \
	void Set_ ## Key (const Type& v) ; \
	void Clear_ ## Key () ; \
private: \
	mutable std::pair<bool, Type> m ## Key = { false, Type() };

	DECLARE_CONFIGURATION_FUNCTIONS( AutosaveInterval, long);

	// page setup and zoom
	DECLARE_CONFIGURATION_FUNCTIONS( FieldFrameZoom, double);
	DECLARE_CONFIGURATION_FUNCTIONS( FieldFrameWidth, long);
	DECLARE_CONFIGURATION_FUNCTIONS( FieldFrameHeight, long);

	// printing configurations
	DECLARE_CONFIGURATION_FUNCTIONS( PrintFile, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintCmd, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintOpts, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintViewCmd, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintViewOpts, wxString);

	DECLARE_CONFIGURATION_FUNCTIONS( PageWidth, double);
	DECLARE_CONFIGURATION_FUNCTIONS( PageHeight, double);
	DECLARE_CONFIGURATION_FUNCTIONS( PageOffsetX, double);
	DECLARE_CONFIGURATION_FUNCTIONS( PageOffsetY, double);
	DECLARE_CONFIGURATION_FUNCTIONS( PaperLength, double);

	DECLARE_CONFIGURATION_FUNCTIONS( HeadFont, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( MainFont, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( NumberFont, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( ContFont, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( BoldFont, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( ItalFont, wxString);
	DECLARE_CONFIGURATION_FUNCTIONS( BoldItalFont, wxString);

	DECLARE_CONFIGURATION_FUNCTIONS( HeaderSize, double);
	DECLARE_CONFIGURATION_FUNCTIONS( YardsSize, double);
	DECLARE_CONFIGURATION_FUNCTIONS( TextSize, double);
	DECLARE_CONFIGURATION_FUNCTIONS( DotRatio, double);
	DECLARE_CONFIGURATION_FUNCTIONS( NumRatio, double);
	DECLARE_CONFIGURATION_FUNCTIONS( PLineRatio, double);
	DECLARE_CONFIGURATION_FUNCTIONS( SLineRatio, double);
	DECLARE_CONFIGURATION_FUNCTIONS( ContRatio, double);

	DECLARE_CONFIGURATION_FUNCTIONS( PrintPSModes, long);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintPSLandscape, long);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintPSOverview, long);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintPSDoCont, long);
	DECLARE_CONFIGURATION_FUNCTIONS( PrintPSDoContSheet, long);

	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_X_4, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_Y_4, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_Z_4, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewAngle_4, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewAngle_Z_4, float);

	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_X_5, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_Y_5, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_Z_5, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewAngle_5, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewAngle_Z_5, float);

	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_X_6, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_Y_6, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewPoint_Z_6, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewAngle_6, float);
	DECLARE_CONFIGURATION_FUNCTIONS( OmniViewAngle_Z_6, float);

	DECLARE_CONFIGURATION_FUNCTIONS( AnimationFrameWidth, long);
	DECLARE_CONFIGURATION_FUNCTIONS( AnimationFrameHeight, long);

	DECLARE_CONFIGURATION_FUNCTIONS( AnimationFrameSashPosition, long);
	DECLARE_CONFIGURATION_FUNCTIONS( AnimationFrameOmniAnimation, bool);
	DECLARE_CONFIGURATION_FUNCTIONS( AnimationFrameSplitScreen, bool);
	DECLARE_CONFIGURATION_FUNCTIONS( AnimationFrameSplitVertical, bool);

public:
	// helpers for displaying different config attributes
	std::vector<wxString> GetColorNames() const;
	std::vector<wxString> GetDefaultColors() const;
	std::vector<int> GetDefaultPenWidth() const;
	std::vector<wxString> Get_yard_text_index() const;
	std::vector<wxString> Get_spr_line_text_index() const;

	// Colors
	using ColorWidth_t = std::pair<wxColour, long>;
	mutable std::map<CalChartColors, ColorWidth_t> mColorsAndWidth;
	std::pair<wxBrush, wxPen> Get_CalChartBrushAndPen(CalChartColors c) const;
	void Set_CalChartBrushAndPen(CalChartColors c, const wxBrush& brush, const wxPen& pen);
	void Clear_ConfigColor(size_t selection);

	// Shows
	static const size_t kShowModeValues = 10;
	using ShowModeInfo_t = std::array<long, kShowModeValues>;
	mutable std::map<CalChartShowModes, ShowModeInfo_t> mShowModeInfos;
	ShowModeInfo_t Get_ShowModeInfo(CalChartShowModes which) const;
	void Set_ShowModeInfo(CalChartShowModes which, const ShowModeInfo_t& values);
	void Clear_ShowModeInfo(CalChartShowModes which);
	
	static const size_t kSpringShowModeValues = 21;
	using SpringShowModeInfo_t = std::array<long, kSpringShowModeValues>;
	mutable std::map<CalChartSpringShowModes, SpringShowModeInfo_t> mSpringShowModeInfos;
	SpringShowModeInfo_t Get_SpringShowModeInfo(CalChartSpringShowModes which) const;
	void Set_SpringShowModeInfo(CalChartSpringShowModes which, const SpringShowModeInfo_t& values);
	void Clear_SpringShowModeInfo(CalChartSpringShowModes which);

//#define MAX_SPR_LINES 5
//#define MAX_YARD_LINES 53
//
	// Yard Lines
	static const size_t kYardTextValues = 53;
	mutable std::map<size_t, wxString> mYardTextInfos;
	wxString Get_yard_text(size_t which) const;
	void Set_yard_text(size_t which, const wxString&);
	void Clear_yard_text(size_t which);

	static const size_t kSprLineTextValues = 5;
	mutable std::map<size_t, wxString> mSprLineTextInfos;
	wxString Get_spr_line_text(size_t which) const;
	void Set_spr_line_text(size_t which, const wxString&);
	void Clear_spr_line_text(size_t which);
};


#endif
