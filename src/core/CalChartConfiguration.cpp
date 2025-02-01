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
#include "CalChartShowMode.h"

namespace CalChart {
// the layer cake that is the architecture:
// Get/Set/ClearConfigDetailsKey
//   handles writing ConfigurationType items to the ConfigurationDetails.
//   should only be called by Get/Set/ClearConfigPathKey<T>.
// GetConfigPathKey<T>
//   redirects primative and "complicated" types to the appropriate ...ConfigDetailsKey
//   should only be called by Get/Set/ClearConfigValue
// Get/Set/ClearConfigValue<T>
//   Handles making sure the value is different before setting it.
//   should only be called by Get/Set/ClearCachedConfigValue
// Get/Set/ClearCachedConfigValue<T>
//   Handles the caching behavior
// Get, Clear and Set ConfigDetails are the primatives; do not use these directly.
auto GetConfigDetailsKey(ConfigurationDetails const& details, std::string_view key, ConfigurationType const& defaultValue) -> ConfigurationType
{
    return details.Read(key, defaultValue);
}

void SetConfigDetailsKey(ConfigurationDetails& details, std::string_view key, ConfigurationType const& value)
{
    details.Write(key, value);
}

void ClearConfigDetailsKey(ConfigurationDetails& details, std::string_view key)
{
    details.Clear(key);
}

// Get, Set, Clear Paths.  For Primative types, we call directly.  For more complicated types, like ColorWidth,
// this will do "serialization" or a way to break the values down into constituent parts.
template <typename T>
auto GetConfigPathKey(ConfigurationDetails const& details, std::string const& key, T const& defaultValue) -> T
{
    return std::visit(
        [defaultValue](auto&& value) {
            using U = std::decay_t<decltype(value)>;
            if constexpr (std::is_convertible_v<U, T>) {
                return static_cast<T>(value);
            }
            return defaultValue;
        },
        GetConfigDetailsKey(details, key, defaultValue));
}

template <typename T>
void SetConfigPathKey(ConfigurationDetails& details, std::string const& key, T const& value)
{
    SetConfigDetailsKey(details, key, value);
}

template <typename T>
void ClearConfigPathKey(ConfigurationDetails& details, std::string const& key)
{
    ClearConfigDetailsKey(details, key);
}

template <>
auto GetConfigPathKey(ConfigurationDetails const& details, std::string const& key, ColorWidth_t const& defaultValue) -> ColorWidth_t
{
    auto wkey = key + "_Width";
    return {
        GetConfigPathKey(details, key, std::get<0>(defaultValue)),
        GetConfigPathKey(details, wkey, std::get<1>(defaultValue))
    };
}

template <>
void SetConfigPathKey(ConfigurationDetails& details, std::string const& key, ColorWidth_t const& value)
{
    auto wkey = key + "_Width";
    SetConfigPathKey(details, key, std::get<0>(value));
    SetConfigPathKey(details, wkey, std::get<1>(value));
}

template <>
void ClearConfigPathKey<ColorWidth_t>(ConfigurationDetails& details, std::string const& key)
{
    auto wkey = key + "_Width";
    ClearConfigPathKey<ColorWidth_t::first_type>(details, key);
    ClearConfigPathKey<ColorWidth_t::second_type>(details, wkey);
}

template <>
auto GetConfigPathKey(ConfigurationDetails const& details, std::string const& key, CalChart::ShowModeData_t const& defaultValue) -> CalChart::ShowModeData_t
{
    auto values = defaultValue;
    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        values[i] = GetConfigPathKey(details, "SHOWMODES/" + key + "/" + CalChart::ShowModeKeys.at(i), values.at(i));
    }
    return values;
}

template <>
void SetConfigPathKey(ConfigurationDetails& details, std::string const& key, CalChart::ShowModeData_t const& value)
{
    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        SetConfigPathKey(details, "SHOWMODES/" + key + "/" + CalChart::ShowModeKeys.at(i), value.at(i));
    }
}

