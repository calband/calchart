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
#include "CalChartPrintShowToPS.hpp"
#include "CalChartConfiguration.h"
#include "CalChartPostScript.h"
#include "CalChartRanges.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "CalChartShowMode.h"

#include "prolog0.h"
#include "prolog2.h"
#include "setup0.h"
#include "setup2.h"

#include <ctime>
#include <format>
#include <sstream>
#include <string>

namespace CalChart {

namespace {
    constexpr auto DPI = 72;

    constexpr auto dot_routines = std::array{
        "dotplain",
        "dotsolid",
        "dotbs",
        "dotsl",
        "dotx",
        "dotsolbs",
        "dotsolsl",
        "dotsolx",
    };

    const auto GeneratePageHeader = [](auto dots_x, auto dots_y) {
        return std::format(
            "0 setgray\n"
            "0.25 setlinewidth\n"
            "{:.2f} {:.2f} translate\n",
            dots_x, dots_y);
    };

    const auto GenerateStartPage = [](auto landscape,
                                       auto translate_x,
                                       auto translate_y,
                                       auto pageOffsetX,
                                       auto paperLength,
                                       auto pageOffsetY,
                                       auto real_width,
                                       auto real_height) -> std::string {
        auto result = GeneratePageHeader(pageOffsetX * DPI, (paperLength - pageOffsetY) * DPI - real_height);
        if (landscape) {
            result += std::format("{:.2f} 0 translate 90 rotate\n", real_width);
        }
        result += std::format("{:.2f} {:.2f} translate\n", translate_x, translate_y);
        return result;
    };

    const auto GeneratePageBreak = []() { return "showpage\n"; };

    const auto GeneratePrintContinuityPreamble = [](auto y_def,
                                                     auto h_def,
                                                     auto rmargin,
                                                     auto tab1,
                                                     auto tab2,
                                                     auto tab3,
                                                     auto font_size) {
        return std::format(
            "/y {:.2f} def\n"
            "/h {:.2f} def\n"
            "/lmargin 0 def /rmargin {:.2f} def\n"
            "/tab1 {:.2f} def /tab2 {:.2f} def /tab3 {:.2f} def\n"
            "/contfont findfont {:.2f} scalefont setfont\n",
            y_def, h_def, rmargin, tab1, tab2, tab3, font_size);
    };

    const auto GeneratePrintTrailer = [](auto num_pages) {
        return std::format("%%Trailer\n%%Pages: {}\n%%EOF\n", num_pages);
    };

    const auto GenerateFieldValues = [](auto step_offset, auto fieldoff, auto j, auto step_size, auto field_h, auto const& yardText) {
        auto which = (step_offset + (kYardTextValues - 1) * 4 + CoordUnits2Int(fieldoff.x) + j) / 8;
        return std::format("/lmargin {:.2f} def /rmargin {:.2f} def\n", step_size * j, step_size * j)
            + std::format("/y {:.2f} def\n", field_h + (step_size / 2))
            + std::format("({}) dup centerText\n", yardText.at(which))
            + std::format("/y {:.2f} def\n"
                          "centerText\n",
                -(step_size * 2));
    };

    const auto GenerateField = [](auto step_offset, auto step_width, auto fieldoff, auto step_size, auto field_h, auto yard_size, auto const& yardText) {
        auto result = std::format(
            "drawfield\n"
            "/mainfont findfont {:.2f} scalefont setfont\n",
            step_size * yard_size);

        for (short j = 0; j <= step_width; j += 8) {
            result += GenerateFieldValues(step_offset, fieldoff, j, step_size, field_h, yardText);
        }
        return result;
    };

    const auto GenerateAllFonts = [](auto const& fonts) -> std::string {
        return std::accumulate(std::begin(fonts), std::end(fonts), std::string{}, [](auto acc, auto font) {
            return acc + " " + font;
        });
    };

    const auto GenerateResourcesPreamble = [](auto const& fonts) -> std::string {
        auto allFonts = GenerateAllFonts(fonts);
        return std::string("%%DocumentNeededResources: font")
            + allFonts
            + "\n"
              "%%DocumentSuppliedResources: font CalChart\n"
              "%%BeginDefaults\n"
              "%%PageResources: font"
            + allFonts
            + " CalChart\n"
            + "%%EndDefaults\n";
    };

    const auto GenerateDateCreated = []() -> std::string {
        time_t t;
        time(&t);
        std::ostringstream os;
        os << "%%CreationDate: " << ctime(&t);
        return os.str();
    };

