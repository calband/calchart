#pragma once
/*
 * CalChartConfiguration.h
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

/**
 * CalChart::Configuration
 *
 * There are many configurable values for CalChart, such as the size some elements are drawn, the
 * default layout of windows, click behaviors, etc.  CalChart::Configuration interfaces with the
 * system config and acts as a "cache" for the values.  By caching the values it allows modification
 * of the configuration -- and to preview what changing a value would look like -- without modifying
 * the system configuration.  On Get, it reads the values from system config, and caches a local copy.
 * On Set (or clear), it updates it's cache, and puts the command into a write-queue. The write-queue
 * needs to be explicitly flushed or the values will be lost.
 *
 * CalChart::Configuration takes a `ConfigurationDetails` object which provides the specialization
 * of writing values out to the system.  The implementation needs to implement a "key/value" interface
 * that operates on 4 fundamental types: Booleans, Integers, Reals, and Strings, along with one
 * specialized value, "Color".
 *
 * Color is the odd one out.  The reason being is that CalChart Core attempts to stay away from RGB
 * specific values, but instead has a table of default color descriptions.  That means the details of
 * how to translate "Forest Green" into actual RBG needs to be determined by the system.  So details
 * about how to "Serialize" the value is left to the System Implementation.
 *
 * "Simple" configuration values are accessible by a "Get_", "Set_", and "Clear_" API.  For example,
 * to reac the AutosaveInterval value:
 *
 * CalChart::Configuration& mConfig; // (set somewhere else)
 *
 * auto save_interval = mConfig.Get_AutosaveInterval();
 *
 * To add a new config value:
 *  Add DECLARE_CONFIGURATION_FUNCTIONS in the class declaration of the right type.  This
 *  will make the Get_, Set_ and Clear_ functions available.  Then in the implementation file, declare
 *  IMPLEMENT_CONFIGURATION_FUNCTIONS with the default.
 *
 * Brushes and Pens are cached as the individual Color and Width, and as such require a more
 * specialized API.
 */

#include "CalChartConstants.h"
#include "CalChartDrawPrimatives.h"
#include <array>
#include <functional>
#include <map>
#include <optional>
#include <vector>

// forward declare
namespace CalChart {
class ShowMode;
}

namespace CalChart {

// Color/Width is a type that combines color and width.
using ColorWidth_t = std::pair<CalChart::Color, int>;
using ConfigurationType = std::variant<bool, int64_t, double, std::string, CalChart::Color>;

inline auto operator<<(std::ostream& os, ConfigurationType const& type) -> std::ostream&
{
    return std::visit(
        [&os](auto&& v) -> std::ostream& {
            return os << v;
        },
        type);
}

// Default is supplied to allow implementation to switch on type
class ConfigurationDetails {
public:
    virtual ~ConfigurationDetails() = default;
    [[nodiscard]] virtual auto Read(std::string_view key, ConfigurationType const& defaultValue) const -> ConfigurationType = 0;
    virtual void Write(std::string_view key, ConfigurationType const& value) = 0;
    virtual void Clear(std::string_view key) = 0;
};

class Configuration {
public:
    explicit Configuration(std::shared_ptr<ConfigurationDetails> details)
        : mDetails{ details }
    {
    }
    ~Configuration() = default;

    // we make the copy/assign explicit to avoid unintential copies of configurations.
    [[nodiscard]] auto Copy() const -> Configuration
    {
        return *this;
    }
    auto Assign(Configuration const& config) -> Configuration&
    {
        return *this = config;
    }

    // explicit flush
    void FlushWriteQueue() const;

private:
    std::shared_ptr<ConfigurationDetails> mDetails;
    Configuration(Configuration const&) = default;
    auto operator=(Configuration const&) -> Configuration& = default;
    mutable std::map<std::string, std::function<void(ConfigurationDetails&)>> mWriteQueue;

// macro for declaring configuration Get_, Set_, and Clear_
#define DECLARE_CONFIGURATION_FUNCTIONS(Key, Type) \
public:                                            \
    [[nodiscard]] auto Get_##Key() const -> Type;  \
    void Set_##Key(Type const& v);                 \
    void Clear_##Key();                            \
                                                   \
private:                                           \
    mutable std::optional<Type> m##Key = {};

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
    DECLARE_CONFIGURATION_FUNCTIONS(ControlPointRatio, double);

