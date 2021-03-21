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

#include <array>
#include <ctype.h>
#include <string>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/confbase.h>
#include <wx/config.h>
#include <wx/file.h>
#include <wx/pen.h>
#include <wx/utils.h>

#include "CalChartApp.h"
#include "cc_omniview_constants.h"
#include "confgr.h"
#include "modes.h"

using namespace CalChart;

const wxString ColorInfo[COLOR_NUM] = {
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
    wxT("GHOST POINT"),
    wxT("GHOST POINT TEXT"),
    wxT("HLIT GHOST POINT"),
    wxT("HLIT GHOST POINT TEXT"),
    wxT("ANIM FRONT"),
    wxT("ANIM BACK"),
    wxT("ANIM SIDE"),
    wxT("HILIT ANIM FRONT"),
    wxT("HILIT ANIM BACK"),
    wxT("HILIT ANIM SIDE"),
    wxT("ANIM COLLISION"),
    wxT("ANIM COLLISION WARNING"),
    wxT("SHAPES"),
    wxT("CONTINUITY PATHS"),
};

const std::tuple<wxString, int> ColorInfoDefaults[COLOR_NUM] = {
    { wxT("FOREST GREEN"), 1 },
    { wxT("WHITE"), 1 },
    { wxT("BLACK"), 1 },
    { wxT("WHITE"), 1 },
    { wxT("BLACK"), 1 },
    { wxT("YELLOW"), 1 },
    { wxT("YELLOW"), 1 },
    { wxT("PURPLE"), 1 },
    { wxT("BLACK"), 1 },
    { wxT("PURPLE"), 1 },
    { wxT("BLACK"), 1 },
    { wxT("BLUE"), 1 },
    { wxT("NAVY"), 1 },
    { wxT("PURPLE"), 1 },
    { wxT("PLUM"), 1 },
    { wxT("WHITE"), 1 },
    { wxT("YELLOW"), 1 },
    { wxT("SKY BLUE"), 1 },
    { wxT("RED"), 1 },
    { wxT("RED"), 1 },
    { wxT("RED"), 1 },
    { wxT("PURPLE"), 1 },
    { wxT("CORAL"), 1 },
    { wxT("ORANGE"), 2 },
    { wxT("RED"), 1 },
};

const wxString ContCellColorInfo[COLOR_CONTCELLS_NUM] = {
    "CONT CELL PROCEDURE",
    "CONT CELL VALUE",
    "CONT CELL FUNCTION",
    "CONT CELL DIRECTION",
    "CONT CELL STEPTYPE",
    "CONT CELL POINT",
    "CONT CELL UNSET",
};

const std::tuple<wxString, int> ContCellColorInfoDefaults[COLOR_CONTCELLS_NUM] = {
    { "LIME GREEN", 1 },
    { "YELLOW", 1 },
    { "SLATE BLUE", 1 },
    { "MEDIUM ORCHID", 1 },
    { "SKY BLUE", 1 },
    { "GOLD", 1 },
    { "WHITE", 1 },
};

///// Show mode configuration /////

const wxString kShowModeStrings[SHOWMODE_NUM] = {
    wxT("Standard"), wxT("Full Field"), wxT("Tunnel"), wxT("Old Field"),
    wxT("Pro Field")
};

// What values mean:
// whash ehash (steps from west sideline)
// left top right bottom (border in steps)
// x y w h (region of the field to use, in steps)
const std::array<long, CalChartConfiguration::kShowModeValues>
    kShowModeDefaultValues[SHOWMODE_NUM] = {
        { { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } },
        { { 32, 52, 8, 8, 8, 8, -96, -42, 192, 84 } },
        { { 32, 52, 8, 8, 8, 8, 16, -42, 192, 84 } },
        { { 28, 52, 8, 8, 8, 8, -80, -42, 160, 84 } },
        { { 36, 48, 8, 8, 8, 8, -80, -42, 160, 84 } }
    };

// Yard lines
const std::array<wxString, kYardTextValues> yard_text_defaults = []() {
    std::array<wxString, kYardTextValues> values;
    auto default_yards = ShowMode::GetDefaultYardLines();
    for (auto i = 0; i < kYardTextValues; ++i) {
        values[i] = default_yards[i];
    }
    return values;
}();