    const auto GenerateFontHeader = [](auto const& fonts) -> std::string {
        auto allFonts = GenerateAllFonts(fonts);
        auto result = std::string("%%IncludeResources: font")
            + allFonts
            + "\n";

        auto fontname = std::array{
            "head",
            "main",
            "number",
            "cont",
            "bold",
            "ital",
            "boldital",
        };
        auto fontPairs = CalChart::Ranges::zip_view(fontname, fonts);
        result += std::accumulate(std::begin(fontPairs), std::end(fontPairs), std::string{}, [](auto acc, auto font) {
            return acc + std::format("/{}font0 /{} def\n", std::get<0>(font), std::get<1>(font));
        });
        return result;
    };

    const auto GenerateSplitText = [](auto const& sheet, char identifier) -> std::string {
        auto result = std::format("%%Page: {}({})\n", sheet.GetName(), identifier);
        if (!sheet.GetNumber().empty()) {
            result += std::format("/pagenumtext ({}{}) def\n", sheet.GetNumber(), identifier);
        } else {
            result += "/pagenumtext () def\n";
        }
        return result;
    };
    const auto GenerateUnsplitText = [](auto const& sheet) -> std::string {
        auto result = std::format("%%Page: {}\n", sheet.GetName());
        if (!sheet.GetNumber().empty()) {
            result += std::format("/pagenumtext ({}) def\n", sheet.GetNumber());
        } else {
            result += "/pagenumtext () def\n";
        }
        return result;
    };
    const auto GenerateSplitTextNorth = [](auto const& sheet) -> std::string {
        return GenerateSplitText(sheet, 'N');
    };
    const auto GenerateSplitTextSouth = [](auto const& sheet) -> std::string {
        return GenerateSplitText(sheet, 'S');
    };
    const auto GeneratePageNameAndNumber = [](auto const& sheet, auto splitSheet, auto secondSplit) -> std::string {
        if (splitSheet) {
            return GenerateSplitTextNorth(sheet);
        }
        if (secondSplit) {
            return GenerateSplitTextSouth(sheet);
        }
        return GenerateUnsplitText(sheet);
    };

    auto CalculateSplitRangeNorth = [](auto fieldsize, auto step_width, auto const& mode) -> std::tuple<Coord::units, Coord::units, short> {
        auto step_offset = CoordUnits2Int(fieldsize.x) - step_width;
        auto clip_s = Int2CoordUnits(step_offset) + mode.FieldOffset().x; /* south yardline */
        auto clip_n = mode.MaxPosition().x;
        return { clip_s, clip_n, step_offset };
    };
    auto CalculateSplitRangeSouth = [](auto step_width, auto const& mode) -> std::tuple<Coord::units, Coord::units, short> {
        auto step_offset = 0;
        auto clip_s = mode.MinPosition().x;
        auto clip_n = Int2CoordUnits(step_width) + mode.FieldOffset().x; /* north yardline */
        return { clip_s, clip_n, step_offset };
    };
    auto CalculateMaxSouthNorth = [](auto const& sheet, auto const& mode) -> std::tuple<Coord::units, Coord::units> {
        auto fmin = mode.FieldOffset().x;
        auto fmax = mode.FieldSize().x + fmin;

        /* find bounds */
        auto allX = sheet.GetAllPoints()
            | std::views::transform([](auto&& point) { return point.GetPos().x; });
        auto [max_s, max_n] = std::accumulate(allX.begin(), allX.end(), std::pair<Coord::units, Coord::units>{ fmax, fmin }, [](auto acc, auto x) {
            return std::pair<Coord::units, Coord::units>{ std::min(x, std::get<0>(acc)), std::max(x, std::get<1>(acc)) };
        });
        /* make sure bounds are on field */
        return { std::max(max_s, fmin), std::min(max_n, fmax) };
    };
    auto CalculateUnsplitRange = [](auto const& sheet, auto step_width, auto const& mode) -> std::tuple<Coord::units, Coord::units, short> {
        auto pmin = mode.MinPosition().x;
        auto pmax = mode.MaxPosition().x;
        auto fmin = mode.FieldOffset().x;

        /* find bounds */
        auto [max_s, max_n] = CalculateMaxSouthNorth(sheet, mode);

        auto step_offset = (CoordUnits2Int(mode.FieldSize().x) - step_width) / 2;
        step_offset = (step_offset / 8) * 8;
        auto clip_s = pmin;
        auto clip_n = pmax;
        auto x_s = CoordUnits2Int(max_s) - CoordUnits2Int(fmin);
        auto x_n = CoordUnits2Int(max_n) - CoordUnits2Int(fmin);
        if ((x_s < step_offset) || (x_n > (step_offset + step_width))) {
            /* Recenter formation */
            step_offset = x_s - (step_width - (x_n - x_s)) / 2;
            if (step_offset < 0) {
                step_offset = 0;
            } else if ((step_offset + step_width) > CoordUnits2Int(mode.FieldSize().x)) {
                step_offset = CoordUnits2Int(mode.FieldSize().x) - step_width;
            }
            step_offset = (step_offset / 8) * 8;
        }
        return { clip_s, clip_n, step_offset };
    };