template <>
void ClearConfigPathKey<CalChart::ShowModeData_t>(ConfigurationDetails& details, std::string const& key)
{
    for (auto i = 0; i < CalChart::kShowModeValues; ++i) {
        ClearConfigPathKey<CalChart::ShowModeData_t::value_type>(details, "SHOWMODES/" + key + "/" + CalChart::ShowModeKeys.at(i));
    }
}

// functions for dealing with the wx config directly
template <typename T>
auto GetConfigValue(ConfigurationDetails const& details, std::string const& key, T const& defaultValue) -> T
{
    return GetConfigPathKey(details, key, defaultValue);
}

// default value need to check if we need to set a value
template <typename T>
void SetConfigValue(ConfigurationDetails& details, std::string const& key, T const& value, T const& defaultValue)
{
    // don't write if we don't have to
    if (GetConfigPathKey<T>(details, key, defaultValue) == value) {
        return;
    }
    // clear out the value if it's the same as the default
    if (defaultValue == value) {
        ClearConfigPathKey<T>(details, key);
        return;
    }
    SetConfigPathKey(details, key, value);
}

template <typename T>
void ClearConfigValue(ConfigurationDetails& details, std::string const& key)
{
    ClearConfigPathKey<T>(details, key);
}

namespace {

    template <typename T, typename Key>
    auto GetCachedConfigValue(std::optional<T>& cache, ConfigurationDetails const* details, Key const& key, T const& defaultValue)
    {
        if (details == nullptr) {
            return defaultValue;
        }
        if (!cache) {
            cache = GetConfigValue<T>(*details, key, defaultValue);
        }
        return *cache;
    }

    template <typename T, typename Key, typename Queue>
    void SetCachedConfigValue(std::optional<T>& cache, Queue& writeQueue, Key const& key, T const& value, T const& defaultValue)
    {
        writeQueue[key] = [key, value, defaultValue](ConfigurationDetails& details) {
            SetConfigValue<T>(details, key, value, defaultValue);
        };
        cache = value;
    }

    template <typename T, typename Key, typename Queue>
    void ClearCachedConfigValue(std::optional<T>& cache, Queue& writeQueue, Key const& key, T const& defaultValue)
    {
        writeQueue[key] = [key](ConfigurationDetails& details) {
            ClearConfigValue<T>(details, key);
        };
        // have the cache act like we've loaded in this value already
        cache = defaultValue;
    }

}

#define IMPLEMENT_CONFIGURATION_FUNCTIONS(KeyName, Type, TheValue)                                   \
    static const auto k##KeyName##Key = #KeyName;                                                    \
    static const Type k##KeyName##Value = (TheValue);                                                \
    auto Configuration::Get_##KeyName() const -> Type                                                \
    {                                                                                                \
        return GetCachedConfigValue(m##KeyName, mDetails.get(), k##KeyName##Key, k##KeyName##Value); \
    }                                                                                                \
    void Configuration::Set_##KeyName(Type const& v)                                                 \
    {                                                                                                \
        SetCachedConfigValue(m##KeyName, mWriteQueue, k##KeyName##Key, v, k##KeyName##Value);        \
    }                                                                                                \
    void Configuration::Clear_##KeyName()                                                            \
    {                                                                                                \
        ClearCachedConfigValue<Type>(m##KeyName, mWriteQueue, k##KeyName##Key, k##KeyName##Value);   \
    }

IMPLEMENT_CONFIGURATION_FUNCTIONS(AutosaveInterval, long, 60);
IMPLEMENT_CONFIGURATION_FUNCTIONS(FeatureCurves, bool, false);

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
IMPLEMENT_CONFIGURATION_FUNCTIONS(BoldItalFont, std::string, "Courier-BoldItalic");

IMPLEMENT_CONFIGURATION_FUNCTIONS(HeaderSize, double, 3.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(YardsSize, double, 1.5);
IMPLEMENT_CONFIGURATION_FUNCTIONS(TextSize, double, 10.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(DotRatio, double, 0.9);
IMPLEMENT_CONFIGURATION_FUNCTIONS(NumRatio, double, 1.35);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(SLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ContRatio, double, 0.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(ControlPointRatio, double, 0.45);

IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContUseNewDraw, bool, true);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContDotRatio, double, 1.0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContPLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContSLineRatio, double, 1.2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContLinePad, long, 2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintContMaxFontSize, long, 10);

IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSModes, long, 0);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSLandscape, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSOverview, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSDoCont, bool, false);
IMPLEMENT_CONFIGURATION_FUNCTIONS(PrintPSDoContSheet, bool, false);

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_4, float, CalChart::kViewPoint_x_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_4, float, CalChart::kViewPoint_y_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_4, float, CalChart::kViewPoint_z_1);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_4, float, CalChart::kViewAngle_1.getValue());
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_4, float, CalChart::kViewAngle_z_1.getValue());

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_5, float, CalChart::kViewPoint_x_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_5, float, CalChart::kViewPoint_y_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_5, float, CalChart::kViewPoint_z_2);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_5, float, CalChart::kViewAngle_2.getValue());
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_5, float, CalChart::kViewAngle_z_2.getValue());

IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_X_6, float, CalChart::kViewPoint_x_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Y_6, float, CalChart::kViewPoint_y_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewPoint_Z_6, float, CalChart::kViewPoint_z_3);
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_6, float, CalChart::kViewAngle_3.getValue());
IMPLEMENT_CONFIGURATION_FUNCTIONS(OmniViewAngle_Z_6, float, CalChart::kViewAngle_z_3.getValue());

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

auto Configuration::GetColorPaletteColor(int which) const -> CalChart::Color
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    return std::get<0>(GetCachedConfigValue(mColorPaletteColor.at(which), mDetails.get(), "PaletteColor" + std::to_string(which), ColorWidth_t{ CalChart::kPaletteColorDefault.at(which), 1 }));
}

void Configuration::SetColorPaletteColor(int which, CalChart::Color const& color)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    auto v = ColorWidth_t{ color, 1 };
    SetCachedConfigValue(mColorPaletteColor.at(which), mWriteQueue, "PaletteColor" + std::to_string(which), v, ColorWidth_t{ CalChart::kPaletteColorDefault.at(which), 1 });
}

void Configuration::ClearColorPaletteColor(int which)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    ClearCachedConfigValue(mColorPaletteColor.at(which), mWriteQueue, "PaletteColor" + std::to_string(which), ColorWidth_t{ CalChart::kPaletteColorDefault.at(which), 1 });
}

auto Configuration::GetColorPaletteName(int which) const -> std::string
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    return GetCachedConfigValue(mColorPaletteName.at(which), mDetails.get(), "PaletteName" + std::to_string(which), CalChart::kPaletteNameDefault.at(which));
}

void Configuration::SetColorPaletteName(int which, std::string const& name)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    SetCachedConfigValue(mColorPaletteName.at(which), mWriteQueue, "PaletteName" + std::to_string(which), name, CalChart::kPaletteNameDefault.at(which));
}

