#pragma once
/*
 * CalChartConfiguration.h
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

/**
 * CalChartConfiguration
 *
 * CalChartConfiguration interfaces with the system config and acts as a "cache" for the values.
 *
 * On Get, it reads the values from system config, and caches a local copy.
 * On Set (or clear), it updates it's cache, and puts the command into a write-queue.
 * The write-queue needs to be explicitly flushed or the values will be lost.
 *
 * To use a config value, first get the Global config, and then Get_ the value from it.  For example:
 *
 * auto save_interval = CalChartConfiguration::GetGlobalConfig().Get_AutosaveInterval();
 *
 * To add a new config value:
 *  Add DECLARE_CONFIGURATION_FUNCTIONS in the class declaration of the right type.  This
 *  will make the Get_, Set_ and Clear_ functions available.  Then in the implementation file, declare
 *  IMPLEMENT_CONFIGURATION_FUNCTIONS with the default.
 */

#include "CalChartConstants.h"
#include "CalChartDrawPrimatives.h"
#include <array>
#include <functional>
#include <map>
#include <memory>
#include <vector>
#include <wx/gdicmn.h>

// forward declare
class wxPathList;
namespace CalChart {
class ShowMode;
}

constexpr auto kNumberPalettes = 4; // arbitrary, could go more, but 4 is a good starting point

class CalChartConfiguration {
public:
    static CalChartConfiguration& GetGlobalConfig();
    static void AssignConfig(CalChartConfiguration const& config);

    // explicit flush
    void FlushWriteQueue() const;

private:
    mutable std::map<std::string, std::function<void()>> mWriteQueue;

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
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFrameZoom_3_6_0, double);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldCanvasScrollX, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldCanvasScrollY, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFrameWidth, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFrameHeight, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFramePositionX, long);
    DECLARE_CONFIGURATION_FUNCTIONS(FieldFramePositionY, long);

    // printing configurations
    DECLARE_CONFIGURATION_FUNCTIONS(PrintFile, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintCmd, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintOpts, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintViewCmd, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintViewOpts, std::string);

    DECLARE_CONFIGURATION_FUNCTIONS(PageWidth, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PageHeight, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PageOffsetX, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PageOffsetY, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PaperLength, double);

    DECLARE_CONFIGURATION_FUNCTIONS(HeadFont, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(MainFont, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(NumberFont, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(ContFont, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(BoldFont, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(ItalFont, std::string);
    DECLARE_CONFIGURATION_FUNCTIONS(BoldItalFont, std::string);

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

    DECLARE_CONFIGURATION_FUNCTIONS(UseSprites, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(SpriteBitmapScale, double);
    DECLARE_CONFIGURATION_FUNCTIONS(SpriteBitmapOffsetY, double);

    DECLARE_CONFIGURATION_FUNCTIONS(ScrollDirectionNatural, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(CommandUndoSetSheet, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(CommandUndoSelection, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(BeepOnCollisions, bool);

    DECLARE_CONFIGURATION_FUNCTIONS(CalChartFrameAUILayout_3_6_1, std::string);

    DECLARE_CONFIGURATION_FUNCTIONS(ContCellLongForm, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellFontSize, long);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellRounding, long);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellTextPadding, long);
    DECLARE_CONFIGURATION_FUNCTIONS(ContCellBoxPadding, long);

public:
    // helpers for displaying different config attributes
    std::vector<std::string> GetColorNames() const;
    std::vector<std::string> GetDefaultColors() const;
    std::vector<int> GetDefaultPenWidth() const;
    std::vector<std::string> GetContCellColorNames() const;
    std::vector<std::string> GetContCellDefaultColors() const;
    std::vector<int> GetContCellDefaultPenWidth() const;
    std::vector<std::string> Get_yard_text_index() const;

    // color palettes:  The color Palettes allow you to set different "blocks" of
    // colors.
    // When a Palette is set all the sets and gets are treated against that palette
    long GetActiveColorPalette() const;
    void SetActiveColorPalette(long);
    void ClearActiveColorPalette();

    CalChart::Color GetColorPaletteColor(long which) const;
    void SetColorPaletteColor(long which, CalChart::Color);
    void ClearColorPaletteColor(long which);

    std::string GetColorPaletteName(long which) const;
    void SetColorPaletteName(long which, std::string const&);
    void ClearColorPaletteName(long which);

    // Colors
    using ColorWidth_t = std::pair<CalChart::Color, int>;
    CalChart::BrushAndPen Get_CalChartBrushAndPen(CalChart::Colors c) const;
    void Set_CalChartBrushAndPen(CalChart::Colors c, CalChart::BrushAndPen);
    void Clear_CalChartConfigColor(CalChart::Colors selection);

    CalChart::BrushAndPen Get_CalChartBrushAndPen(int palette, CalChart::Colors c) const;
    void Set_CalChartBrushAndPen(int palette, CalChart::Colors c, CalChart::BrushAndPen);
    void Clear_CalChartConfigColor(int palette, CalChart::Colors selection);

    CalChart::BrushAndPen Get_ContCellBrushAndPen(CalChart::ContinuityCellColors c) const;
    void Set_ContCellBrushAndPen(CalChart::ContinuityCellColors c, CalChart::BrushAndPen);
    void Clear_ContCellConfigColor(CalChart::ContinuityCellColors selection);

    // Shows
    CalChart::ShowModeData_t Get_ShowModeData(CalChart::ShowModes which) const;
    void Set_ShowModeData(CalChart::ShowModes which, CalChart::ShowModeData_t const& values);
    void Clear_ShowModeData(CalChart::ShowModes which);

    // Yard Lines
    static constexpr auto kYardTextValues = 53;
    std::string Get_yard_text(size_t which) const;
    void Set_yard_text(size_t which, std::string const&);
    void Clear_yard_text(size_t which);

private:
    std::vector<CalChart::Color> GetDefaultColorPaletteColors() const;
    std::vector<std::string> GetDefaultColorPaletteNames() const;

    mutable int mActiveColorPalette = -1;
    mutable std::map<CalChart::Colors, ColorWidth_t> mColorsAndWidth[kNumberPalettes];
    mutable std::map<CalChart::ContinuityCellColors, ColorWidth_t> mContCellColorsAndWidth;
    mutable std::map<CalChart::ShowModes, CalChart::ShowModeData_t> mShowModeInfos;
    mutable std::map<size_t, std::string> mYardTextInfos;
};

std::vector<CalChart::Color> GetColorPaletteColors(CalChartConfiguration const& config);
std::vector<std::string> GetColorPaletteNames(CalChartConfiguration const& config);
std::array<std::string, CalChartConfiguration::kYardTextValues> Get_yard_text_all(CalChartConfiguration const& config);

// to find a specific Show:
CalChart::ShowMode GetConfigShowMode(std::string const& which);
