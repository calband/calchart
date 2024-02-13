/*
 * CalChartConfiguration.cpp
 * Functions for manipulating configuration Settings
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartConfiguration.h"
#include "CalChartApp.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartShowMode.h"
#include "cc_omniview_constants.h"
#include <array>
#include <string>
#include <wx/config.h>

namespace {
auto GetConfig() -> CalChartConfiguration&
{
    static CalChartConfiguration sconfig;
    return sconfig;
}
}

auto CalChartConfiguration::GetGlobalConfig() -> CalChartConfiguration&
{
    return GetConfig();
}

void CalChartConfiguration::AssignConfig(CalChartConfiguration const& config)
{
    // now flush out the config
    GetConfig() = config;
    GetConfig().FlushWriteQueue();
}

auto GetAndConfigureConfig(std::string const& path, std::string const& subpath) -> wxConfigBase*
{
    auto* config = wxConfigBase::Get();
    auto fullpath = "/" + path + "/" + subpath;
    config->SetPath(fullpath);
    return config;
}

// Get, Clear and Set PathKey are the primatives; do not use these directly.
// we assume that path and subpath do not have leading/trailing "/"
template <typename T>
auto GetConfigPathKey(std::string const& path, std::string const& subpath, std::string const& key) -> std::optional<T>
{
    auto* config = GetAndConfigureConfig(path, subpath);
    auto value = T{};
    if (!config->Read(key, &value)) {
        return {};
    }
    return value;
}

template <typename T>
auto GetConfigPathKey(std::string const& path, std::string const& key) -> std::optional<T>
{
    return GetConfigPathKey<T>(path, "", key);
}

// default value need to check if we need to set a value
template <typename T>
void SetConfigPathKey(std::string const& path, std::string const& subpath, std::string const& key, T const& value)
{
    auto* config = GetAndConfigureConfig(path, subpath);
    config->Write(key, value);
    config->Flush();
}

template <typename T>
void SetConfigPathKey(std::string const& path, std::string const& key, T const& value)
{
    SetConfigPathKey(path, "", key, value);
}

// clear out the config if it matches
void ClearConfigPathKey(std::string const& path, std::string const& subpath, std::string const& key)
{
    auto* config = GetAndConfigureConfig(path, subpath);
    config->DeleteEntry(key);
    config->Flush();
}

void ClearConfigPathKey(std::string const& path, std::string const& key)
{
    ClearConfigPathKey(path, "", key);
}

// functions for dealing with the wx config directly
template <typename T>
auto GetConfigValue(std::string const& key, T const& def) -> T
{
    return GetConfigPathKey<T>("CalChart", key).value_or(def);
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
void SetConfigValue(std::string const& key, T const& value, T const& def)
{
    // don't write if we don't have to
    if (GetConfigValue<T>(key, def) == value) {
        return;
    }
    // clear out the value if it's the same as the default
    if (def == value) {
        ClearConfigPathKey("CalChart", key);
        return;
    }
    SetConfigPathKey("CalChart", key, value);
}

namespace {

auto GetColorWidthKeys(std::optional<int> palette, std::string key) -> std::array<std::string, 6>
{
    auto rkey = key + "_Red";
    auto gkey = key + "_Green";
    auto bkey = key + "_Blue";
    auto wkey = key + "_Width";
    auto path = "CalChart/COLORS";
    auto subpath = palette ? "PALETTE" + std::to_string(*palette) : "";
    return { rkey, gkey, bkey, wkey, path, subpath };
}

// if a palette is not supplied, we just use the Color path, otherwise, we look an a particular palette.
auto GetColorConfigValueForPalette(std::optional<int> palette, std::string const& key, CalChartConfiguration::ColorWidth_t const& def) -> CalChartConfiguration::ColorWidth_t
{
    auto [rkey, gkey, bkey, wkey, path, subpath] = GetColorWidthKeys(palette, key);
    auto r = GetConfigPathKey<long>(path, subpath, rkey);
    auto g = GetConfigPathKey<long>(path, subpath, gkey);
    auto b = GetConfigPathKey<long>(path, subpath, bkey);
    auto w = GetConfigPathKey<long>(path, subpath, wkey);

    if (r && g && b && w) {
        return { CalChart::ColorRGB(*r, *g, *b), *w };
    }
    return def;
}

void SetColorConfigValueForPalette(std::optional<int> palette, std::string const& key, CalChartConfiguration::ColorWidth_t const& value, CalChartConfiguration::ColorWidth_t const& def)
{
    // don't write if we don't have to
    if (GetColorConfigValueForPalette(palette, key, def) == value) {
        return;
    }
    auto [rkey, gkey, bkey, wkey, path, subpath] = GetColorWidthKeys(palette, key);
    if (def == value) {
        ClearConfigPathKey(path, subpath, rkey);
        ClearConfigPathKey(path, subpath, gkey);
        ClearConfigPathKey(path, subpath, bkey);
        ClearConfigPathKey(path, subpath, wkey);
        return;
    }

    auto [r, g, b] = wxCalChart::toRGB(std::get<0>(value));
    auto w = std::get<1>(value);
    SetConfigPathKey(path, subpath, rkey, r);
    SetConfigPathKey(path, subpath, gkey, g);
    SetConfigPathKey(path, subpath, bkey, b);
    SetConfigPathKey(path, subpath, wkey, w);
}

void ClearColorConfigValueForPalette(std::optional<int> palette, std::string const& key)
{
    auto [rkey, gkey, bkey, wkey, path, subpath] = GetColorWidthKeys(palette, key);
    ClearConfigPathKey(path, subpath, rkey);
    ClearConfigPathKey(path, subpath, gkey);
    ClearConfigPathKey(path, subpath, bkey);
    ClearConfigPathKey(path, subpath, wkey);
}

}

template <>
auto GetConfigValue(
    std::string const& key,
    CalChartConfiguration::ColorWidth_t const& def) -> CalChartConfiguration::ColorWidth_t
{
    return GetColorConfigValueForPalette({}, key, def);
}

template <>
void SetConfigValue(
    std::string const& key,
    CalChartConfiguration::ColorWidth_t const& value,
    CalChartConfiguration::ColorWidth_t const& def)
{
    SetColorConfigValueForPalette({}, key, value, def);
}

// Color depends on a palette.  We use overloading instead of specialization here.
auto GetConfigValue(
    int palette,
    std::string const& key,
    CalChartConfiguration::ColorWidth_t const& def) -> CalChartConfiguration::ColorWidth_t
{
    return GetColorConfigValueForPalette(palette, key, def);
}

void SetConfigValue(
    int palette,
    std::string const& key,
    CalChartConfiguration::ColorWidth_t const& value,
    CalChartConfiguration::ColorWidth_t const& def)
{
    SetColorConfigValueForPalette(palette, key, value, def);
}

template <>
auto GetConfigValue(std::string const& key, CalChart::ShowModeData_t const& def) -> CalChart::ShowModeData_t
{
    auto values = def;
    auto path = "SHOWMODES/" + key;
    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        values[i] = GetConfigPathKey<long>(path, CalChart::ShowModeKeys[i]).value_or(values[i]);
    }
    return values;
}

// Specialize on show mode
template <>
void SetConfigValue(std::string const& key,
    CalChart::ShowModeData_t const& value,
    CalChart::ShowModeData_t const& def)
{
    // don't write if we don't have to
    if (GetConfigValue<CalChart::ShowModeData_t>(key, def) == value) {
        return;
    }
    auto path = "/SHOWMODES/" + key;

    // TODO: fix this so it clears
    // clear out the value if it's the same as the default
    if (def == value) {
        for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
            ClearConfigPathKey(path, CalChart::ShowModeKeys.at(i));
        }
        return;
    }

    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        SetConfigPathKey(path, CalChart::ShowModeKeys.at(i), value.at(i));
    }
}

// Specialize on std::string
template <>
auto GetConfigValue(std::string const& key, std::string const& def) -> std::string
{
    return GetConfigPathKey<wxString>("CalChart", key).value_or(def);
}

template <>
void SetConfigValue(std::string const& key, std::string const& value, std::string const& def)
{
    // don't write if we don't have to
    if (GetConfigValue<wxString>(key, def) == value) {
        return;
    }
    SetConfigPathKey<wxString>("CalChart", key, value);
}

#define IMPLEMENT_CONFIGURATION_FUNCTIONS(KeyName, Type, TheValue)                                             \
    static const auto k##KeyName##Key = #KeyName;                                                              \
    static const Type k##KeyName##Value = (TheValue);                                                          \
    auto CalChartConfiguration::Get_##KeyName() const -> Type                                                  \
    {                                                                                                          \
        if (!m##KeyName) {                                                                                     \
            m##KeyName = GetConfigValue<Type>(k##KeyName##Key, k##KeyName##Value);                             \
        }                                                                                                      \
        return *m##KeyName;                                                                                    \
    }                                                                                                          \
    void CalChartConfiguration::Set_##KeyName(Type const& v)                                                   \
    {                                                                                                          \
        mWriteQueue[k##KeyName##Key] = [v]() { SetConfigValue<Type>(k##KeyName##Key, v, k##KeyName##Value); }; \
        m##KeyName = v;                                                                                        \
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

IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContUseNewDraw, bool, true);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContDotRatio, double, 1.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContPLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContSLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContLinePad, long, 2);

IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSModes, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSLandscape, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSOverview, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSDoCont, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSDoContSheet, bool, false);

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_4, float, kViewPoint_x_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_4, float, kViewPoint_y_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_4, float, kViewPoint_z_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_4, float, kViewAngle_1.getValue());
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_4, float, kViewAngle_z_1.getValue());

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_5, float, kViewPoint_x_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_5, float, kViewPoint_y_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_5, float, kViewPoint_z_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_5, float, kViewAngle_2.getValue());
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_5, float, kViewAngle_z_2.getValue());

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_6, float, kViewPoint_x_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_6, float, kViewPoint_y_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_6, float, kViewPoint_z_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_6, float, kViewAngle_3.getValue());
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_6, float, kViewAngle_z_3.getValue());

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

IMPLEMENT_CONFIGURATION_FUNCTIONS(ActiveColorPalette, long, 0);

// OBSOLETE Settings
// "MainFrameZoom" now obsolete with version post 3.2, use "MainFrameZoom2"
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameZoom, float, 0.5);
// "MainFrameZoom", "MainFrameWidth", "MainFrameHeight" now obsolete with
// version post 3.3.1, use Field versions
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameZoom2, float, 0.5);
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameWidth, long, 600);
// IMPLEMENT_CONFIGURATION_FUNCTIONS( MainFrameHeight, long, 450);

///// Color Configuration /////

auto CalChartConfiguration::GetColorPaletteColor(int which) const -> CalChart::Color
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    if (!mColorPaletteColor.at(which)) {
        auto colorWidth = GetColorConfigValueForPalette(which, "PaletteColor", { wxCalChart::toColor(wxColour{ CalChart::kPaletteColorDefault[which] }), 1 });
        mColorPaletteColor.at(which) = std::get<0>(colorWidth);
    }
    return *mColorPaletteColor.at(which);
}

void CalChartConfiguration::SetColorPaletteColor(int which, CalChart::Color color)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    auto v = CalChartConfiguration::ColorWidth_t{ color, 1 };
    mWriteQueue[std::string("PaletteColor") + std::to_string(which)] = [which, v]() {
        SetColorConfigValueForPalette(which, "PaletteColor", v, { CalChart::kPaletteColorDefault[which], 1 });
    };
    mColorPaletteColor.at(which) = color;
}

void CalChartConfiguration::ClearColorPaletteColor(int which)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    mWriteQueue[std::string("PaletteColor") + std::to_string(which)] = [which]() {
        ClearColorConfigValueForPalette(which, "PaletteColor");
    };
    mColorPaletteColor.at(which).reset();
}

auto CalChartConfiguration::GetColorPaletteName(int which) const -> std::string
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    if (!mColorPaletteName.at(which)) {
        mColorPaletteName.at(which) = GetConfigValue<std::string>(std::string("PaletteName") + std::to_string(which), CalChart::kPaletteNameDefault[which]);
    }
    return *mColorPaletteName.at(which);
}

void CalChartConfiguration::SetColorPaletteName(int which, std::string const& name)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    mWriteQueue[std::string("PaletteName") + std::to_string(which)] = [which, name]() {
        SetConfigValue<std::string>(std::string("PaletteName") + std::to_string(which), name, CalChart::kPaletteNameDefault[which]);
    };
    mColorPaletteName.at(which) = name;
}

void CalChartConfiguration::ClearColorPaletteName(int which)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    mWriteQueue[std::string("PaletteName") + std::to_string(which)] = [which]() {
        SetConfigValue<std::string>(std::string("PaletteName") + std::to_string(which), CalChart::kPaletteNameDefault[which], CalChart::kPaletteNameDefault[which]);
    };
    mColorPaletteName.at(which).reset();
}

// Assumes the index is less than colorsAndWidth/InfoDefaultArray lengths
template <typename Map, typename InfoDefault>
auto Get_BrushAndPen(std::optional<int> palette, int index, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray) -> CalChart::BrushAndPen
{
    if (!colorsAndWidth.at(index)) {
        auto key = std::get<0>(InfoDefaultArray.at(index));
        auto def = CalChartConfiguration::ColorWidth_t{ wxCalChart::toColor(std::get<1>(InfoDefaultArray.at(index))), std::get<2>(InfoDefaultArray.at(index)) };
        colorsAndWidth.at(index) = palette ? GetConfigValue(*palette, key, def) : GetConfigValue(key, def);
    }
    auto colorAndWidth = *colorsAndWidth.at(index);
    return { std::get<0>(colorAndWidth), CalChart::Brush::Style::Solid, CalChart::Pen::Style::Solid, std::get<1>(colorAndWidth) };
}

template <typename Map, typename InfoDefault, typename WriteQueue>
void Set_BrushAndPen(std::optional<int> palette, int index, CalChart::BrushAndPen brushAndPen, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray, WriteQueue& writeQueue)
{
    CalChartConfiguration::ColorWidth_t v{ brushAndPen.color, brushAndPen.width };

    auto key = std::get<0>(InfoDefaultArray.at(index));
    auto def = CalChartConfiguration::ColorWidth_t{ wxCalChart::toColor(std::get<1>(InfoDefaultArray.at(index))), std::get<2>(InfoDefaultArray.at(index)) };
    writeQueue[std::get<0>(InfoDefaultArray.at(index))] = [palette, key, def, v]() {
        if (palette) {
            SetConfigValue(*palette, key, v, def);
        } else {
            SetConfigValue(key, v, def);
        }
    };
    colorsAndWidth.at(index) = v;
}

template <typename Map, typename InfoDefault>
void Clear_BrushAndPen(std::optional<int> palette, int index, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray)
{
    auto key = std::get<0>(InfoDefaultArray.at(index));
    auto def = CalChartConfiguration::ColorWidth_t{ wxCalChart::toColor(std::get<1>(InfoDefaultArray.at(index))), std::get<2>(InfoDefaultArray.at(index)) };
    // TODO: This shouldn't clear right away, should wait.
    if (palette) {
        SetConfigValue(*palette, key, def, def);
    } else {
        SetConfigValue(key, def, def);
    }
    // clear out the cached value
    colorsAndWidth.at(index).reset();
}

auto CalChartConfiguration::Get_CalChartBrushAndPen(int palette, CalChart::Colors which) const -> CalChart::BrushAndPen
{
    if (which >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    return Get_BrushAndPen(palette, toUType(which), mColorsAndWidth[palette], CalChart::ColorInfoDefaults);
}

void CalChartConfiguration::Set_CalChartBrushAndPen(int palette, CalChart::Colors which, CalChart::BrushAndPen brushAndPen)
{
    if (which >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    Set_BrushAndPen(palette, toUType(which), brushAndPen, mColorsAndWidth[palette], CalChart::ColorInfoDefaults, mWriteQueue);
}

void CalChartConfiguration::Clear_CalChartConfigColor(int palette, CalChart::Colors which)
{
    if (which >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    Clear_BrushAndPen(palette, toUType(which), mColorsAndWidth[palette], CalChart::ColorInfoDefaults);
}

auto CalChartConfiguration::Get_ContCellBrushAndPen(CalChart::ContinuityCellColors which) const -> CalChart::BrushAndPen
{
    if (which >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    return Get_BrushAndPen({}, toUType(which), mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults);
}

void CalChartConfiguration::Set_ContCellBrushAndPen(CalChart::ContinuityCellColors which, CalChart::BrushAndPen brushAndPen)
{
    if (which >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    Set_BrushAndPen({}, toUType(which), brushAndPen, mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults, mWriteQueue);
}

void CalChartConfiguration::Clear_ContCellConfigColor(CalChart::ContinuityCellColors which)
{
    if (which >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    Clear_BrushAndPen({}, toUType(which), mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults);
}

///// Show Configuration /////

auto CalChartConfiguration::Get_ShowModeData(CalChart::ShowModes which) const -> CalChart::ShowModeData_t
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }
    if (!mShowModeInfos.at(twhich)) {
        auto& defaultValue = CalChart::kShowModeDefaultValues[twhich];
        mShowModeInfos.at(twhich) = GetConfigValue<CalChart::ShowModeData_t>(
            std::string(std::get<0>(defaultValue)), CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
    }
    return *mShowModeInfos.at(twhich);
}

void CalChartConfiguration::Set_ShowModeData(CalChart::ShowModes which, CalChart::ShowModeData_t const& values)
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }
    auto& defaultValue = CalChart::kShowModeDefaultValues[twhich];
    mWriteQueue[std::string(std::get<0>(defaultValue))] = [defaultValue, values]() {
        SetConfigValue<CalChart::ShowModeData_t>(std::string(std::get<0>(defaultValue)), values,
            CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
    };
    mShowModeInfos.at(twhich) = values;
}

void CalChartConfiguration::Clear_ShowModeData(CalChart::ShowModes which)
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }
    auto& defaultValue = CalChart::kShowModeDefaultValues[twhich];
    SetConfigValue<CalChart::ShowModeData_t>(std::string(std::get<0>(defaultValue)), CalChart::ShowModeData_t{ std::get<1>(defaultValue) },
        CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
    mShowModeInfos.at(twhich).reset();
}

// Yard Lines
auto CalChartConfiguration::Get_yard_text(size_t which) const -> std::string
{
    if (which >= CalChart::kYardTextValues) {
        throw std::runtime_error("Error, exceeding kYardTextValues size");
    }

    if (!mYardTextInfos.at(which)) {
        auto key = std::string{ "YardLines_" } + std::to_string(which);
        auto default_value = CalChart::kDefaultYardLines[which];
        mYardTextInfos.at(which) = GetConfigValue(key, CalChart::kDefaultYardLines[which]);
    }
    return *mYardTextInfos.at(which);
}

void CalChartConfiguration::Set_yard_text(size_t which, std::string const& value)
{
    if (which >= kYardTextValues) {
        throw std::runtime_error("Error, exceeding kYardTextValues size");
    }
    auto key = std::string{ "YardLines_" } + std::to_string(which);
    auto default_value = CalChart::kDefaultYardLines[which];
    mWriteQueue[key] = [key, value, default_value]() {
        SetConfigValue(key, value, default_value);
    };
    mYardTextInfos.at(which) = value;
}

void CalChartConfiguration::Clear_yard_text(size_t which)
{
    if (which >= kYardTextValues) {
        throw std::runtime_error("Error, exceeding kYardTextValues size");
    }
    auto key = std::string{ "YardLines_" } + std::to_string(which);
    auto default_value = CalChart::kDefaultYardLines[which];
    SetConfigValue(key, default_value, default_value);
    mYardTextInfos.at(which).reset();
}

// function technically const because it is changing a mutable value
void CalChartConfiguration::FlushWriteQueue() const
{
    for (auto& i : mWriteQueue) {
        i.second();
    }
    mWriteQueue.clear();
}

auto GetConfigShowMode(CalChartConfiguration const& config, std::string const& which) -> CalChart::ShowMode
{
    auto iter = std::find_if(std::begin(CalChart::kShowModeDefaultValues), std::end(CalChart::kShowModeDefaultValues), [which](auto&& t) {
        return std::get<0>(t) == which;
    });
    if (iter != std::end(CalChart::kShowModeDefaultValues)) {
        auto item = static_cast<CalChart::ShowModes>(std::distance(std::begin(CalChart::kShowModeDefaultValues), iter));
        return CalChart::ShowMode::CreateShowMode(config.Get_ShowModeData(item), Get_yard_text_all(config));
    }
    throw std::runtime_error("No such show");
}

auto GetColorPaletteColors(CalChartConfiguration const& config) -> std::vector<CalChart::Color>
{
    auto result = std::vector<CalChart::Color>{};
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteColor(i));
    }
    return result;
}

auto GetColorPaletteNames(CalChartConfiguration const& config) -> std::vector<std::string>
{
    auto result = std::vector<std::string>{};
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteName(i));
    }
    return result;
}

auto Get_yard_text_all(CalChartConfiguration const& config) -> std::array<std::string, CalChart::kYardTextValues>
{
    auto values = std::array<std::string, CalChart::kYardTextValues>{};
    for (auto i = 0; i < CalChart::kYardTextValues; ++i) {
        values[i] = config.Get_yard_text(i);
    }
    return values;
}