const auto yard_text_index = yard_text_defaults;

const wxString kPaletteColorDefault[kNumberPalettes] = {
    wxT("FOREST GREEN"),
    wxT("GREY"),
    wxT("GREY"),
    wxT("GREY"),
};

const wxString kPaletteNameDefault[kNumberPalettes] = {
    wxT("Default"),
    wxT("[Unset]"),
    wxT("[Unset]"),
    wxT("[Unset]"),
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
T GetConfigPathKey(const wxString& path, const wxString& key, const T& def)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(path);
    T value = def;
    config->Read(key, &value);
    return value;
}

// clear out the config if it matches
template <typename T>
void ClearConfigPathKey(const wxString& path, const wxString& key)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(path);
    config->DeleteEntry(key);
    config->Flush();
}

// default value need to check if we need to set a value
template <typename T>
void SetConfigPathKey(const wxString& path, const wxString& key,
    const T& value)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->SetPath(path);
    config->Write(key, value);
    config->Flush();
}

// functions for dealing with the wx config directly
template <typename T>
T GetConfigValue(const wxString& key, const T& def)
{
    return GetConfigPathKey<T>(wxT("/CalChart"), key, def);
}

// clear out the config if it matches
// template <typename T>
// void ClearConfigValue(const wxString& key)
//{
//	wxConfigBase *config = wxConfigBase::Get();
//	config->SetPath(wxT("/CalChart"));
//	config->DeleteEntry(key);
//	config->Flush();
//}

// default value need to check if we need to set a value
template <typename T>
void SetConfigValue(const wxString& key, const T& value, const T& def)
{
    // don't write if we don't have to
    if (GetConfigValue<T>(key, def) == value)
        return;
    // clear out the value if it's the same as the default
    if (def == value) {
        ClearConfigPathKey<T>(wxT("/CalChart"), key);
        return;
    }
    SetConfigPathKey(wxT("/CalChart"), key, value);
}

static CalChartConfiguration::ColorWidth_t
GetColorConfigValueForPalette(int palette, const wxString& key, const CalChartConfiguration::ColorWidth_t& def)
{
    auto r = std::get<0>(def).Red();
    auto g = std::get<0>(def).Green();
    auto b = std::get<0>(def).Blue();
    auto rkey = key + wxT("_Red");
    auto gkey = key + wxT("_Green");
    auto bkey = key + wxT("_Blue");
    auto palettekey = std::string("/PALETTE") + std::to_string(palette) + "/COLORS";
    r = GetConfigPathKey<long>(palettekey, rkey, r);
    g = GetConfigPathKey<long>(palettekey, gkey, g);
    b = GetConfigPathKey<long>(palettekey, bkey, b);

    auto w = std::get<1>(def);
    w = GetConfigPathKey<long>(palettekey + "/WIDTH", key, w);

    return CalChartConfiguration::ColorWidth_t(wxColour(r, g, b), w);
}

// Specialize on Color
template <>
CalChartConfiguration::ColorWidth_t
GetConfigValue(wxString const& key, CalChartConfiguration::ColorWidth_t const& def)
{
    return GetColorConfigValueForPalette(GetConfigValue(wxT("ActiveColorPalette"), 0), key, def);
}

static void SetColorConfigValueForPalette(int palette, wxString const& key, CalChartConfiguration::ColorWidth_t const& value, CalChartConfiguration::ColorWidth_t const& def)
{
    // don't write if we don't have to
    if (GetColorConfigValueForPalette(palette, key, def) == value)
        return;
    auto rkey = key + wxT("_Red");
    auto gkey = key + wxT("_Green");
    auto bkey = key + wxT("_Blue");

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

    auto r = std::get<0>(value).Red();
    auto g = std::get<0>(value).Green();
    auto b = std::get<0>(value).Blue();
    auto w = std::get<1>(value);
    SetConfigPathKey<long>(palettekey, rkey, r);
    SetConfigPathKey<long>(palettekey, gkey, g);
    SetConfigPathKey<long>(palettekey, bkey, b);
    SetConfigPathKey<long>(palettekey + "/WIDTH", key, w);
}

