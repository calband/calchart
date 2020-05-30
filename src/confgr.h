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

#pragma once

#include <array>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <wx/gdicmn.h>
#include <wx/string.h>

// forward declare
class wxPen;
class wxBrush;
class wxPathList;
class wxColour;
namespace CalChart {
class ShowMode;
}

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

enum ContCellColors {
    COLOR_CONTCELLS_PROC,
    COLOR_CONTCELLS_VALUE,
    COLOR_CONTCELLS_FUNCTION,
    COLOR_CONTCELLS_DIRECTION,
    COLOR_CONTCELLS_STEPTYPE,
    COLOR_CONTCELLS_POINT,
    COLOR_CONTCELLS_UNSET,
    COLOR_CONTCELLS_NUM,
};

enum CalChartShowModes {
    STANDARD,
    FULL_FIELD,
    TUNNEL,
    OLD_FIELD,
    PRO_FIELD,
    SHOWMODE_NUM
};

extern wxString const kShowModeStrings[SHOWMODE_NUM];

constexpr auto kNumberPalettes = 4; // arbitrary, could go more, but 4 is a good starting point

// CalChartConfiguration interfaces with the system config and acts as a "cache"
// for the values.
// On Get, it reads the values from system config, and caches a copy.
// On Set (and clear), it updates it's cache, and puts the command into a
// write-queue.
// The write-queue needs to be explicitly flushed or the values will be lost
//
// To use a config value, first get the Global config, and then Get_ the value
// from it:
// auto save_interval =
// CalChartConfiguration::GetGlobalConfig().Get_AutosaveInterval();
//
// To add a new config value:
// Add DECLARE_CONFIGURATION_FUNCTIONS in the class declaration of the right
// type; this will make the Get_, Set_ and Clear_
// functions available.  Then in the implementation file, declare
// IMPLEMENT_CONFIGURATION_FUNCTIONS with the default.
class CalChartConfiguration {
public:
    static CalChartConfiguration& GetGlobalConfig();
    static void AssignConfig(CalChartConfiguration const& config);

    // explicit flush
    void FlushWriteQueue() const;

private:
    mutable std::map<wxString, std::function<void()>> mWriteQueue;

// macro for declaring configuration Get_, Set_, and Clear_
#define DECLARE_CONFIGURATION_FUNCTIONS(Key, Type) \
public:                                            \
    Type Get_##Key() const;                        \
    void Set_##Key(Type const& v);                 \
    void Clear_##Key();                            \
                                                   \
private:                                           \
    mutable std::pair<bool, Type> m##Key = { false, Type() };

    DECLARE_CONFIGURATION_FUNCTIONS(AutosaveInterval, long);