void Configuration::ClearColorPaletteName(int which)
{
    if (which > CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    ClearCachedConfigValue(mColorPaletteName.at(which), mWriteQueue, "PaletteName" + std::to_string(which), CalChart::kPaletteNameDefault.at(which));
}

auto ToBrushAndPen(ColorWidth_t const& in) -> CalChart::BrushAndPen
{
    return { std::get<0>(in), CalChart::Brush::Style::Solid, CalChart::Pen::Style::Solid, std::get<1>(in) };
}

// Assumes the index is less than colorsAndWidth/InfoDefaultArray lengths
template <typename Map, typename InfoDefault>
auto Get_BrushAndPen(ConfigurationDetails const* details, std::optional<int> palette, int index, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray) -> CalChart::BrushAndPen
{
    auto key = std::get<0>(InfoDefaultArray.at(index));
    auto keyWithPalette = key + (palette ? std::to_string(*palette) : "");
    auto defaultValue = ColorWidth_t{ std::get<1>(InfoDefaultArray.at(index)), std::get<2>(InfoDefaultArray.at(index)) };
    return ToBrushAndPen(GetCachedConfigValue(colorsAndWidth.at(index), details, keyWithPalette, defaultValue));
}

template <typename Map, typename InfoDefault, typename WriteQueue>
void Set_BrushAndPen(std::optional<int> palette, int index, CalChart::BrushAndPen brushAndPen, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray, WriteQueue& writeQueue)
{
    ColorWidth_t v{ brushAndPen.color, brushAndPen.width };

    auto key = std::get<0>(InfoDefaultArray.at(index));
    auto keyWithPalette = key + (palette ? std::to_string(*palette) : "");
    auto defaultValue = ColorWidth_t{ std::get<1>(InfoDefaultArray.at(index)), std::get<2>(InfoDefaultArray.at(index)) };

    SetCachedConfigValue(colorsAndWidth.at(index), writeQueue, keyWithPalette, v, defaultValue);
}

template <typename Map, typename InfoDefault, typename WriteQueue>
void Clear_BrushAndPen(std::optional<int> palette, int index, Map& colorsAndWidth, InfoDefault const& InfoDefaultArray, WriteQueue& writeQueue)
{
    auto key = std::get<0>(InfoDefaultArray.at(index));
    auto keyWithPalette = key + (palette ? std::to_string(*palette) : "");
    auto defaultValue = ColorWidth_t{ std::get<1>(InfoDefaultArray.at(index)), std::get<2>(InfoDefaultArray.at(index)) };

    ClearCachedConfigValue(colorsAndWidth.at(index), writeQueue, keyWithPalette, defaultValue);
}

auto Configuration::Get_CalChartBrushAndPen(int palette, CalChart::Colors which) const -> CalChart::BrushAndPen
{
    if (which >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    return Get_BrushAndPen(mDetails.get(), palette, toUType(which), mColorsAndWidth[palette], CalChart::ColorInfoDefaults);
}

void Configuration::Set_CalChartBrushAndPen(int palette, CalChart::Colors which, CalChart::BrushAndPen brushAndPen)
{
    if (which >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    Set_BrushAndPen(palette, toUType(which), brushAndPen, mColorsAndWidth[palette], CalChart::ColorInfoDefaults, mWriteQueue);
}

void Configuration::Clear_CalChartConfigColor(int palette, CalChart::Colors which)
{
    if (which >= CalChart::Colors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::Colors::NUM size");
    }
    if (palette >= CalChart::kNumberPalettes) {
        throw std::runtime_error("Error, exceeding kNumberPalettes size");
    }
    Clear_BrushAndPen(palette, toUType(which), mColorsAndWidth[palette], CalChart::ColorInfoDefaults, mWriteQueue);
}

auto Configuration::Get_ContCellBrushAndPen(CalChart::ContinuityCellColors which) const -> CalChart::BrushAndPen
{
    if (which >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    return Get_BrushAndPen(mDetails.get(), {}, toUType(which), mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults);
}

void Configuration::Set_ContCellBrushAndPen(CalChart::ContinuityCellColors which, CalChart::BrushAndPen brushAndPen)
{
    if (which >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    Set_BrushAndPen({}, toUType(which), brushAndPen, mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults, mWriteQueue);
}

void Configuration::Clear_ContCellConfigColor(CalChart::ContinuityCellColors which)
{
    if (which >= CalChart::ContinuityCellColors::NUM) {
        throw std::runtime_error("Error, exceeding CalChart::ContinuityCellColors::NUM size");
    }
    Clear_BrushAndPen({}, toUType(which), mContCellColorsAndWidth, CalChart::ContCellColorInfoDefaults, mWriteQueue);
}

///// Show Configuration /////

auto Configuration::Get_ShowModeData(CalChart::ShowModes which) const -> CalChart::ShowModeData_t
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }
    auto& defaultValue = CalChart::kShowModeDefaultValues.at(twhich);
    return GetCachedConfigValue(mShowModeInfos.at(twhich), mDetails.get(), std::get<0>(defaultValue), CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
}

void Configuration::Set_ShowModeData(CalChart::ShowModes which, CalChart::ShowModeData_t const& values)
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }
    auto& defaultValue = CalChart::kShowModeDefaultValues[twhich];
    SetCachedConfigValue(mShowModeInfos.at(twhich), mWriteQueue, std::get<0>(defaultValue), values, CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
}

void Configuration::Clear_ShowModeData(CalChart::ShowModes which)
{
    auto twhich = toUType(which);
    if (twhich >= toUType(CalChart::ShowModes::NUM)) {
        throw std::runtime_error("Error, exceeding CalChart::ShowModes::NUM size");
    }
    auto& defaultValue = CalChart::kShowModeDefaultValues[twhich];
    ClearCachedConfigValue(mShowModeInfos.at(twhich), mWriteQueue, std::get<0>(defaultValue), CalChart::ShowModeData_t{ std::get<1>(defaultValue) });
}

// Yard Lines
auto Configuration::Get_yard_text(size_t which) const -> std::string
{
    if (which >= CalChart::kYardTextValues) {
        throw std::runtime_error("Error, exceeding kYardTextValues size");
    }

    auto key = std::string{ "YardLines_" } + std::to_string(which);
    return GetCachedConfigValue(mYardTextInfos.at(which), mDetails.get(), key, CalChart::kDefaultYardLines.at(which));
}

void Configuration::Set_yard_text(size_t which, std::string const& value)
{
    if (which >= kYardTextValues) {
        throw std::runtime_error("Error, exceeding kYardTextValues size");
    }
    auto key = std::string{ "YardLines_" } + std::to_string(which);
    return SetCachedConfigValue(mYardTextInfos.at(which), mWriteQueue, key, value, CalChart::kDefaultYardLines.at(which));
}

void Configuration::Clear_yard_text(size_t which)
{
    if (which >= kYardTextValues) {
        throw std::runtime_error("Error, exceeding kYardTextValues size");
    }
    auto key = std::string{ "YardLines_" } + std::to_string(which);
    return ClearCachedConfigValue(mYardTextInfos.at(which), mWriteQueue, key, CalChart::kDefaultYardLines.at(which));
}

// function technically const because it is changing a mutable value
void Configuration::FlushWriteQueue() const
{
    if (mDetails == nullptr) {
        return;
    }
    for (auto& i : mWriteQueue) {
        i.second(*mDetails);
    }
    mWriteQueue.clear();
}

auto GetConfigShowMode(Configuration const& config, std::string const& which) -> CalChart::ShowMode
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

auto GetColorPaletteColors(Configuration const& config) -> std::vector<CalChart::Color>
{
    auto result = std::vector<CalChart::Color>{};
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteColor(i));
    }
    return result;
}

auto GetColorPaletteNames(Configuration const& config) -> std::vector<std::string>
{
    auto result = std::vector<std::string>{};
    for (auto i = 0; i < CalChart::kNumberPalettes; ++i) {
        result.push_back(config.GetColorPaletteName(i));
    }
    return result;
}

auto Get_yard_text_all(Configuration const& config) -> std::array<std::string, CalChart::kYardTextValues>
{
    auto values = std::array<std::string, CalChart::kYardTextValues>{};
    for (auto i = 0; i < CalChart::kYardTextValues; ++i) {
        values[i] = config.Get_yard_text(i);
    }
    return values;
}

}