// Specialize on Color
template <>
void SetConfigValue(const wxString& key,
    const CalChartConfiguration::ColorWidth_t& value,
    const CalChartConfiguration::ColorWidth_t& def)
{
    SetColorConfigValueForPalette(GetConfigValue(wxT("ActiveColorPalette"), 0), key, value, def);
}

// Specialize on show mode
wxString ShowModeKeys[CalChartConfiguration::kShowModeValues] = {
    wxT("whash"), wxT("ehash"), wxT("bord1_x"), wxT("bord1_y"),
    wxT("bord2_x"), wxT("bord2_y"), wxT("size_x"), wxT("size_y"),
    wxT("offset_x"), wxT("offset_y")
};

template <>
CalChartConfiguration::ShowModeInfo_t
GetConfigValue(const wxString& key,
    const CalChartConfiguration::ShowModeInfo_t& def)
{
    auto values = def;
    wxString path = wxT("/SHOWMODES/") + key;
    for (auto i = 0; i < CalChartConfiguration::kShowModeValues; ++i) {
        values[i] = GetConfigPathKey<long>(path, ShowModeKeys[i], values[i]);
    }
    return values;
}

// Specialize on show mode
template <>
void SetConfigValue(const wxString& key,
    const CalChartConfiguration::ShowModeInfo_t& value,
    const CalChartConfiguration::ShowModeInfo_t& def)
{
    // don't write if we don't have to
    if (GetConfigValue<CalChartConfiguration::ShowModeInfo_t>(key, def) == value)
        return;
    wxString path = wxT("/SHOWMODES/") + key;

    // TODO: fix this so it clears
    // clear out the value if it's the same as the default
    if (def == value) {
        for (auto i = 0; i < CalChartConfiguration::kShowModeValues; ++i) {
            ClearConfigPathKey<long>(path, ShowModeKeys[i]);
        }
        return;
    }

    for (auto i = 0; i < CalChartConfiguration::kShowModeValues; ++i) {
        SetConfigPathKey<long>(path, ShowModeKeys[i], value[i]);
    }
}