    // page setup and zoom
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFrameZoom, double);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldCanvasScrollX, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldCanvasScrollY, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFrameWidth, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFrameHeight, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFramePositionX, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFramePositionY, long);

    // printing configurations
    DECLARE_CONFIGURATION_FUNCTIONS(PrintFile, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintCmd, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintOpts, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintViewCmd, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintViewOpts, wxString);

    DECLARE_CONFIGURATION_FUNCTIONS(PageWidth, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PageHeight, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PageOffsetX, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PageOffsetY, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PaperLength, double);

    DECLARE_CONFIGURATION_FUNCTIONS(HeadFont, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(MainFont, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(NumberFont, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(ContFont, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(BoldFont, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(ItalFont, wxString);
    DECLARE_CONFIGURATION_FUNCTIONS(BoldItalFont, wxString);

    DECLARE_CONFIGURATION_FUNCTIONS(HeaderSize, double);
    DECLARE_CONFIGURATION_FUNCTIONS(YardsSize, double);
    DECLARE_CONFIGURATION_FUNCTIONS(TextSize, double);
    DECLARE_CONFIGURATION_FUNCTIONS(DotRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(NumRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PLineRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(SLineRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(ContRatio, double);

    DECLARE_CONFIGURATION_FUNCTIONS(PrintPSModes, long);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintPSLandscape, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintPSOverview, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintPSDoCont, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintPSDoContSheet, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_4, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_4, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_4, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewAngle_4, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_4, float);

    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_5, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_5, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_5, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewAngle_5, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_5, float);

    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_6, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_6, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_6, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewAngle_6, float);
    DECLARE_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_6, float);

    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameWidth, long);
    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameHeight, long);

    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameSashPosition, long);
    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameOmniAnimation, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameSplitScreen, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameSplitVertical, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(ScrollDirectionNatural, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(CommandUndoSetSheet, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(CommandUndoSelection, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(BeepOnCollisions, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(CalChartFrameAUILayout, wxString);

    DECLARE_CONFIGURATION_FUNCTIONS(ContCellLongForm, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellFontSize, long);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellRounding, long);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellTextPadding, long);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellBoxPadding, long);

public:
    // helpers for displaying different config attributes
    std::vector<wxString> GetColorNames() const;
    std::vector<wxString> GetDefaultColors() const;
    std::vector<int> GetDefaultPenWidth() const;
    std::vector<wxString> GetContCellColorNames() const;
    std::vector<wxString> GetContCellDefaultColors() const;
    std::vector<int> GetContCellDefaultPenWidth() const;
    std::vector<wxString> Get_yard_text_index() const;

    // color palettes:  The color Palettes allow you to set different "blocks" of
    // colors.
    // When a Palette is set all the sets and gets are treated against that palette
    long GetActiveColorPalette() const;
    void SetActiveColorPalette(long);
    void ClearActiveColorPalette();

    wxBrush GetColorPaletteColor(long which) const;
    void SetColorPaletteColor(long which, wxBrush const&);
    void ClearColorPaletteColor(long which);

    wxString GetColorPaletteName(long which) const;
    void SetColorPaletteName(long which, wxString const&);
    void ClearColorPaletteName(long which);

    // Colors
    using ColorWidth_t = std::pair<wxColour, int>;
    std::pair<wxBrush, wxPen> Get_CalChartBrushAndPen(CalChartColors c) const;
    void Set_CalChartBrushAndPen(CalChartColors c, wxBrush const& brush, wxPen const& pen);
    void Clear_CalChartConfigColor(CalChartColors selection);

    std::pair<wxBrush, wxPen> Get_CalChartBrushAndPen(int palette, CalChartColors c) const;
    void Set_CalChartBrushAndPen(int palette, CalChartColors c, wxBrush const& brush, wxPen const& pen);
    void Clear_CalChartConfigColor(int palette, CalChartColors selection);

    std::pair<wxBrush, wxPen> Get_ContCellBrushAndPen(ContCellColors c) const;
    void Set_ContCellBrushAndPen(ContCellColors c, wxBrush const& brush, wxPen const& pen);
    void Clear_ContCellConfigColor(ContCellColors selection);

    // Shows
    static constexpr auto kShowModeValues = 10;
    using ShowModeInfo_t = std::array<long, kShowModeValues>;
    ShowModeInfo_t Get_ShowModeInfo(CalChartShowModes which) const;
    void Set_ShowModeInfo(CalChartShowModes which, ShowModeInfo_t const& values);
    void Clear_ShowModeInfo(CalChartShowModes which);

    // Yard Lines
    static constexpr auto kYardTextValues = 53;
    wxString Get_yard_text(size_t which) const;
    void Set_yard_text(size_t which, wxString const&);
    void Clear_yard_text(size_t which);

private:
    std::vector<wxBrush> GetDefaultColorPaletteColors() const;
    std::vector<wxString> GetDefaultColorPaletteNames() const;

    mutable int mActiveColorPalette = -1;
    mutable std::map<CalChartColors, ColorWidth_t> mColorsAndWidth[kNumberPalettes];
    mutable std::map<ContCellColors, ColorWidth_t> mContCellColorsAndWidth;
    mutable std::map<CalChartShowModes, ShowModeInfo_t> mShowModeInfos;
    mutable std::map<size_t, wxString> mYardTextInfos;
};

std::vector<wxBrush> GetColorPaletteColors(CalChartConfiguration const& config);
std::vector<wxString> GetColorPaletteNames(CalChartConfiguration const& config);
std::array<std::string, CalChartConfiguration::kYardTextValues> Get_yard_text_all(CalChartConfiguration const& config);

// to find a specific Show:
CalChart::ShowMode GetConfigShowMode(wxString const& which);