    auto CalculateStandardRange = [](auto const& sheet, auto fieldsize, auto step_width, auto const& mode, auto splitSheet, auto secondSplit) -> std::tuple<Coord::units, Coord::units, short> {
        if (splitSheet) {
            return CalculateSplitRangeNorth(fieldsize, step_width, mode);
        }
        if (secondSplit) {
            return CalculateSplitRangeSouth(step_width, mode);
        }
        return CalculateUnsplitRange(sheet, step_width, mode);
    };
}

static std::tuple<float, float, float, float, float>
CalcValues(bool PrintLandscape, bool PrintDoCont, double PageWidth, double PageHeight, double ContRatio)
{
    float width, height, real_width, real_height, field_y;
    if (PrintLandscape) {
        width = PageHeight * DPI;
        if (PrintDoCont) {
            height = PageWidth * (1.0 - ContRatio) * DPI;
            field_y = PageWidth * ContRatio * DPI;
        } else {
            height = PageWidth * DPI;
            field_y = 0;
        }
    } else {
        width = PageWidth * DPI;
        if (PrintDoCont) {
            height = PageHeight * (1.0 - ContRatio) * DPI;
            field_y = PageHeight * ContRatio * DPI;
        } else {
            height = PageHeight * DPI;
            field_y = 0;
        }
    }
    real_width = PageWidth * DPI;
    real_height = PageHeight * DPI;
    return std::tuple<float, float, float, float, float>(
        width, height, real_width, real_height, field_y);
}

static auto
CalcAllValues(bool PrintLandscape, bool PrintDoCont, bool overview,
    double PageWidth, double PageHeight, double ContRatio,
    int min_yards, ShowMode const& mode)
{
    float width{}, height{}, real_width{}, real_height{};
    float field_x{}, field_y{}, field_w{}, field_h{};
    float step_size{};
    short step_width{};

    auto fullsize = mode.Size();
    auto fieldsize = mode.FieldSize();
    auto fullwidth = CoordUnits2Float(fullsize.x);
    auto fullheight = CoordUnits2Float(fullsize.y);
    auto fieldwidth = CoordUnits2Float(fieldsize.x);
    auto fieldheight = CoordUnits2Float(fieldsize.y);

    /* first, calculate dimensions */
    if (!overview) {
        std::tie(width, height, real_width, real_height, field_y) = CalcValues(
            PrintLandscape, PrintDoCont, PageWidth, PageHeight, ContRatio);
        step_width = (short)(width / (height / (fullheight + 8.0)));
        if (step_width > CoordUnits2Int(fieldsize.x)) {
            step_width = CoordUnits2Int(fieldsize.x);
        }
        min_yards = min_yards * 16 / 10;
        if (step_width > min_yards) {
            /* We can get the minimum size */
            step_width = (step_width / 8) * 8;
            field_w = step_width * (height / (fullheight + 8.0));
            field_h = height * (fieldheight / (fullheight + 8.0));
            if ((field_w / step_width * (step_width + 4)) > width) {
                /* Oops, we didn't made it big enough */
                field_h *= width * step_width / (step_width + 4) / field_w;
                field_w = width * step_width / (step_width + 4);
            }
        } else {
            /* Decrease height to get minimum size */
            field_w = width;
            field_h = width * fieldheight / min_yards;
            step_width = min_yards;
        }
        step_size = field_w / step_width;
        field_x = (width - field_w) / 2;
        field_y += height - (field_h + step_size * 16);
    } else {
        if (PrintLandscape) {
            width = PageHeight * DPI;
            height = PageWidth * DPI;
        } else {
            width = PageWidth * DPI;
            height = PageHeight * DPI;
        }
        if ((width * (fullheight / fullwidth)) > height) {
            width = height * (fullwidth / fullheight);
        } else {
            height = width * (fullheight / fullwidth);
        }
        width--;
        height--;
        if (PrintLandscape) {
            real_width = height;
            real_height = width;
        } else {
            real_width = width;
            real_height = height;
        }
        field_x = 8.0 / fullwidth * width;
        field_y = 8.0 / fullheight * height;
        width *= fieldwidth / fullwidth;
        height *= fieldheight / fullheight;
    }
    return std::tuple<float, float, float, float, float,
        float, float, float, float, short>(
        width, height, real_width, real_height, field_x, field_y, field_w,
        field_h,
        step_size, step_width);
}

PrintShowToPS::PrintShowToPS(
    CalChart::Show const& show,
    bool printLandscape,
    bool printDoCont,
    bool printDoContSheet,
    bool printOverview,
    int minYards,
    ShowMode const& mode,
    CalChart::Configuration const& config)
    : PrintShowToPS(show, printLandscape, printDoCont, printDoContSheet, printOverview, minYards, mode,
        { config.Get_HeadFont(), config.Get_MainFont(), config.Get_NumberFont(), config.Get_ContFont(), config.Get_BoldFont(), config.Get_ItalFont(), config.Get_BoldItalFont() },
        { config.Get_PageWidth(), config.Get_PageHeight(), config.Get_PageOffsetX(), config.Get_PageOffsetY(), config.Get_PaperLength() },
        { config.Get_HeaderSize(), config.Get_YardsSize(), config.Get_TextSize() },
        { config.Get_DotRatio(), config.Get_NumRatio(), config.Get_PLineRatio(), config.Get_SLineRatio(), config.Get_ContRatio() },
        mode.Get_yard_text())
{
}

PrintShowToPS::PrintShowToPS(
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
    YardLinesInfo_t yardText)
    : mShow(show)
    , mPrintLandscape(printLandscape)
    , mPrintDoCont(printDoCont)
    , mPrintDoContSheet(printDoContSheet)
    , mOverview(printOverview)
    , mMode(mode)
    , fonts(fonts_)
    , mYardText(std::move(yardText))
{
    std::tie(mPageWidth, mPageHeight, mPageOffsetX, mPageOffsetY, mPaperLength) = pageInfo;
    std::tie(mHeaderSize, mYardsSize, mTextSize) = textSizes;
    std::tie(mDotRatio, mNumRatio, mPLineRatio, mSLineRatio, mContRatio) = ratios;
    std::tie(width, height, real_width, real_height, field_x, field_y, field_w, field_h, step_size, step_width)
        = CalcAllValues(mPrintLandscape, mPrintDoCont, mOverview, mPageWidth, mPageHeight, mContRatio, minYards, mMode);
}

auto PrintShowToPS::operator()(std::set<size_t> const& isPicked, std::string const& title) const -> std::tuple<std::string, int>
{
    /* Now write postscript header */
    auto result = GenerateHeader(title)
        + GenerateFieldDefinition();

    auto numPagesSoFar = 0;
    /* print continuity sheets first */
    if (mPrintDoContSheet && !mOverview) {
        auto [contResult, numContPages] = GenerateContinuitySheets(numPagesSoFar);
        numPagesSoFar = numContPages;
        result += contResult;
    }

    /* do stuntsheet pages now */
    auto [sheetResult, numSheetPages] = GenerateSheets(isPicked, numPagesSoFar);
    numPagesSoFar = numSheetPages;
    result += sheetResult;

    /* finally, write trailer */
    result += GeneratePrintTrailer(numPagesSoFar);

    return { result, numPagesSoFar };
}

auto PrintShowToPS::GenerateHeader(std::string_view title) const -> std::string
{
    auto result = std::format("%!PS-Adobe-3.0\n")
        + std::format("%%BoundingBox: {:.0f} {:.0f} {:.0f} {:.0f}\n",
            mPageOffsetX * DPI,
            (mPaperLength - mPageOffsetY) * DPI - real_height,
            mPageOffsetX * DPI + real_width,
            (mPaperLength - mPageOffsetY) * DPI)
        + GenerateDateCreated()
        + std::format(
            "%%Title: {}\n"
            "%%Creator: CalChart\n"
            "%%Pages: (atend)\n"
            "%%PageOrder: Ascend\n",
            title);
    if (!mOverview) {
        result += GenerateResourcesPreamble(fonts);
    }
    result += "%%EndComments\n";
    return result;
}

auto PrintShowToPS::GenerateFieldDefinition() const -> std::string
{
    if (!mOverview) {
        return std::string("%%BeginProlog\n")
            + std::format("/fieldw {:.2f} def\n", field_w)

            + std::format("/fieldh {:.2f} def\n", field_h)
            + std::format("/fieldy {:.2f} def\n", field_y)
            + std::format("/stepw {} def\n", step_width)
            + std::format("/whash {} def\n", mMode.HashW())
            + std::format("/ehash {} def\n", mMode.HashE())
            + std::format("/headsize {:.2f} def\n", mHeaderSize)
            + std::format("/yardsize {:.2f} def\n", mYardsSize)
            + prolog0_ps
            + "%%EndProlog\n"
            + "%%BeginSetup\n"
            + GenerateFontHeader(fonts)
            + setup0_ps
            + "%%EndSetup\n";
    }
    return std::string("%%BeginProlog\n")
        + std::format("/whash {} def\n", mMode.HashW())
        + std::format("/ehash {} def\n", mMode.HashE())
        + std::format("/fieldw {:.2f} def\n", width)
        + std::format("/fieldh {:.2f} def\n", height)
        + prolog2_ps
        + "%%EndProlog\n"
        + "%%BeginSetup\n"
        + setup2_ps
        + "%%EndSetup\n";
}

auto PrintShowToPS::GenerateSheets(std::set<size_t> const& isPicked, int numPagesSoFar) const -> std::tuple<std::string, int>
{
    auto result = std::string{};
    for (auto sheet = mShow.GetSheetBegin(); sheet != mShow.GetSheetEnd(); ++sheet) {
        if ((isPicked.count(std::distance(mShow.GetSheetBegin(), sheet)) != 0)) {
            ++numPagesSoFar;
            if (!mOverview) {
                result += GenerateStandard(*sheet, false);
                if (IsSplitSheet(*sheet)) {
                    result += GeneratePageBreak();
                    ++numPagesSoFar;
                    result += GenerateStandard(*sheet, true);
                }
            } else {
                result += GenerateOverview(*sheet);
            }
            result += GeneratePageBreak();
        }
    }
    return { result, numPagesSoFar };
}

auto PrintShowToPS::GenerateContinuitySheets(int numPagesSoFar) const -> std::tuple<std::string, int>
{
    auto lines_left = 0;
    auto need_eject = false;
    auto result = std::string{};
    for (auto sheet = mShow.GetSheetBegin(); sheet != mShow.GetSheetEnd(); ++sheet) {
        auto continuity = sheet->GetPrintableContinuity();
        for (auto& text : continuity) {
            if (!text.on_main) {
                continue;
            }
            if (lines_left <= 0) {
                if (numPagesSoFar > 0) {
                    result += GeneratePageBreak();
                }
                ++numPagesSoFar;
                result += std::format("%%Page: CONT{}\n", numPagesSoFar)
                    + GeneratePageHeader(mPageOffsetX * DPI, (mPaperLength - mPageOffsetY) * DPI - real_height);
                lines_left = (short)(real_height / mTextSize - 0.5);
                result += GeneratePrintContinuityPreamble(real_height - mTextSize,
                    mTextSize,
                    real_width,
                    real_width * 0.5 / 7.5,
                    real_width * 1.5 / 7.5,
                    real_width * 2.0 / 7.5,
                    mTextSize);
            }

            result += PostScript::GenerateContinuityLine(text, PSFONT::NORM, mTextSize);
            lines_left--;
            need_eject = true;
        }
    }
    if (need_eject) {
        result += GeneratePageBreak();
    }
    return { result, numPagesSoFar };
}

auto PrintShowToPS::GenerateContSections(Sheet const& sheet) const -> std::string
{
    const auto& continuity = sheet.GetPrintableContinuity();
    auto cont_len = std::count_if(continuity.begin(), continuity.end(), [](auto&& c) { return c.on_sheet; });
    if (cont_len == 0) {
        return {};
    }

    auto cont_height = field_y - step_size * 10;
    auto this_size = std::min(cont_height / (cont_len + 0.5), mTextSize);
    auto result = GeneratePrintContinuityPreamble(
        cont_height - this_size,
        this_size,
        width,
        width * 0.5 / 7.5,
        width * 1.5 / 7.5,
        width * 2.0 / 7.5,
        this_size);
    return std::accumulate(continuity.begin(), continuity.end(), result, [this_size](auto acc, auto const& text) {
        if (!text.on_sheet) {
            return acc;
        }
        return acc + PostScript::GenerateContinuityLine(text, PSFONT::NORM, this_size);
    });
}

auto PrintShowToPS::IsSplitSheet(Sheet const& sheet) const -> bool
{
    auto fmin = mMode.FieldOffset().x;

    /* find bounds */
    auto [max_s, max_n] = CalculateMaxSouthNorth(sheet, mMode);

    return (CoordUnits2Int((max_n - max_s) + ((max_s + fmin) % Int2CoordUnits(8))) > step_width);
}

auto PrintShowToPS::GenerateStandard(Sheet const& sheet, bool split_sheet) const -> std::string
{
    auto fieldsize = mMode.FieldSize();
    auto fieldoff = mMode.FieldOffset();
    auto secondSplit = IsSplitSheet(sheet);
    auto [clip_s, clip_n, step_offset] = CalculateStandardRange(sheet, fieldsize, step_width, mMode, split_sheet, secondSplit);
    auto dot_w = step_size / 2 * mDotRatio;

    auto result = GeneratePageNameAndNumber(sheet, split_sheet, secondSplit)
        + GenerateStartPage(mPrintLandscape, field_x, field_y, mPageOffsetX, mPaperLength, mPageOffsetY, real_width, real_height)
        + GenerateField(step_offset, step_width, fieldoff, step_size, field_h, mYardsSize, mYardText)
        + std::format("/w {:.4f} def\n", dot_w)
        + std::format("/plinew {:.4f} def\n", dot_w * mPLineRatio)
        + std::format("/slinew {:.4f} def\n", dot_w * mSLineRatio)
        + std::format("/numberfont findfont {:.2f} scalefont setfont\n", dot_w * 2 * mNumRatio);

    auto pointsOfInterest = CalChart::Ranges::enumerate_view(sheet.GetAllPoints())
        | std::views::filter([clip_s, clip_n](auto&& enumPoint) {
              return std::get<1>(enumPoint).GetPos().x >= clip_s && std::get<1>(enumPoint).GetPos().x <= clip_n;
          });
    result = std::accumulate(std::begin(pointsOfInterest), std::end(pointsOfInterest), result, [this, step_offset, fieldheight = CoordUnits2Float(fieldsize.y), fieldoffx = CoordUnits2Float(fieldoff.x), fieldoffy = CoordUnits2Float(fieldoff.y)](auto acc, auto enumPoint) {
        auto [enumeration, point] = enumPoint;
        auto dot_x = (CoordUnits2Float(point.GetPos().x) - fieldoffx - step_offset) / step_width * field_w;
        auto dot_y = (1.0 - (CoordUnits2Float(point.GetPos().y) - fieldoffy) / fieldheight) * field_h;
        return acc + std::format("{:.2f} {:.2f} {}\n", dot_x, dot_y, dot_routines[point.GetSymbol()])
            + std::format("({}) {:.2f} {:.2f} {}\n", mShow.GetPointLabel(enumeration), dot_x, dot_y, (point.GetFlip() ? "donumber2" : "donumber"));
    });
    if (mPrintDoCont) {
        result += std::format("{:.2f} {:.2f} translate\n", -field_x, -field_y)
            + GenerateContSections(sheet);
    }
    return result;
}

auto PrintShowToPS::GenerateOverview(Sheet const& sheet) const -> std::string
{
    auto fieldoff = mMode.FieldOffset();
    auto fieldsize = mMode.FieldSize();
    auto fieldwidth = CoordUnits2Float(fieldsize.x);
    auto result = std::format("%%Page: {}\n", sheet.GetName())
        + GenerateStartPage(mPrintLandscape,
            field_x,
            field_y,
            mPageOffsetX,
            mPaperLength,
            mPageOffsetY,
            real_width,
            real_height)
        + "drawfield\n"
        + std::format("/w {:.2f} def\n", width / fieldwidth * 2.0 / 3.0);

    auto fieldheight = CoordUnits2Float(fieldsize.y);

    auto allBoxes = sheet.GetAllPoints()
        | std::views::transform([](auto&& point) { return point.GetPos(); })
        | std::views::transform([this, fieldoff, fieldwidth, fieldheight](auto&& position) {
              return std::format("{:.2f} {:.2f} dotbox\n",
                  CoordUnits2Float(position.x - fieldoff.x) / fieldwidth * width,
                  (1.0 - CoordUnits2Float(position.y - fieldoff.y) / fieldheight) * height);
          });
    return std::accumulate(allBoxes.begin(), allBoxes.end(), result, std::plus<>{});
}
}