    DECLARE_CONFIGURATION_FUNCTIONS(PrintContUseNewDraw, bool);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintContDotRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintContPLineRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintContSLineRatio, double);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintContLinePad, long);
    DECLARE_CONFIGURATION_FUNCTIONS(PrintContMaxFontSize, long);

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
    DECLARE_CONFIGURATION_FUNCTIONS(AnimationFrameSheetSlider, bool);

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

    DECLARE_CONFIGURATION_FUNCTIONS(ActiveColorPalette, long);

    // Update check: version string the user chose to ignore/dismiss (empty = none)
    DECLARE_CONFIGURATION_FUNCTIONS(IgnoredUpdateVersion, std::string);

    // GitHub token: personal access token for bug report submission
    DECLARE_CONFIGURATION_FUNCTIONS(GitHubToken, std::string);

    // Viewer: enable experimental viewer feature
    DECLARE_CONFIGURATION_FUNCTIONS(AllowViewer, bool);

public:
    // color palettes:  The color Palettes allow you to set different "blocks" of
    // colors.
    // When a Palette is set all the sets and gets are treated against that palette
    [[nodiscard]] auto GetColorPaletteColor(int which) const -> CalChart::Color;
    void SetColorPaletteColor(int which, CalChart::Color const&);
    void ClearColorPaletteColor(int which);

private:
    mutable std::array<std::optional<ColorWidth_t>, CalChart::kNumberPalettes> mColorPaletteColor;

public:
    [[nodiscard]] auto GetColorPaletteName(int which) const -> std::string;
    void SetColorPaletteName(int which, std::string const&);
    void ClearColorPaletteName(int which);

private:
    mutable std::array<std::optional<std::string>, CalChart::kNumberPalettes> mColorPaletteName;

public:
    // Colors
    // use the current Active Color Palette to get the Brush and Pen.
    [[nodiscard]] auto Get_CalChartBrushAndPen(CalChart::Colors c) const
    {
        return Get_CalChartBrushAndPen(static_cast<int>(Get_ActiveColorPalette()), c);
    }
    [[nodiscard]] auto Get_CalChartBrushAndPen(int palette, CalChart::Colors c) const -> CalChart::BrushAndPen;
    void Set_CalChartBrushAndPen(int palette, CalChart::Colors c, CalChart::BrushAndPen);
    void Clear_CalChartConfigColor(int palette, CalChart::Colors selection);

private:
    mutable std::array<std::array<std::optional<ColorWidth_t>, toUType(CalChart::Colors::NUM)>, CalChart::kNumberPalettes> mColorsAndWidth;

public:
    [[nodiscard]] auto Get_ContCellBrushAndPen(CalChart::ContinuityCellColors c) const -> CalChart::BrushAndPen;
    void Set_ContCellBrushAndPen(CalChart::ContinuityCellColors c, CalChart::BrushAndPen);
    void Clear_ContCellConfigColor(CalChart::ContinuityCellColors selection);

private:
    mutable std::array<std::optional<ColorWidth_t>, toUType(CalChart::ContinuityCellColors::NUM)> mContCellColorsAndWidth;

public:
    // Shows
    [[nodiscard]] auto Get_ShowModeData(CalChart::ShowModes which) const -> CalChart::ShowModeData_t;
    void Set_ShowModeData(CalChart::ShowModes which, CalChart::ShowModeData_t const& values);
    void Clear_ShowModeData(CalChart::ShowModes which);

private:
    mutable std::array<std::optional<CalChart::ShowModeData_t>, toUType(CalChart::ShowModes::NUM)> mShowModeInfos;

public:
    // Yard Lines
    static constexpr auto kYardTextValues = 53;
    [[nodiscard]] auto Get_yard_text(size_t which) const -> std::string;
    void Set_yard_text(size_t which, std::string const&);
    void Clear_yard_text(size_t which);

private:
    mutable std::array<std::optional<std::string>, kYardTextValues> mYardTextInfos;
};

auto GetColorPaletteColors(Configuration const& config) -> std::vector<CalChart::Color>;
auto GetColorPaletteNames(Configuration const& config) -> std::vector<std::string>;
auto Get_yard_text_all(Configuration const& config) -> std::array<std::string, Configuration::kYardTextValues>;

// to find a specific Show:
auto GetConfigShowMode(Configuration const& config, std::string const& which) -> CalChart::ShowMode;

}
