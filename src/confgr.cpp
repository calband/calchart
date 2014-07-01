/*
 * confgr.cpp
 * Basic configuration initialization for all systems
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

#include <wx/utils.h>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/pen.h>
#include <wx/config.h>
#include <wx/confbase.h>
#include <wx/file.h>
#include <ctype.h>
#include <string>
#include <array>

#include "calchartapp.h"
#include "confgr.h"
#include "modes.h"
#include "cc_omniview_constants.h"

const std::tuple<wxString, wxString, int> ColorInfo[COLOR_NUM] =
{
	{ wxT("FIELD"),					wxT("FOREST GREEN"),	1 },
	{ wxT("FIELD DETAIL"),			wxT("WHITE"),			1 },
	{ wxT("FIELD TEXT"),			wxT("BLACK"),			1 },
	{ wxT("POINT"),					wxT("WHITE"),			1 },
	{ wxT("POINT TEXT"),			wxT("BLACK"),			1 },
	{ wxT("HILIT POINT"),			wxT("YELLOW"),			1 },
	{ wxT("HILIT POINT TEXT"),		wxT("BLACK"),			1 },
	{ wxT("REF POINT"),				wxT("PURPLE"),			1 },
	{ wxT("REF POINT TEXT"),		wxT("BLACK"),			1 },
	{ wxT("HILIT REF POINT"),		wxT("PURPLE"),			1 },
	{ wxT("HILIT REF POINT TEXT"),	wxT("BLACK"),			1 },
	{ wxT("ANIM FRONT"),			wxT("WHITE"),			1 },
	{ wxT("ANIM BACK"),				wxT("YELLOW"),			1 },
	{ wxT("ANIM SIDE"),				wxT("SKY BLUE"),		1 },
	{ wxT("HILIT ANIM FRONT"),		wxT("RED"),				1 },
	{ wxT("HILIT ANIM BACK"),		wxT("RED"),				1 },
	{ wxT("HILIT ANIM SIDE"),		wxT("RED"),				1 },
	{ wxT("ANIM COLLISION"),		wxT("PURPLE"),			1 },
	{ wxT("CONTINUITY PATHS"),		wxT("RED"),				1 },
	{ wxT("SHAPES"),				wxT("ORANGE"),			2 },
};

CalChartConfiguration& GetConfig()
{
	static CalChartConfiguration sconfig;
	return sconfig;
}

// constants for behavior:
// Autosave

template <typename T>
T GetConfigValue(const wxString& key, const T& def)
{
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/CalChart"));
	T value = def;
	config->Read(key, &value);
	return value;
}

// default value need to check if we need to set a value
template <typename T>
void SetConfigValue(const wxString& key, const T& value, const T& def)
{
	// don't write if we don't have to
	if (GetConfigValue<T>(key, def) == value)
		return;
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/CalChart"));
	config->Write(key, value);
	config->Flush();
}

template <typename T>
void ClearConfigValue(const wxString& key)
{
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/CalChart"));
	config->DeleteEntry(key);
	config->Flush();
}

// printing controls
wxString yard_text[MAX_YARD_LINES] =
{
	wxT("N"), wxT("M"), wxT("L"), wxT("K"), wxT("J"), wxT("I"), wxT("H"), wxT("G"), wxT("F"), wxT("E"), wxT("D"), wxT("C"), wxT("B"), wxT("A"),
	wxT("-10"), wxT("-5"), wxT("0"), wxT("5"), wxT("10"), wxT("15"), wxT("20"), wxT("25"), wxT("30"), wxT("35"), wxT("40"), wxT("45"), wxT("50"),
	wxT("45"), wxT("40"), wxT("35"), wxT("30"), wxT("25"), wxT("20"), wxT("15"), wxT("10"), wxT("5"), wxT("0"), wxT("-5"), wxT("-10"),
	wxT("A"), wxT("B"), wxT("C"), wxT("D"), wxT("E"), wxT("F"), wxT("G"), wxT("H"), wxT("I"), wxT("J"), wxT("K"), wxT("L"), wxT("M"), wxT("N")
};

const wxString yard_text_index[MAX_YARD_LINES] =
{
	wxT("N"), wxT("M"), wxT("L"), wxT("K"), wxT("J"), wxT("I"), wxT("H"), wxT("G"), wxT("F"), wxT("E"), wxT("D"), wxT("C"), wxT("B"), wxT("A"),
	wxT("-10"), wxT("-5"), wxT("0"), wxT("5"), wxT("10"), wxT("15"), wxT("20"), wxT("25"), wxT("30"), wxT("35"), wxT("40"), wxT("45"), wxT("50"),
	wxT("45"), wxT("40"), wxT("35"), wxT("30"), wxT("25"), wxT("20"), wxT("15"), wxT("10"), wxT("5"), wxT("0"), wxT("-5"), wxT("-10"),
	wxT("A"), wxT("B"), wxT("C"), wxT("D"), wxT("E"), wxT("F"), wxT("G"), wxT("H"), wxT("I"), wxT("J"), wxT("K"), wxT("L"), wxT("M"), wxT("N")
};

static bool yard_text_valid = false;

wxString spr_line_text[MAX_SPR_LINES] =
{
	wxT("A"), wxT("B"), wxT("C"), wxT("D"), wxT("E"),
};

const wxString spr_line_text_index[MAX_SPR_LINES] =
{
	wxT("A"), wxT("B"), wxT("C"), wxT("D"), wxT("E"),
};

static bool spr_line_yard_text_valid = false;

#define IMPLEMENT_CONFIGURATION_FUNCTIONS( KeyName, Type, TheValue ) \
static const wxString k ## KeyName ## Key = wxT( #KeyName ); \
static const Type k ## KeyName ## Value = (TheValue); \
Type CalChartConfiguration::Get_ ## KeyName () const { return GetConfigValue<Type>( k ## KeyName ## Key, k ## KeyName ## Value); } \
void CalChartConfiguration::Set_ ## KeyName (const Type& v) { return SetConfigValue<Type>( k ## KeyName ## Key, v, k ## KeyName ## Value); } \
void CalChartConfiguration::Clear_ ## KeyName () { return ClearConfigValue<Type>( k ## KeyName ## Key ); }


IMPLEMENT_CONFIGURATION_FUNCTIONS( AutosaveInterval, long, 60);

IMPLEMENT_CONFIGURATION_FUNCTIONS( FieldFrameZoom, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS( FieldFrameWidth, long, 600);
IMPLEMENT_CONFIGURATION_FUNCTIONS( FieldFrameHeight, long, 450);

// printing
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintFile, wxString, wxT("LPT1"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintCmd, wxString, wxT("lpr"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintOpts, wxString, wxT(""));
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintViewCmd, wxString, wxT("ghostview"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintViewOpts, wxString, wxT(""));

IMPLEMENT_CONFIGURATION_FUNCTIONS( PageWidth, double, 7.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PageHeight, double, 10.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PageOffsetX, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PageOffsetY, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PaperLength, double, 11.0);

IMPLEMENT_CONFIGURATION_FUNCTIONS( HeadFont, wxString, wxT("Palatino-Bold"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFont, wxString, wxT("Helvetica"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( NumberFont, wxString, wxT("Helvetica-Bold"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( ContFont, wxString, wxT("Courier"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( BoldFont, wxString, wxT("Courier-Bold"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( ItalFont, wxString, wxT("Courier-Italic"));
IMPLEMENT_CONFIGURATION_FUNCTIONS( BoldItalFont, wxString, wxT("Courier-BoldItalic"));

IMPLEMENT_CONFIGURATION_FUNCTIONS( HeaderSize, double, 3.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( YardsSize, double, 1.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS( TextSize, double, 10.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( DotRatio, double, 0.9);
IMPLEMENT_CONFIGURATION_FUNCTIONS( NumRatio, double, 1.35);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS( SLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS( ContRatio, double, 0.2);

IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintPSModes, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintPSLandscape, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintPSOverview, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintPSDoCont, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS( PrintPSDoContSheet, long, 0);

IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_X_4, float, kViewPoint_x_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_Y_4, float, kViewPoint_y_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_Z_4, float, kViewPoint_z_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewAngle_4, float, kViewAngle_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewAngle_Z_4, float, kViewAngle_z_1);

IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_X_5, float, kViewPoint_x_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_Y_5, float, kViewPoint_y_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_Z_5, float, kViewPoint_z_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewAngle_5, float, kViewAngle_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewAngle_Z_5, float, kViewAngle_z_2);

IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_X_6, float, kViewPoint_x_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_Y_6, float, kViewPoint_y_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewPoint_Z_6, float, kViewPoint_z_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewAngle_6, float, kViewAngle_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS( OmniViewAngle_Z_6, float, kViewAngle_z_3);

IMPLEMENT_CONFIGURATION_FUNCTIONS( AnimationFrameWidth, long, 600);
IMPLEMENT_CONFIGURATION_FUNCTIONS( AnimationFrameHeight, long, 450);

IMPLEMENT_CONFIGURATION_FUNCTIONS( AnimationFrameSashPosition, long, 100);
IMPLEMENT_CONFIGURATION_FUNCTIONS( AnimationFrameOmniAnimation, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS( AnimationFrameSplitScreen, bool, true);
IMPLEMENT_CONFIGURATION_FUNCTIONS( AnimationFrameSplitVertical, bool, false);

// OBSOLETE Settigns
// "MainFrameZoom" now obsolete with version post 3.2, use "MainFrameZoom2"
//IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameZoom, float, 0.5);
// "MainFrameZoom", "MainFrameWidth", "MainFrameHeight" now obsolete with version post 3.3.1, use Field versions
//IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameZoom2, float, 0.5);
//IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameWidth, long, 600);
//IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameHeight, long, 450);


///// Show mode configuration /////

const wxString kShowModeStrings[SHOWMODE_NUM] =
{
	wxT("Standard"),
	wxT("Full Field"),
	wxT("Tunnel"),
	wxT("Old Field"),
	wxT("Pro Field")
};

// What values mean:
// whash ehash (steps from west sideline)
// left top right bottom (border in steps)
// x y w h (region of the field to use, in steps)
const std::vector<long> kShowModeDefaultValues[SHOWMODE_NUM] =
{
	{ 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 },
	{ 32, 52, 8, 8, 8, 8, -96, -42, 192, 84 },
	{ 32, 52, 8, 8, 8, 8, 16, -42, 192, 84 },
	{ 28, 52, 8, 8, 8, 8, -80, -42, 160, 84 },
	{ 36, 48, 8, 8, 8, 8, -80, -42, 160, 84 }
};

const wxString kSpringShowModeStrings[SPRINGSHOWMODE_NUM] =
{
	wxT("Zellerbach"),
};

// what values mean
// X (hex digit of which yard lines to print:
//   8 = left, 4 = right, 2 = above, 1 = below)
// left top right bottom (border in steps)
// x y w h (region of the field to use, in steps)
// x y w h (size of stage EPS file as per BoundingBox)
// x y w h (where to put the field on the stage, depends on the EPS file)
// l r t b (location of yard line text's inside edge, depends on the EPS file)
const std::vector<long> kSpringShowModeDefaultValues[SPRINGSHOWMODE_NUM] =
{
	{ 0xD, 8, 8, 8, 8, -16, -30, 32, 28, 0, 0, 571, 400, 163, 38, 265, 232, 153, 438, 270, 12 }
};

std::vector<long>
CalChartConfiguration::GetConfigurationShowMode(size_t which)
{
	std::vector<long> values = kShowModeDefaultValues[which];

	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/SHOWMODES"));
	if (config->Exists(kShowModeStrings[which]))
	{
		config->SetPath(kShowModeStrings[which]);
		values[0] = config->Read(wxT("whash"), values[0]);
		values[1] = config->Read(wxT("ehash"), values[1]);
		values[2] = config->Read(wxT("bord1_x"), values[2]);
		values[3] = config->Read(wxT("bord1_y"), values[3]);
		values[4] = config->Read(wxT("bord2_x"), values[4]);
		values[5] = config->Read(wxT("bord2_y"), values[5]);
		values[6] = config->Read(wxT("size_x"), values[6]);
		values[7] = config->Read(wxT("size_y"), values[7]);
		values[8] = config->Read(wxT("offset_x"), values[8]);
		values[9] = config->Read(wxT("offset_y"), values[9]);
	}
	return values;
}

std::unique_ptr<ShowMode>
CalChartConfiguration::GetMode(const wxString& which)
{
	auto iter = std::find(std::begin(kShowModeStrings), std::end(kShowModeStrings), which);
	if (iter != std::end(kShowModeStrings))
	{
		return CreateShowMode(which, GetConfig().GetConfigurationShowMode(std::distance(std::begin(kShowModeStrings), iter)));
	}
	iter = std::find(std::begin(kSpringShowModeStrings), std::end(kSpringShowModeStrings), which);
	if (iter != std::end(kSpringShowModeStrings))
	{
		return CreateSpringShowMode(which, GetConfig().GetConfigurationSpringShowMode(std::distance(std::begin(kSpringShowModeStrings), iter)));
	}
	return {};
}

std::vector<long>
CalChartConfiguration::GetConfigurationSpringShowMode(size_t which)
{
	std::vector<long> values = kSpringShowModeDefaultValues[which];

	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/SPRINGSHOWMODES"));
	if (config->Exists(kSpringShowModeStrings[which]))
	{
		config->SetPath(kSpringShowModeStrings[which]);
		values[0] = config->Read(wxT("which_spr_yards"), values[0]);
		values[1] = config->Read(wxT("bord1_x"), values[1]);
		values[2] = config->Read(wxT("bord1_y"), values[2]);
		values[3] = config->Read(wxT("bord2_x"), values[3]);
		values[4] = config->Read(wxT("bord2_y"), values[4]);

		values[5] = config->Read(wxT("mode_steps_x"), values[5]);
		values[6] = config->Read(wxT("mode_steps_y"), values[6]);
		values[7] = config->Read(wxT("mode_steps_w"), values[7]);
		values[8] = config->Read(wxT("mode_steps_h"), values[8]);
		values[9] = config->Read(wxT("eps_stage_x"), values[9]);
		values[10] = config->Read(wxT("eps_stage_y"), values[10]);
		values[11] = config->Read(wxT("eps_stage_w"), values[11]);
		values[12] = config->Read(wxT("eps_stage_h"), values[12]);
		values[13] = config->Read(wxT("eps_field_x"), values[13]);
		values[14] = config->Read(wxT("eps_field_y"), values[14]);
		values[15] = config->Read(wxT("eps_field_w"), values[15]);
		values[16] = config->Read(wxT("eps_field_h"), values[16]);
		values[17] = config->Read(wxT("eps_text_left"), values[17]);
		values[18] = config->Read(wxT("eps_text_right"), values[18]);
		values[19] = config->Read(wxT("eps_text_top"), values[19]);
		values[20] = config->Read(wxT("eps_text_bottom"), values[20]);
	}
	return values;
}

void
CalChartConfiguration::SetConfigurationShowMode(size_t which, const std::vector<long>& values)
{
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/SHOWMODES"));
	config->SetPath(kShowModeStrings[which]);
	config->Write(wxT("whash"), values[0]);
	config->Write(wxT("ehash"), values[1]);
	config->Write(wxT("bord1_x"), values[2]);
	config->Write(wxT("bord1_y"), values[3]);
	config->Write(wxT("bord2_x"), values[4]);
	config->Write(wxT("bord2_y"), values[5]);
	config->Write(wxT("size_x"), values[6]);
	config->Write(wxT("size_y"), values[7]);
	config->Write(wxT("offset_x"), values[8]);
	config->Write(wxT("offset_y"), values[9]);
	config->Flush();
}

void
CalChartConfiguration::SetConfigurationSpringShowMode(size_t which, const std::vector<long>& values)
{
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/SPRINGSHOWMODES"));
	config->SetPath(kSpringShowModeStrings[which]);
	config->Write(wxT("which_spr_yards"), values[0]);
	config->Write(wxT("bord1_x"), values[1]);
	config->Write(wxT("bord1_y"), values[2]);
	config->Write(wxT("bord2_x"), values[3]);
	config->Write(wxT("bord2_y"), values[4]);

	config->Write(wxT("mode_steps_x"), values[5]);
	config->Write(wxT("mode_steps_y"), values[6]);
	config->Write(wxT("mode_steps_w"), values[7]);
	config->Write(wxT("mode_steps_h"), values[8]);
	config->Write(wxT("eps_stage_x"), values[9]);
	config->Write(wxT("eps_stage_y"), values[10]);
	config->Write(wxT("eps_stage_w"), values[11]);
	config->Write(wxT("eps_stage_h"), values[12]);
	config->Write(wxT("eps_field_x"), values[13]);
	config->Write(wxT("eps_field_y"), values[14]);
	config->Write(wxT("eps_field_w"), values[15]);
	config->Write(wxT("eps_field_h"), values[16]);
	config->Write(wxT("eps_text_left"), values[17]);
	config->Write(wxT("eps_text_right"), values[18]);
	config->Write(wxT("eps_text_top"), values[19]);
	config->Write(wxT("eps_text_bottom"), values[20]);
	config->Flush();
}

void
CalChartConfiguration::ClearConfigurationShowMode(size_t which)
{
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/SHOWMODES"));
	config->DeleteEntry(kShowModeStrings[which]);
}

void
CalChartConfiguration::ClearConfigurationSpringShowMode(size_t which)
{
	wxConfigBase *config = wxConfigBase::Get();
	config->SetPath(wxT("/SPRINGSHOWMODES"));
	config->DeleteEntry(kSpringShowModeStrings[which]);
}

///// Color Configuration /////
// Color is more complicated, we use functions for setting that
std::array<long, 3> sColors[COLOR_NUM];
long sWidths[COLOR_NUM];
std::bitset<COLOR_NUM> s_color_valid;
std::bitset<COLOR_NUM> s_width_valid;

std::array<long, 3> ReadConfigColor(CalChartColors i)
{
	if (s_color_valid.test(i))
	{
		return sColors[i];
	}
	wxConfigBase *config = wxConfigBase::Get();
	
	// read out the color configuration:
	config->SetPath(wxT("/COLORS"));
	wxString rbuf = std::get<0>(ColorInfo[i]) + wxT("_Red");
	wxString gbuf = std::get<0>(ColorInfo[i]) + wxT("_Green");
	wxString bbuf = std::get<0>(ColorInfo[i]) + wxT("_Blue");
	if (config->Exists(rbuf) && config->Exists(gbuf) && config->Exists(bbuf))
	{
		long r = config->Read(rbuf, 0l);
		long g = config->Read(gbuf, 0l);
		long b = config->Read(bbuf, 0l);
		sColors[i] = { {r, g, b} };
	}
	else
	{
		auto color = wxColour(std::get<1>(ColorInfo[i]));
		sColors[i] = { {color.Red(), color.Green(), color.Blue()} };
	}
	s_color_valid.set(i);
	return sColors[i];
}

long ReadConfigColorWidth(CalChartColors i)
{
	if (s_width_valid.test(i))
	{
		return sWidths[i];
	}
	wxConfigBase *config = wxConfigBase::Get();
	
	// read out the color configuration:
	config->SetPath(wxT("/COLORS"));
	// store widths in a subgroup
	config->SetPath(wxT("WIDTH"));
	sWidths[i] = std::get<2>(ColorInfo[i]);
	config->Read(std::get<0>(ColorInfo[i]), &sWidths[i]);
	s_width_valid.set(i);
	return sWidths[i];
}

void
CalChartConfiguration::ClearConfigColor(size_t selection)
{
	wxConfigBase *config = wxConfigBase::Get();
	
	config->SetPath(wxT("/COLORS"));
	wxString rbuf = std::get<0>(ColorInfo[selection]) + wxT("_Red");
	config->DeleteEntry(rbuf);
	wxString gbuf = std::get<0>(ColorInfo[selection]) + wxT("_Green");
	config->DeleteEntry(gbuf);
	wxString bbuf = std::get<0>(ColorInfo[selection]) + wxT("_Blue");
	config->DeleteEntry(bbuf);
	config->SetPath(wxT("WIDTH"));
	config->DeleteEntry(std::get<0>(ColorInfo[selection]));
	config->Flush();
	s_color_valid.reset(selection);
	s_width_valid.reset(selection);
}

void ReadConfigYardlines()
{
	if (yard_text_valid)
	{
		return;
	}
	for (size_t i = 0; i < MAX_YARD_LINES; ++i)
	{
		wxString key;
		key.Printf(wxT("YardLines_%ld"), i);
		yard_text[i] = GetConfigValue<wxString>(key, yard_text_index[i]);
	}
	yard_text_valid = true;
}

void ReadConfigSpringYardlines()
{
	if (spr_line_yard_text_valid)
	{
		return;
	}
	for (size_t i = 0; i < MAX_SPR_LINES; ++i)
	{
		wxString key;
		key.Printf(wxT("SpringShowYardLines_%ld"), i);
		spr_line_text[i] = GetConfigValue<wxString>(key, spr_line_text_index[i]);
	}
	spr_line_yard_text_valid = true;
}

wxString
CalChartConfiguration::Get_yard_text(size_t which) const
{
	if (which >= MAX_YARD_LINES)
		throw std::runtime_error("Error, exceeding yard_text size");
	ReadConfigYardlines();
	return yard_text[which];
}

void
CalChartConfiguration::Set_yard_text(size_t which, const wxString& name)
{
	if (which >= MAX_YARD_LINES)
		throw std::runtime_error("Error, exceeding yard_text size");
	yard_text[which] = name;
	wxString key;
	key.Printf(wxT("YardLines_%ld"), which);
	SetConfigValue<wxString>(key, yard_text[which], yard_text_index[which]);
}

wxString
CalChartConfiguration::Get_spr_line_text(size_t which) const
{
	if (which >= MAX_SPR_LINES)
		throw std::runtime_error("Error, exceeding yard_text size");
	ReadConfigSpringYardlines();
	return spr_line_text[which];
}

void
CalChartConfiguration::Set_spr_line_text(size_t which, const wxString& name)
{
	if (which >= MAX_SPR_LINES)
		throw std::runtime_error("Error, exceeding yard_text size");
	spr_line_text[which] = name;
	wxString key;
	key.Printf(wxT("SpringShowYardLines_%ld"), which);
	SetConfigValue<wxString>(key, spr_line_text[which], spr_line_text_index[which]);
}

std::vector<wxString>
CalChartConfiguration::Get_yard_text_index() const
{
	return { std::begin(yard_text_index), std::end(yard_text_index) };
}

std::vector<wxString>
CalChartConfiguration::Get_spr_line_text_index() const
{
	return { std::begin(spr_line_text_index), std::end(spr_line_text_index) };
}

std::vector<wxString>
CalChartConfiguration::GetColorNames() const
{
	std::vector<wxString> result;
	std::transform(std::begin(ColorInfo), std::end(ColorInfo), std::back_inserter(result), [](const std::tuple<wxString, wxString, int>& i) { return std::get<0>(i); });
	return result;
}

std::vector<wxString>
CalChartConfiguration::GetDefaultColors() const
{
	std::vector<wxString> result;
	std::transform(std::begin(ColorInfo), std::end(ColorInfo), std::back_inserter(result), [](const std::tuple<wxString, wxString, int>& i) { return std::get<1>(i); });
	return result;
}

std::vector<int>
CalChartConfiguration::GetDefaultPenWidth() const
{
	std::vector<int> result;
	std::transform(std::begin(ColorInfo), std::end(ColorInfo), std::back_inserter(result), [](const std::tuple<wxString, wxString, int>& i) { return std::get<2>(i); });
	return result;
}


void
CalChartConfiguration::ClearConfigShowYardline()
{
	for (size_t i = 0; i < MAX_YARD_LINES; ++i)
	{
		wxString key;
		key.Printf(wxT("YardLines_%ld"), i);
		ClearConfigValue<wxString>(key);
	}
	yard_text_valid = false;
}

void
CalChartConfiguration::ClearConfigSpringShowYardline()
{
	for (size_t i = 0; i < MAX_SPR_LINES; ++i)
	{
		wxString key;
		key.Printf(wxT("SpringShowYardLines_%ld"), i);
		ClearConfigValue<wxString>(key);
	}
	spr_line_yard_text_valid = false;
}

wxBrush
CalChartConfiguration::GetCalChartBrush(CalChartColors c) const
{
	auto color_values = ReadConfigColor(c);
	auto color = wxColour(color_values[0], color_values[1], color_values[2]);
	return *wxTheBrushList->FindOrCreateBrush(color, wxSOLID);
}

wxPen
CalChartConfiguration::GetCalChartPen(CalChartColors c) const
{
	auto color_values = ReadConfigColor(c);
	auto color = wxColour(color_values[0], color_values[1], color_values[2]);
	auto width = ReadConfigColorWidth(c);
	return *wxThePenList->FindOrCreatePen(color, width, wxSOLID);
}

void
CalChartConfiguration::SetCalChartBrushAndPen(CalChartColors c, const wxBrush& brush, const wxPen& pen)
{
	if (c >= COLOR_NUM)
		throw std::runtime_error("Error, exceeding COLOR_NUM size");

	wxConfigBase *config = wxConfigBase::Get();

	// read out the color configuration:
	config->SetPath(wxT("/COLORS"));
	wxString rbuf = std::get<0>(ColorInfo[c]) + wxT("_Red");
	config->Write(rbuf, static_cast<long>(brush.GetColour().Red()));
	wxString gbuf = std::get<0>(ColorInfo[c]) + wxT("_Green");
	config->Write(gbuf, static_cast<long>(brush.GetColour().Green()));
	wxString bbuf = std::get<0>(ColorInfo[c]) + wxT("_Blue");
	config->Write(bbuf, static_cast<long>(brush.GetColour().Blue()));
	config->SetPath(wxT("WIDTH"));
	config->Write(std::get<0>(ColorInfo[c]), pen.GetWidth());
	config->Flush();
	s_color_valid.reset(c);
	s_width_valid.reset(c);
}