#define IMPLEMENT_CONFIGURATION_FUNCTIONS(KeyName, Type, TheValue)                                             \
    static const wxString k##KeyName##Key = wxT(#KeyName);                                                     \
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
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintFile, wxString, wxT("LPT1"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintCmd, wxString, wxT("lpr"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintOpts, wxString, wxT(""));
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintViewCmd, wxString, wxT("ghostview"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintViewOpts, wxString, wxT(""));

IMPLEMENT_CONFIGURATION_FUNCTIONS(PageWidth, double, 7.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PageHeight, double, 10.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PageOffsetX, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PageOffsetY, double, 0.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PaperLength, double, 11.0);

IMPLEMENT_CONFIGURATION_FUNCTIONS(HeadFont, wxString, wxT("Palatino-Bold"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(MainFont, wxString, wxT("Helvetica"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(NumberFont, wxString, wxT("Helvetica-Bold"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContFont, wxString, wxT("Courier"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(BoldFont, wxString, wxT("Courier-Bold"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(ItalFont, wxString, wxT("Courier-Italic"));
IMPLEMENT_CONFIGURATION_FUNCTIONS(BoldItalFont, wxString,
    wxT("Courier-BoldItalic"));

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

IMPLEMENT_CONFIGURATION_FUNCTIONS(CalChartFrameAUILayout_3_6_0, wxString, wxT(""));

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

template <int Which, typename ColorArray>
auto GetFromColorArray(ColorArray const& array)
{
    using array_type = std::decay_t<decltype(*array)>;
    using element_type = typename std::tuple_element<Which, array_type>::type;
    std::vector<element_type> result;
    std::transform(std::begin(array), std::end(array), std::back_inserter(result),
        [](auto&& i) {
            return std::get<Which>(i);
        });
    return result;
}

std::vector<wxString> CalChartConfiguration::GetColorNames() const
{
    return { ColorInfo, ColorInfo + sizeof(ColorInfo) / sizeof(ColorInfo[0]) };
}

std::vector<wxString> CalChartConfiguration::GetDefaultColors() const
{
    return GetFromColorArray<0>(ColorInfoDefaults);
}

std::vector<int> CalChartConfiguration::GetDefaultPenWidth() const
{
    return GetFromColorArray<1>(ColorInfoDefaults);
}

std::vector<wxString> CalChartConfiguration::GetContCellColorNames() const
{
    return { ContCellColorInfo, ContCellColorInfo + sizeof(ContCellColorInfo) / sizeof(ContCellColorInfo[0]) };
}

std::vector<wxString> CalChartConfiguration::GetContCellDefaultColors() const
{
    return GetFromColorArray<0>(ContCellColorInfoDefaults);
}

std::vector<int> CalChartConfiguration::GetContCellDefaultPenWidth() const
{
    return GetFromColorArray<1>(ContCellColorInfoDefaults);
}

std::vector<wxString> CalChartConfiguration::Get_yard_text_index() const
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

wxBrush CalChartConfiguration::GetColorPaletteColor(long which) const
{
    auto colorWidth = GetColorConfigValueForPalette(which, std::string("PaletteColor"), { kPaletteColorDefault[which], 1 });
    return *wxTheBrushList->FindOrCreateBrush(std::get<0>(colorWidth), wxBRUSHSTYLE_SOLID);
}

void CalChartConfiguration::SetColorPaletteColor(long which, wxBrush const& brush)
{
    CalChartConfiguration::ColorWidth_t v{ brush.GetColour(), 1 };
    mWriteQueue[std::string("PaletteColor") + std::to_string(which)] = [which, v]() {
        SetColorConfigValueForPalette(which, std::string("PaletteColor"), v, { kPaletteColorDefault[which], 1 });
    };
}

void CalChartConfiguration::ClearColorPaletteColor(long which)
{
    CalChartConfiguration::ColorWidth_t v{ kPaletteColorDefault[which], 1 };
    mWriteQueue[std::string("PaletteColor") + std::to_string(which)] = [which, v]() {
        SetColorConfigValueForPalette(which, std::string("PaletteColor"), v, v);
    };
}

std::vector<wxBrush> CalChartConfiguration::GetDefaultColorPaletteColors() const
{
    auto results = std::vector<wxBrush>{};
    std::transform(std::begin(kPaletteColorDefault), std::end(kPaletteColorDefault), std::back_inserter(results), [](auto&& i) {
        return *wxTheBrushList->FindOrCreateBrush(i, wxBRUSHSTYLE_SOLID);
    });

    return results;
}

wxString CalChartConfiguration::GetColorPaletteName(long which) const
{
    if (which > kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    return GetConfigValue<wxString>(std::string("PaletteName") + std::to_string(which), kPaletteNameDefault[which]);
}

void CalChartConfiguration::SetColorPaletteName(long which, wxString const& name)
{
    mWriteQueue[std::string("PaletteName") + std::to_string(which)] = [which, name]() {
        SetConfigValue<wxString>(std::string("PaletteName") + std::to_string(which), name, kPaletteNameDefault[which]);
    };
}

void CalChartConfiguration::ClearColorPaletteName(long which)
{
    mWriteQueue[std::string("PaletteName") + std::to_string(which)] = [which]() {
        SetConfigValue<wxString>(std::string("PaletteName") + std::to_string(which), kPaletteNameDefault[which], kPaletteNameDefault[which]);
    };
}

std::vector<wxString> CalChartConfiguration::GetDefaultColorPaletteNames() const
{
    return { std::begin(kPaletteNameDefault), std::end(kPaletteNameDefault) };
}

template <typename Color, typename Map, typename Info, typename InfoDefault>
std::pair<wxBrush, wxPen>
Get_BrushAndPen(Color c, Map& colorsAndWidth, Info const& InfoArray, InfoDefault const& InfoDefaultArray)
{
    if (!colorsAndWidth.count(c)) {
        colorsAndWidth[c] = GetConfigValue<CalChartConfiguration::ColorWidth_t>(
            InfoArray[c],
            CalChartConfiguration::ColorWidth_t(std::get<0>(InfoDefaultArray[c]), std::get<1>(InfoDefaultArray[c])));
    }
    auto colorAndWidth = colorsAndWidth[c];
    return {
        *wxTheBrushList->FindOrCreateBrush(std::get<0>(colorAndWidth), wxBRUSHSTYLE_SOLID),
        *wxThePenList->FindOrCreatePen(std::get<0>(colorAndWidth), std::get<1>(colorAndWidth), wxPENSTYLE_SOLID)
    };
}

template <typename Color, typename Map, typename Info, typename InfoDefault, typename WriteQueue>
void Set_BrushAndPen(Color c, const wxBrush& brush, const wxPen& pen, Map& colorsAndWidth, Info const& InfoArray, InfoDefault const& InfoDefaultArray, WriteQueue& writeQueue)
{
    CalChartConfiguration::ColorWidth_t v{ brush.GetColour(), pen.GetWidth() };

    writeQueue[InfoArray[c]] = [info = InfoArray[c], infodefault = InfoDefaultArray[c], v]() {
        SetConfigValue<CalChartConfiguration::ColorWidth_t>(
            info, v,
            CalChartConfiguration::ColorWidth_t(std::get<0>(infodefault), std::get<1>(infodefault)));
    };
    colorsAndWidth[c] = v;
}

template <typename Color, typename Map, typename Info, typename InfoDefault>
void Clear_ConfigColor(Color selection, Map& colorsAndWidth, Info const& InfoArray, InfoDefault const& InfoDefaultArray)
{
    auto default_value = CalChartConfiguration::ColorWidth_t(std::get<0>(InfoDefaultArray[selection]),
        std::get<1>(InfoDefaultArray[selection]));
    SetConfigValue<CalChartConfiguration::ColorWidth_t>(InfoArray[selection], default_value,
        default_value);
    // clear out the cached value
    colorsAndWidth.erase(selection);
}

std::pair<wxBrush, wxPen>
CalChartConfiguration::Get_CalChartBrushAndPen(CalChartColors c) const
{
    return Get_CalChartBrushAndPen(GetActiveColorPalette(), c);
}

void CalChartConfiguration::Set_CalChartBrushAndPen(CalChartColors c,
    const wxBrush& brush,
    const wxPen& pen)
{
    return Set_CalChartBrushAndPen(GetActiveColorPalette(), c, brush, pen);
}

void CalChartConfiguration::Clear_CalChartConfigColor(CalChartColors selection)
{
    return Clear_CalChartConfigColor(GetActiveColorPalette(), selection);
}

std::pair<wxBrush, wxPen>
CalChartConfiguration::Get_CalChartBrushAndPen(int palette, CalChartColors c) const
{
    if (c >= COLOR_NUM)
        throw std::runtime_error("Error, exceeding COLOR_NUM size");
    if (palette >= kNumberPalettes)
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    return Get_BrushAndPen(c, mColorsAndWidth[palette], ColorInfo, ColorInfoDefaults);
}

void CalChartConfiguration::Set_CalChartBrushAndPen(int palette, CalChartColors c, wxBrush const& brush, wxPen const& pen)
{
    if (c >= COLOR_NUM)
        throw std::runtime_error("Error, exceeding COLOR_NUM size");
    if (palette >= kNumberPalettes)
        throw std::runtime_error("Error, exceeding kNumberPalettes size");

    Set_BrushAndPen(c, brush, pen, mColorsAndWidth[palette], ColorInfo, ColorInfoDefaults, mWriteQueue);
}

void CalChartConfiguration::Clear_CalChartConfigColor(int palette, CalChartColors selection)
{
    if (selection >= COLOR_NUM)
        throw std::runtime_error("Error, exceeding COLOR_NUM size");
    if (palette >= kNumberPalettes)
        throw std::runtime_error("Error, exceeding kNumberPalettes size");

    Clear_ConfigColor(selection, mColorsAndWidth[palette], ColorInfo, ColorInfoDefaults);
}

std::pair<wxBrush, wxPen>
CalChartConfiguration::Get_ContCellBrushAndPen(ContCellColors c) const
{
    if (c >= COLOR_CONTCELLS_NUM)
        throw std::runtime_error("Error, exceeding COLOR_CONTCELLS_NUM size");

    return Get_BrushAndPen(c, mContCellColorsAndWidth, ContCellColorInfo, ContCellColorInfoDefaults);
}

void CalChartConfiguration::Set_ContCellBrushAndPen(ContCellColors c,
    const wxBrush& brush,
    const wxPen& pen)
{
    if (c >= COLOR_CONTCELLS_NUM)
        throw std::runtime_error("Error, exceeding COLOR_CONTCELLS_NUM size");

    Set_BrushAndPen(c, brush, pen, mContCellColorsAndWidth, ContCellColorInfo, ContCellColorInfoDefaults, mWriteQueue);
}

void CalChartConfiguration::Clear_ContCellConfigColor(ContCellColors selection)
{
    if (selection >= COLOR_CONTCELLS_NUM)
        throw std::runtime_error("Error, exceeding COLOR_CONTCELLS_NUM size");

    Clear_ConfigColor(selection, mContCellColorsAndWidth, ContCellColorInfo, ContCellColorInfoDefaults);
}

///// Show Configuration /////

ShowMode::ShowModeInfo_t
CalChartConfiguration::Get_ShowModeInfo(CalChartShowModes which) const
{
    if (which >= SHOWMODE_NUM)
        throw std::runtime_error("Error, exceeding SHOWMODE_NUM size");

    if (!mShowModeInfos.count(which)) {
        mShowModeInfos[which] = GetConfigValue<ShowModeInfo_t>(
            kShowModeStrings[which], kShowModeDefaultValues[which]);
    }
    return mShowModeInfos[which];
}

void CalChartConfiguration::Set_ShowModeInfo(CalChartShowModes which,
    const ShowModeInfo_t& values)
{
    if (which >= SHOWMODE_NUM)
        throw std::runtime_error("Error, exceeding SHOWMODE_NUM size");

    mWriteQueue[kShowModeStrings[which]] = [which, values]() {
        SetConfigValue<ShowModeInfo_t>(kShowModeStrings[which], values,
            kShowModeDefaultValues[which]);
    };
    mShowModeInfos[which] = values;
}

void CalChartConfiguration::Clear_ShowModeInfo(CalChartShowModes which)
{
    if (which >= SHOWMODE_NUM)
        throw std::runtime_error("Error, exceeding SHOWMODE_NUM size");

    auto default_value = kShowModeDefaultValues[which];
    SetConfigValue<ShowModeInfo_t>(kShowModeStrings[which], default_value,
        default_value);
    mShowModeInfos.erase(which);
}

// Yard Lines
wxString CalChartConfiguration::Get_yard_text(size_t which) const
{
    if (which >= kYardTextValues)
        throw std::runtime_error("Error, exceeding kYardTextValues size");

    if (!mYardTextInfos.count(which)) {
        wxString key;
        key.Printf(wxT("YardLines_%d"), static_cast<int>(which));
        mYardTextInfos[which] = GetConfigValue(key, yard_text_defaults[which]);
    }
    return mYardTextInfos[which];
}

void CalChartConfiguration::Set_yard_text(size_t which, const wxString& value)
{
    if (which >= kYardTextValues)
        throw std::runtime_error("Error, exceeding kYardTextValues size");

    wxString key;
    key.Printf(wxT("YardLines_%d"), static_cast<int>(which));
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

    wxString key;
    key.Printf(wxT("YardLines_%d"), static_cast<int>(which));
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

ShowMode GetConfigShowMode(const wxString& which)
{
    auto iter = std::find(std::begin(kShowModeStrings),
        std::end(kShowModeStrings), which);
    if (iter != std::end(kShowModeStrings)) {
        auto item = static_cast<CalChartShowModes>(
            std::distance(std::begin(kShowModeStrings), iter));
        return ShowMode::CreateShowMode(CalChartConfiguration::GetGlobalConfig().Get_ShowModeInfo(item), Get_yard_text_all(CalChartConfiguration::GetGlobalConfig()));
    }
    throw std::runtime_error("No such show");
}

std::vector<wxBrush> GetColorPaletteColors(CalChartConfiguration const& config)
{
    auto result = std::vector<wxBrush>{};
    for (auto i = 0; i < kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteColor(i));
    }
    return result;
}

std::vector<wxString> GetColorPaletteNames(CalChartConfiguration const& config)
{
    auto result = std::vector<wxString>{};
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
