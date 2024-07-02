#pragma once
/*
 * print.cpp
 * Handles all postscript printing to a file stream
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

#include "CalChartConstants.h"
#include "CalChartShowMode.h"

#include <array>
#include <set>

namespace CalChart {

class Show;
class Sheet;
struct Textline;
class ShowMode;
class Configuration;

// intented to print a show to a buffer.
// fonts are the list of fonts in this order:
// std::string head_font_str;
// std::string main_font_str;
// std::string number_font_str;
// std::string cont_font_str;
// std::string bold_font_str;
// std::string ital_font_str;
// std::string bold_ital_font_str;

// pageInfo is a array of:
// double pageWidth,
// double pageHeight,
// double pageOffsetX,
// double pageOffsetY,
// double paperLength,

// textSizes
// double headerSize,
// double yardsSize,
// double textSize,

// ratios
// double dotRatio,
// double NumRatio,
// double PLineRatio,
// double SLineRatio,
// double contRatio,

class PrintShowToPS {
public:
    PrintShowToPS(
        CalChart::Show const& show,
        bool printLandscape,
        bool printDoCont,
        bool printDoContSheet,
        bool printOverview,
        int minYards,
        ShowMode const& mode,
        CalChart::Configuration const& config);

    PrintShowToPS(
        CalChart::Show const& show,
        bool printLandscape,
        bool printDoCont,
        bool printDoContSheet,
        bool printOverview,
        int minYards,
        ShowMode const& mode,
        std::array<std::string, 7> const& fonts_,
        std::array<double, 5> pageInfo,
        std::array<double, 3> textSizes,
        std::array<double, 5> ratios,
        YardLinesInfo_t yardText);

    auto operator()(std::set<size_t> const& isPicked, std::string const& title) const -> std::tuple<std::string, int>;

private:
    [[nodiscard]] auto IsSplitSheet(CalChart::Sheet const& sheet) const -> bool;
    [[nodiscard]] auto GenerateContinuitySheets(int numPagesSoFar) const -> std::tuple<std::string, int>;
    [[nodiscard]] auto GenerateContSections(CalChart::Sheet const& sheet) const -> std::string;
    [[nodiscard]] auto GenerateStandard(CalChart::Sheet const& sheet, bool split_sheet) const -> std::string;
    [[nodiscard]] auto GenerateOverview(CalChart::Sheet const& sheet) const -> std::string;
    [[nodiscard]] auto GenerateHeader(std::string_view title) const -> std::string;
    [[nodiscard]] auto GenerateFieldDefinition() const -> std::string;
    [[nodiscard]] auto GenerateSheets(std::set<size_t> const& isPicked, int numPagesSoFar) const -> std::tuple<std::string, int>;

    CalChart::Show const& mShow;
    bool mPrintLandscape;
    bool mPrintDoCont;
    bool mPrintDoContSheet;
    bool mOverview;
    ShowMode mMode;

    std::array<std::string, 7> fonts;

    double mPageWidth;
    double mPageHeight;
    double mPageOffsetX;
    double mPageOffsetY;
    double mPaperLength;

    double mHeaderSize;
    double mYardsSize;
    double mTextSize;
    double mDotRatio;
    double mNumRatio;
    double mPLineRatio;
    double mSLineRatio;
    double mContRatio;

    YardLinesInfo_t mYardText;

    float width, height, real_width, real_height;
    float field_x, field_y, field_w, field_h;
    float step_size;
    short step_width;
};
}
