/*
 * CalChartConfiguration.cpp
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

#include <array>
#include <string>
#include <wx/confbase.h>
#include <wx/config.h>

#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartShowMode.h"
#include "cc_omniview_constants.h"

using namespace CalChart;

// Yard lines
const std::array<std::string, kYardTextValues> yard_text_defaults = []() {
    std::array<std::string, kYardTextValues> values;
    auto default_yards = ShowMode::GetDefaultYardLines();
    for (auto i = 0; i < kYardTextValues; ++i) {
        values[i] = default_yards[i];
    }
    return values;
}();

const auto yard_text_index = yard_text_defaults;

const std::string kPaletteColorDefault[kNumberPalettes] = {
    "FOREST GREEN",
    "GREY",
    "GREY",
    "GREY",
};

const std::string kPaletteNameDefault[kNumberPalettes] = {
    "Default",
    "[Unset]",
    "[Unset]",
    "[Unset]",
};

static CalChartConfiguration& GetConfig()
{
    static CalChartConfiguration sconfig;
    return sconfig;
}

CalChartConfiguration& CalChartConfiguration::GetGlobalConfig()
{
    return GetConfig();
}

void CalChartConfiguration::AssignConfig(const CalChartConfiguration& config)
{
    // now flush out the config
    GetConfig() = config;
    GetConfig().FlushWriteQueue();
}

// Get, Clear and Set PathKey are the primatives; do not use these directly.
template <typename T>
T GetConfigPathKey(const std::string& path, const std::string& key, const T& def)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(path);
    T value = def;
    config->Read(key, &value);
    return value;
}

// clear out the config if it matches
template <typename T>
void ClearConfigPathKey(const std::string& path, const std::string& key)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(path);
    config->DeleteEntry(key);
    config->Flush();
}

// default value need to check if we need to set a value
template <typename T>
void SetConfigPathKey(const std::string& path, const std::string& key, const T& value)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(path);
    config->Write(key, value);
    config->Flush();
}

// functions for dealing with the wx config directly
template <typename T>
T GetConfigValue(const std::string& key, const T& def)
{
    return GetConfigPathKey<T>("/CalChart", key, def);
}

// clear out the config if it matches
// template <typename T>
// void ClearConfigValue(const std::string& key)
//{
//	wxConfigBase *config = wxConfigBase::Get();
//	config->SetPath("/CalChart");
//	config->DeleteEntry(key);
//	config->Flush();
//}

// default value need to check if we need to set a value
template <typename T>
void SetConfigValue(const std::string& key, const T& value, const T& def)
{
    // don't write if we don't have to
    if (GetConfigValue<T>(key, def) == value)
        return;
    // clear out the value if it's the same as the default
    if (def == value) {
        ClearConfigPathKey<T>("/CalChart", key);
        return;
    }
    SetConfigPathKey("/CalChart", key, value);
}

static CalChartConfiguration::ColorWidth_t
GetColorConfigValueForPalette(int palette, const std::string& key, const CalChartConfiguration::ColorWidth_t& def)
{
    auto r = std::get<0>(def).red;
    auto g = std::get<0>(def).green;
    auto b = std::get<0>(def).blue;
    auto rkey = key + "_Red";
    auto gkey = key + "_Green";
    auto bkey = key + "_Blue";
    auto palettekey = std::string("/PALETTE") + std::to_string(palette) + "/COLORS";
    r = GetConfigPathKey<long>(palettekey, rkey, r);
    g = GetConfigPathKey<long>(palettekey, gkey, g);
    b = GetConfigPathKey<long>(palettekey, bkey, b);

    auto w = std::get<1>(def);
    w = GetConfigPathKey<long>(palettekey + "/WIDTH", key, w);

    return CalChartConfiguration::ColorWidth_t(CalChart::Color(r, g, b), w);
}

// Specialize on Color
template <>
CalChartConfiguration::ColorWidth_t
GetConfigValue(std::string const& key, CalChartConfiguration::ColorWidth_t const& def)
{
    return GetColorConfigValueForPalette(GetConfigValue("ActiveColorPalette", 0), key, def);
}

static void SetColorConfigValueForPalette(int palette, std::string const& key, CalChartConfiguration::ColorWidth_t const& value, CalChartConfiguration::ColorWidth_t const& def)
{
    // don't write if we don't have to
    if (GetColorConfigValueForPalette(palette, key, def) == value)
        return;
    auto rkey = key + "_Red";
    auto gkey = key + "_Green";
    auto bkey = key + "_Blue";

    // TODO: fix this so it clears
    // clear out the value if it's the same as the default
    auto palettekey = std::string("/PALETTE") + std::to_string(palette) + "/COLORS";
    if (def == value) {
        ClearConfigPathKey<long>(palettekey, rkey);
        ClearConfigPathKey<long>(palettekey, gkey);
        ClearConfigPathKey<long>(palettekey, bkey);
        ClearConfigPathKey<long>(palettekey + "/WIDTH", key);
        return;
    }

    auto r = std::get<0>(value).red;
    auto g = std::get<0>(value).green;
    auto b = std::get<0>(value).blue;
    auto w = std::get<1>(value);
    SetConfigPathKey<long>(palettekey, rkey, r);
    SetConfigPathKey<long>(palettekey, gkey, g);
    SetConfigPathKey<long>(palettekey, bkey, b);
    SetConfigPathKey<long>(palettekey + "/WIDTH", key, w);
}

// Specialize on Color
template <>
void SetConfigValue(const std::string& key,
    const CalChartConfiguration::ColorWidth_t& value,
    const CalChartConfiguration::ColorWidth_t& def)
{
    SetColorConfigValueForPalette(GetConfigValue("ActiveColorPalette", 0), key, value, def);
}

template <>
CalChart::ShowModeData_t
GetConfigValue(const std::string& key,
    const CalChart::ShowModeData_t& def)
{
    auto values = def;
    std::string path = "/SHOWMODES/" + key;
    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        values[i] = GetConfigPathKey<long>(path, ShowModeKeys[i], values[i]);
    }
    return values;
}

// Specialize on show mode
template <>
void SetConfigValue(const std::string& key,
    const ShowModeData_t& value,
    const ShowModeData_t& def)
{
    // don't write if we don't have to
    if (GetConfigValue<ShowModeData_t>(key, def) == value) {
        return;
    }
    std::string path = "/SHOWMODES/" + key;

    // TODO: fix this so it clears
    // clear out the value if it's the same as the default
    if (def == value) {
        for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
            ClearConfigPathKey<long>(path, ShowModeKeys[i]);
        }
        return;
    }

    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        SetConfigPathKey<long>(path, ShowModeKeys[i], value[i]);
    }
}

// Specialize on std::string
template <>
std::string
GetConfigValue(std::string const& key, std::string const& def)
{
    return GetConfigPathKey<wxString>("/CalChart", key, def);
}

template <>
void SetConfigValue(const std::string& key, std::string const& value, std::string const& def)
{
    // don't write if we don't have to
    if (GetConfigValue<wxString>(key, def) == value) {
        return;
    }
    SetConfigPathKey<wxString>("/CalChart", key, value);
}

#define IMPLEMENT_CONFIGURATION_FUNCTIONS(KeyName, Type, TheValue)                                             \
    static const std::string k##KeyName##Key = #KeyName;                                                       \
    static const Type k##KeyName##Value = (TheValue);                                                          \
    Type CalChartConfiguration::Get_##KeyName() const                                                          \
    {                                                                                                          \
        if (!m##KeyName.first) {                                                                               \
            m##KeyName.second = GetConfigValue<Type>(k##KeyName##Key, k##KeyName##Value);                      \
            m##KeyName.first = true;                                                                           \
        }                                                                                                      \
        return m##KeyName.second;                                                                              \
    }                                                                                                          \
    void CalChartConfiguration::Set_##KeyName(const Type& v)                                                   \
    {                                                                                                          \
        mWriteQueue[k##KeyName##Key] = [v]() { SetConfigValue<Type>(k##KeyName##Key, v, k##KeyName##Value); }; \
        m##KeyName.first = true;                                                                               \
        m##KeyName.second = v;                                                                                 \
    }                                                                                                          \
    void CalChartConfiguration::Clear_##KeyName()                                                              \
    {                                                                                                          \
        return Set_##KeyName(k##KeyName##Value);                                                               \
    }

IMPLEMENT_CONFIGURATION_FUNCTIONS(AutosaveInterval, long, 60);

IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldFrameZoom_3_6_0, double, 1.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldCanvasScrollX, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldCanvasScrollY, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldFrameWidth, long, 1200);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldFrameHeight, long, 750);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldFramePositionX, long, 50);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FieldFramePositionY, long, 50);

// printing
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintFile, std::string, "LPT1");
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintCmd, std::string, "lpr");
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintOpts, std::string, "");
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintViewCmd, std::string, "ghostview");
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintViewOpts, std::string, "");

IMPLEMENT_CONFIGURATION_FUNCTIONS(PageWidth, double, 7.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PageHeight, double, 10.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PageOffsetX, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PageOffsetY, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PaperLength, double, 11.0);

IMPLEMENT_CONFIGURATION_FUNCTIONS(HeadFont, std::string, "Palatino-Bold");
IMPLEMENT_CONFIGURATION_FUNCTIONS(MainFont, std::string, "Helvetica");
IMPLEMENT_CONFIGURATION_FUNCTIONS(NumberFont, std::string, "Helvetica-Bold");
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContFont, std::string, "Courier");
IMPLEMENT_CONFIGURATION_FUNCTIONS(BoldFont, std::string, "Courier-Bold");
IMPLEMENT_CONFIGURATION_FUNCTIONS(ItalFont, std::string, "Courier-Italic");
IMPLEMENT_CONFIGURATION_FUNCTIONS(BoldItalFont, std::string,
    "Courier-BoldItalic");

IMPLEMENT_CONFIGURATION_FUNCTIONS(HeaderSize, double, 3.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(YardsSize, double, 1.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(TextSize, double, 10.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(DotRatio, double, 0.9);
IMPLEMENT_CONFIGURATION_FUNCTIONS(NumRatio, double, 1.35);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(SLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContRatio, double, 0.2);

IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSModes, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSLandscape, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSOverview, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSDoCont, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSDoContSheet, bool, false);

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_4, float, kViewPoint_x_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_4, float, kViewPoint_y_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_4, float, kViewPoint_z_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_4, float, kViewAngle_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_4, float, kViewAngle_z_1);

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_5, float, kViewPoint_x_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_5, float, kViewPoint_y_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_5, float, kViewPoint_z_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_5, float, kViewAngle_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_5, float, kViewAngle_z_2);

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_6, float, kViewPoint_x_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_6, float, kViewPoint_y_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_6, float, kViewPoint_z_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_6, float, kViewAngle_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_6, float, kViewAngle_z_3);

IMPLEMENT_CONFIGURATION_FUNCTIONS(AnimationFrameWidth, long, 600);
IMPLEMENT_CONFIGURATION_FUNCTIONS(AnimationFrameHeight, long, 450);

IMPLEMENT_CONFIGURATION_FUNCTIONS(AnimationFrameSashPosition, long, 100);
IMPLEMENT_CONFIGURATION_FUNCTIONS(AnimationFrameOmniAnimation, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(AnimationFrameSplitScreen, bool, true);
IMPLEMENT_CONFIGURATION_FUNCTIONS(AnimationFrameSplitVertical, bool, false);

IMPLEMENT_CONFIGURATION_FUNCTIONS(UseSprites, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(SpriteBitmapScale, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(SpriteBitmapOffsetY, double, 0.5);

IMPLEMENT_CONFIGURATION_FUNCTIONS(ScrollDirectionNatural, bool, true);

IMPLEMENT_CONFIGURATION_FUNCTIONS(CommandUndoSetSheet, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(CommandUndoSelection, bool, false);

IMPLEMENT_CONFIGURATION_FUNCTIONS(BeepOnCollisions, bool, true);

// if the layout changes, this is what you increment in order to force a refresh of the layout
IMPLEMENT_CONFIGURATION_FUNCTIONS(CalChartFrameAUILayout_3_6_1, std::string, "");

IMPLEMENT_CONFIGURATION_FUNCTIONS(ContCellLongForm, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContCellFontSize, long, 14);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContCellRounding, long, 4);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContCellTextPadding, long, 4);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContCellBoxPadding, long, 4);

// OBSOLETE Settigns
// "MainFrameZoom" now obsolete with version post 3.2, use "MainFrameZoom2"
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameZoom, float, 0.5);
// "MainFrameZoom", "MainFrameWidth", "MainFrameHeight" now obsolete with
// version post 3.3.1, use Field versions
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameZoom2, float, 0.5);
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameWidth, long, 600);
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameHeight, long, 450);

template <int Which, typename T, std::size_t N, typename Indices = std::make_index_sequence<N>>
auto GetVectorFromTupleArray(std::array<T, N> const& a)
{
    auto data = details::at_offset<Which>(a, Indices{});
    return std::vector(data.begin(), data.end());
}

std::vector<std::string> CalChartConfiguration::GetColorNames() const
{
    return GetVectorFromTupleArray<0>(ColorInfoDefaults);
}

std::vector<std::string> CalChartConfiguration::GetDefaultColors() const
{
    return GetVectorFromTupleArray<1>(ColorInfoDefaults);
}

std::vector<int> CalChartConfiguration::GetDefaultPenWidth() const
{
    return GetVectorFromTupleArray<2>(ColorInfoDefaults);
}

std::vector<std::string> CalChartConfiguration::GetContCellColorNames() const
{
    return GetVectorFromTupleArray<0>(ContCellColorInfoDefaults);
}

std::vector<std::string> CalChartConfiguration::GetContCellDefaultColors() const
{
    return GetVectorFromTupleArray<1>(ContCellColorInfoDefaults);
}

std::vector<int> CalChartConfiguration::GetContCellDefaultPenWidth() const
{
    return GetVectorFromTupleArray<2>(ContCellColorInfoDefaults);
}

std::vector<std::string> CalChartConfiguration::Get_yard_text_index() const
{
    return { std::begin(yard_text_index), std::end(yard_text_index) };
}

///// Color Configuration /////

long CalChartConfiguration::GetActiveColorPalette() const
{
    if (mActiveColorPalette == -1) {
        mActiveColorPalette = GetConfigValue("ActiveColorPalette", 0);
    }
    return mActiveColorPalette;
}

void CalChartConfiguration::SetActiveColorPalette(long which)
{
    mActiveColorPalette = which;
    SetConfigValue<long>("ActiveColorPalette", which, 0);
}

void CalChartConfiguration::ClearActiveColorPalette()
{
    SetActiveColorPalette(0);
}

CalChart::Color CalChartConfiguration::GetColorPaletteColor(long which) const
{
    auto colorWidth = GetColorConfigValueForPalette(which, std::string("PaletteColor"), { wxCalChart::toColor(wxColour{ kPaletteColorDefault[which] }), 1 });
    return std::get<0>(colorWidth);
}

void CalChartConfiguration::SetColorPaletteColor(long which, CalChart::Color color)
{
    CalChartConfiguration::ColorWidth_t v{ color, 1 };
    mWriteQueue[std::string("PaletteColor") + std::to_string(which)] = [which, v]() {
        SetColorConfigValueForPalette(which, std::string("PaletteColor"), v, { wxCalChart::toColor(wxColour{ kPaletteColorDefault[which] }), 1 });
    };
}

void CalChartConfiguration::ClearColorPaletteColor(long which)
{
    CalChartConfiguration::ColorWidth_t v{ wxCalChart::toColor(wxColour{ kPaletteColorDefault[which] }), 1 };
    mWriteQueue[std::string("PaletteColor") + std::to_string(which)] = [which, v]() {
        SetColorConfigValueForPalette(which, std::string("PaletteColor"), v, v);
    };
}

std::vector<CalChart::Color> CalChartConfiguration::GetDefaultColorPaletteColors() const
{
    auto results = std::vector<CalChart::Color>{};
    std::transform(std::begin(kPaletteColorDefault), std::end(kPaletteColorDefault), std::back_inserter(results), [](auto&& i) {
        return wxCalChart::toColor(wxColour{ i });
    });

    return results;
}

std::string CalChartConfiguration::GetColorPaletteName(long which) const
{
    if (which > kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    return GetConfigValue<std::string>(std::string("PaletteName") + std::to_string(which), kPaletteNameDefault[which]);
}

void CalChartConfiguration::SetColorPaletteName(long which, std::string const& name)
{
    mWriteQueue[std::string("PaletteName") + std::to_string(which)] = [which, name]() {
        SetConfigValue<std::string>(std::string("PaletteName") + std::to_string(which), name, kPaletteNameDefault[which]);
    };
}

void CalChartConfiguration::ClearColorPaletteName(long which)
{
    mWriteQueue[std::string("PaletteName") + std::to_string(which)] = [which]() {
        SetConfigValue<std::string>(std::string("PaletteName") + std::to_string(which), kPaletteNameDefault[which], kPaletteNameDefault[which]);
    };
}

std::vector<std::string> CalChartConfiguration::GetDefaultColorPaletteNames() const
{
    return { std::begin(kPaletteNameDefault), std::end(kPaletteNameDefault) };
}

template <typename Color, typename Map, typename InfoDefault>
CalChart::BrushAndPen
Get_BrushAndPen(Color c, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray)
{
    if (!colorsAndWidth.count(c)) {
        colorsAndWidth[c] = GetConfigValue<CalChartConfiguration::ColorWidth_t>(
            std::string(std::get<0>(InfoDefaultArray[toUType(c)])),
            CalChartConfiguration::ColorWidth_t(wxCalChart::toColor(std::get<1>(InfoDefaultArray[toUType(c)])), std::get<2>(InfoDefaultArray[toUType(c)])));
    }
    auto colorAndWidth = colorsAndWidth[c];
    return { std::get<0>(colorAndWidth), CalChart::Brush::Style::Solid, std::get<1>(colorAndWidth) };
}

template <typename Color, typename Map, typename InfoDefault, typename WriteQueue>
void Set_BrushAndPen(Color c, CalChart::BrushAndPen brushAndPen, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray, WriteQueue& writeQueue)
{
    CalChartConfiguration::ColorWidth_t v{ brushAndPen.color, brushAndPen.width };

    CalChartConfiguration::ColorWidth_t defaultValue{ wxCalChart::toColor(std::get<1>(InfoDefaultArray[toUType(c)])), std::get<2>(InfoDefaultArray[toUType(c)]) };
    writeQueue[std::get<0>(InfoDefaultArray[toUType(c)])] = [info = std::get<0>(InfoDefaultArray[toUType(c)]), defaultValue, v]() {
        SetConfigValue<CalChartConfiguration::ColorWidth_t>(info, v, defaultValue);
    };
    colorsAndWidth[c] = v;
}

template <typename Color, typename Map, typename InfoDefault>
void Clear_ConfigColor(Color selection, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray)
{
    auto default_value = CalChartConfiguration::ColorWidth_t(wxCalChart::toColor(std::get<1>(InfoDefaultArray[toUType(selection)])), std::get<2>(InfoDefaultArray[toUType(selection)]));
    SetConfigValue<CalChartConfiguration::ColorWidth_t>(std::get<0>(InfoDefaultArray[toUType(selection)]), default_value, default_value);
    // clear out the cached value
    colorsAndWidth.erase(selection);
}

CalChart::BrushAndPen
CalChartConfiguration::Get_CalChartBrushAndPen(CalChart::Colors c) const
{
    return Get_CalChartBrushAndPen(GetActiveColorPalette(), c);
}

void CalChartConfiguration::Set_CalChartBrushAndPen(CalChart::Colors c, CalChart::BrushAndPen brushAndPen)
{
    return Set_CalChartBrushAndPen(GetActiveColorPalette(), c, brushAndPen);
}

void CalChartConfiguration::Clear_CalChartConfigColor(CalChart::Colors selection)
{
    return Clear_CalChartConfigColor(GetActiveColorPalette(), selection);
}

CalChart::BrushAndPen
CalChartConfiguration::Get_CalChartBrushAndPen(int palette, CalChart::Colors c) const
{
    if (c >= CalChart::Colors::NUM)
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    if (palette >= kNumberPalettes)
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    return Get_BrushAndPen(c, mColorsAndWidth[palette], CalChart::ColorInfoDefaults);
}

void CalChartConfiguration::Set_CalChartBrushAndPen(int palette, CalChart::Colors c, CalChart::BrushAndPen brushAndPen)
{
    if (c >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }

    Set_BrushAndPen(c, brushAndPen, mColorsAndWidth[palette], CalChart::ColorInfoDefaults, mWriteQueue);
}

void CalChartConfiguration::Clear_CalChartConfigColor(int palette, CalChart::Colors selection)
{
    if (selection >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }

    Clear_ConfigColor(selection, mColorsAndWidth[palette], CalChart::ColorInfoDefaults);
}

CalChart::BrushAndPen
CalChartConfiguration::Get_ContCellBrushAndPen(CalChart::ContinuityCellColors c) const
{
    if (c >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    return Get_BrushAndPen(c, mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults);
}

void CalChartConfiguration::Set_ContCellBrushAndPen(CalChart::ContinuityCellColors c, CalChart::BrushAndPen brushAndPen)
{
    if (c >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    Set_BrushAndPen(c, brushAndPen, mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults, mWriteQueue);
}

void CalChartConfiguration::Clear_ContCellConfigColor(CalChart::ContinuityCellColors selection)
{
    if (selection >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    Clear_ConfigColor(selection, mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults);
}

///// Show Configuration /////

CalChart::ShowModeData_t
CalChartConfiguration::Get_ShowModeData(CalChart::ShowModes which) const
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }

    if (!mShowModeInfos.count(which)) {
        auto& defaultValue = kShowModeDefaultValues[twhich];
        mShowModeInfos[which] = GetConfigValue<ShowModeData_t>(
            std::string(std::get<0>(defaultValue)), CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
    }
    return mShowModeInfos[which];
}

void CalChartConfiguration::Set_ShowModeData(CalChart::ShowModes which, ShowModeData_t const& values)
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }

    auto& defaultValue = kShowModeDefaultValues[twhich];
    mWriteQueue[std::string(std::get<0>(defaultValue))] = [defaultValue, which, values]() {
        SetConfigValue<ShowModeData_t>(std::string(std::get<0>(defaultValue)), values,
            CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
    };
    mShowModeInfos[which] = values;
}

void CalChartConfiguration::Clear_ShowModeData(CalChart::ShowModes which)
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }

    auto& defaultValue = kShowModeDefaultValues[twhich];
    SetConfigValue<ShowModeData_t>(std::string(std::get<0>(defaultValue)), CalChart::ShowModeData_t{ std::get<1>(defaultValue) },
        CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
    mShowModeInfos.erase(which);
}

// Yard Lines
std::string CalChartConfiguration::Get_yard_text(size_t which) const
{
    if (which >= kYardTextValues)
        throw std::runtime_error("Error, exceeding kYardTextValues size");

    if (!mYardTextInfos.count(which)) {
        auto key = std::string{ "YardLines_" } + std::to_string(which);
        auto default_value = yard_text_defaults[which];
        mYardTextInfos[which] = GetConfigValue(key, yard_text_defaults[which]);
    }
    return mYardTextInfos[which];
}

void CalChartConfiguration::Set_yard_text(size_t which, const std::string& value)
{
    if (which >= kYardTextValues)
        throw std::runtime_error("Error, exceeding kYardTextValues size");

    auto key = std::string{ "YardLines_" } + std::to_string(which);
    auto default_value = yard_text_defaults[which];
    mWriteQueue[key] = [key, value, default_value]() {
        SetConfigValue(key, value, default_value);
    };
    mYardTextInfos[which] = value;
}

void CalChartConfiguration::Clear_yard_text(size_t which)
{
    if (which >= kYardTextValues)
        throw std::runtime_error("Error, exceeding kYardTextValues size");

    auto key = std::string{ "YardLines_" } + std::to_string(which);
    auto default_value = yard_text_defaults[which];
    SetConfigValue(key, default_value, default_value);
    mYardTextInfos.erase(which);
}

// function technically const because it is changing a mutable value
void CalChartConfiguration::FlushWriteQueue() const
{
    for (auto& i : mWriteQueue) {
        i.second();
    }
    mWriteQueue.clear();
}

ShowMode GetConfigShowMode(const std::string& which)
{
    auto iter = std::find_if(std::begin(kShowModeDefaultValues), std::end(kShowModeDefaultValues), [which](auto&& t) {
        return std::get<0>(t) == which;
    });
    if (iter != std::end(kShowModeDefaultValues)) {
        auto item = static_cast<CalChart::ShowModes>(std::distance(std::begin(kShowModeDefaultValues), iter));
        return ShowMode::CreateShowMode(CalChartConfiguration::GetGlobalConfig().Get_ShowModeData(item), Get_yard_text_all(CalChartConfiguration::GetGlobalConfig()));
    }
    throw std::runtime_error("No such show");
}

std::vector<CalChart::Color> GetColorPaletteColors(CalChartConfiguration const& config)
{
    auto result = std::vector<CalChart::Color>{};
    for (auto i = 0; i < kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteColor(i));
    }
    return result;
}

std::vector<std::string> GetColorPaletteNames(CalChartConfiguration const& config)
{
    auto result = std::vector<std::string>{};
    for (auto i = 0; i < kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteName(i));
    }
    return result;
}

std::array<std::string, kYardTextValues> Get_yard_text_all(CalChartConfiguration const& config)
{
    std::array<std::string, kYardTextValues> values;
    for (auto i = 0; i < kYardTextValues; ++i) {
        values[i] = config.Get_yard_text(i);
    }
    return values;
}
