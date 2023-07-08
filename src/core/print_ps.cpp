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
#include "print_ps.h"

#include "CalChartPostScript.h"
#include "CalChartSheet.h"
#include "CalChartShow.h"
#include "CalChartShowMode.h"

#include "prolog0.h"
#include "prolog2.h"
#include "setup0.h"
#include "setup2.h"

#include <sstream>
#include <string>
#include <time.h>

namespace CalChart {

// from
// http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
// there is no snprint on visual c++ 2010
#if _MSC_VER
#define snprintf _snprintf
#endif

#define DPI 72

static const char* dot_routines[] = {
    "dotplain",
    "dotsolid",
    "dotbs",
    "dotsl",
    "dotx",
    "dotsolbs",
    "dotsolsl",
    "dotsolx",
};

template <typename IOS>
class RAII_setprecision {
public:
    RAII_setprecision(IOS& s)
        : os(s)
        , precision(s.precision())
    {
    }
    ~RAII_setprecision() { os.precision(precision); }

private:
    IOS& os;
    std::streamsize precision;
};

static std::tuple<float, float, float, float, float>
CalcValues(bool PrintLandscape, bool PrintDoCont, double PageWidth,
    double PageHeight, double ContRatio)
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
    const Show& show, bool PrintLandscape, bool PrintDoCont,
    bool PrintDoContSheet, bool PrintOverview, int min_yards,
    ShowMode const& mode, std::array<std::string, 7> const& fonts_,
    double PageWidth, double PageHeight, double PageOffsetX, double PageOffsetY,
    double PaperLength, double HeaderSize, double YardsSize, double TextSize,
    double DotRatio, double NumRatio, double PLineRatio, double SLineRatio,
    double ContRatio, ShowMode::YardLinesInfo_t yardText)
    : mShow(show)
    , mPrintLandscape(PrintLandscape)
    , mPrintDoCont(PrintDoCont)
    , mPrintDoContSheet(PrintDoContSheet)
    , mOverview(PrintOverview)
    , mMode(mode)
    , fonts(fonts_)
    , mPageWidth(PageWidth)
    , mPageHeight(PageHeight)
    , mPageOffsetX(PageOffsetX)
    , mPageOffsetY(PageOffsetY)
    , mPaperLength(PaperLength)
    , mHeaderSize(HeaderSize)
    , mYardsSize(YardsSize)
    , mTextSize(TextSize)
    , mDotRatio(DotRatio)
    , mNumRatio(NumRatio)
    , mPLineRatio(PLineRatio)
    , mSLineRatio(SLineRatio)
    , mContRatio(ContRatio)
    , mYardText(std::move(yardText))
{
    std::tie(width, height, real_width, real_height, field_x, field_y, field_w,
        field_h,
        step_size, step_width)
        = CalcAllValues(mPrintLandscape, mPrintDoCont, mOverview, mPageWidth,
            mPageHeight, mContRatio, min_yards, mMode);
}

int PrintShowToPS::operator()(std::ostream& buffer,
    const std::set<size_t>& isPicked,
    std::string const& title) const
{
    RAII_setprecision<std::ostream> tmp_(buffer);
    buffer << std::fixed;
    buffer.precision(2);

    /* Now write postscript header */
    PrintHeader(buffer, title);

    PrintFieldDefinition(buffer);

    short num_pages = 0;
    /* print continuity sheets first */
    if (mPrintDoContSheet && !mOverview) {
        num_pages = PrintContinuitySheets(buffer, num_pages);
    }

    /* do stuntsheet pages now */
    num_pages = PrintSheets(buffer, isPicked, num_pages);
    /* finally, write trailer */
    PrintTrailer(buffer, num_pages);

    return num_pages;
}

void PrintShowToPS::PrintHeader(std::ostream& buffer, const std::string& title) const
{
    buffer << "%!PS-Adobe-3.0\n";

    {
        RAII_setprecision<std::ostream> tmp_(buffer);
        buffer.precision(0);
        buffer << "%%BoundingBox: " << (mPageOffsetX * DPI) << " "
               << ((mPaperLength - mPageOffsetY) * DPI - real_height) << " "
               << (mPageOffsetX * DPI + real_width) << " "
               << ((mPaperLength - mPageOffsetY) * DPI) << "\n";
    }
    time_t t;
    time(&t);
    buffer << "%%CreationDate: " << ctime(&t);
    buffer << "%%Title: " << title << "\n";
    buffer << "%%Creator: CalChart\n";
    buffer << "%%Pages: (atend)\n";
    buffer << "%%PageOrder: Ascend\n";
    if (!mOverview) {
        buffer << "%%DocumentNeededResources: font";
        for (auto& i : fonts) {
            buffer << " " << i;
        }
        buffer << "\n";
        buffer << "%%DocumentSuppliedResources: font CalChart\n";
        buffer << "%%BeginDefaults\n";
        buffer << "%%PageResources: font";
        for (auto& i : fonts) {
            buffer << " " << i;
        }
        buffer << " CalChart\n";
        buffer << "%%EndDefaults\n";
    }
    buffer << "%%EndComments\n";
}

void PrintShowToPS::PrintTrailer(std::ostream& buffer, short num_pages) const
{
    buffer << "%%Trailer\n";
    buffer << "%%Pages: " << num_pages << "\n";
    buffer << "%%EOF\n";
}

void PrintShowToPS::PrintFieldDefinition(std::ostream& buffer) const
{
    if (!mOverview) {
        buffer << "%%BeginProlog\n";
        buffer << "/fieldw " << field_w << " def\n";
        buffer << "/fieldh " << field_h << " def\n";
        buffer << "/fieldy " << field_y << " def\n";
        buffer << "/stepw " << step_width << " def\n";
        buffer << "/whash " << mMode.HashW() << " def\n";
        buffer << "/ehash " << mMode.HashE() << " def\n";
        buffer << "/headsize " << mHeaderSize << " def\n";
        buffer << "/yardsize " << mYardsSize << " def\n";
        buffer << prolog0_ps;
        buffer << "%%EndProlog\n";
        buffer << "%%BeginSetup\n";
        PostScript::PrintFontHeader(buffer, fonts);
        buffer << setup0_ps;
        buffer << "%%EndSetup\n";
    } else {
        buffer << "%%BeginProlog\n";
        buffer << "/whash " << mMode.HashW() << " def\n";
        buffer << "/ehash " << mMode.HashE() << " def\n";
        buffer << "/fieldw " << width << " def\n";
        buffer << "/fieldh " << height << " def\n";
        buffer << prolog2_ps;
        buffer << "%%EndProlog\n";
        buffer << "%%BeginSetup\n";
        buffer << setup2_ps;
        buffer << "%%EndSetup\n";
    }
}

short PrintShowToPS::PrintSheets(std::ostream& buffer,
    const std::set<size_t>& isPicked,
    short num_pages) const
{
    auto curr_sheet = mShow.GetSheetBegin();
    while (curr_sheet != mShow.GetSheetEnd()) {
        if ((isPicked.count(std::distance(mShow.GetSheetBegin(), curr_sheet)) != 0)) {
            num_pages++;
            if (!mOverview) {
                PrintStandard(buffer, *curr_sheet, false);
                if (SplitSheet(*curr_sheet)) {
                    PostScript::PageBreak(buffer);
                    num_pages++;
                    PrintStandard(buffer, *curr_sheet, true);
                }
            } else {
                PrintOverview(buffer, *curr_sheet);
            }
            PostScript::PageBreak(buffer);
        }
        ++curr_sheet;
    }
    return num_pages;
}
short PrintShowToPS::PrintContinuitySheets(std::ostream& buffer,
    short num_pages) const
{
    short lines_left = 0;
    bool need_eject = false;
    for (auto sheet = mShow.GetSheetBegin(); sheet != mShow.GetSheetEnd();
         ++sheet) {
        auto continuity = sheet->GetPrintableContinuity();
        for (auto& text : continuity) {
            if (!text.on_main)
                continue;
            if (lines_left <= 0) {
                if (num_pages > 0) {
                    PostScript::PageBreak(buffer);
                }
                num_pages++;
                buffer << "%%Page: CONT" << num_pages << "\n";
                PostScript::PageHeader(buffer, mPageOffsetX * DPI,
                    (mPaperLength - mPageOffsetY) * DPI - real_height);
                lines_left = (short)(real_height / mTextSize - 0.5);
                PostScript::PrintContinuityPreamble(buffer, real_height - mTextSize, mTextSize,
                    real_width, real_width * 0.5 / 7.5,
                    real_width * 1.5 / 7.5, real_width * 2.0 / 7.5,
                    mTextSize);
            }

            PostScript::GenerateContinuityLine(buffer, text, PSFONT::NORM, mTextSize);
            lines_left--;
            need_eject = true;
        }
    }
    if (need_eject) {
        PostScript::PageBreak(buffer);
    }
    return num_pages;
}

void PrintShowToPS::PrintContSections(std::ostream& buffer,
    const Sheet& sheet) const
{
    const auto& continuity = sheet.GetPrintableContinuity();
    auto cont_len = std::count_if(continuity.begin(), continuity.end(),
        [](auto&& c) { return c.on_sheet; });
    if (cont_len == 0)
        return;

    float cont_height = field_y - step_size * 10;
    float this_size = cont_height / (cont_len + 0.5);
    if (this_size > mTextSize)
        this_size = mTextSize;
    PostScript::PrintContinuityPreamble(buffer, cont_height - this_size, this_size, width,
        width * 0.5 / 7.5, width * 1.5 / 7.5,
        width * 2.0 / 7.5, this_size);
    for (auto& text : continuity) {
        if (!text.on_sheet)
            continue;
        PostScript::GenerateContinuityLine(buffer, text, PSFONT::NORM, this_size);
    }
}

void PrintShowToPS::print_start_page(std::ostream& buffer, bool landscape,
    double translate_x,
    double translate_y) const
{
    PostScript::PageHeader(buffer, mPageOffsetX * DPI,
        (mPaperLength - mPageOffsetY) * DPI - real_height);
    if (landscape) {
        buffer << real_width << " 0 translate 90 rotate\n";
    }
    buffer << (translate_x) << " " << (translate_y) << " translate\n";
}

bool PrintShowToPS::SplitSheet(const Sheet& sheet) const
{
    auto fmin = mMode.FieldOffset().x;
    auto fmax = mMode.FieldSize().x + fmin;

    /* find bounds */
    auto max_s = fmax;
    auto max_n = fmin;
    for (auto i = 0; i < mShow.GetNumPoints(); i++) {
        if (sheet.GetPoint(i).GetPos().x < max_s)
            max_s = sheet.GetPoint(i).GetPos().x;
        if (sheet.GetPoint(i).GetPos().x > max_n)
            max_n = sheet.GetPoint(i).GetPos().x;
    }
    /* make sure bounds are on field */
    if (max_s < fmin)
        max_s = fmin;
    if (max_n > fmax)
        max_n = fmax;

    return (CoordUnits2Int((max_n - max_s) + ((max_s + fmin) % Int2CoordUnits(8))) > step_width);
}

void PrintShowToPS::PrintStandard(std::ostream& buffer, const Sheet& sheet,
    bool split_sheet) const
{
    RAII_setprecision<std::ostream> tmp_(buffer);
    buffer.precision(2);
    auto fieldsize = mMode.FieldSize();
    auto fieldoff = mMode.FieldOffset();
    auto pmin = mMode.MinPosition().x;
    auto pmax = mMode.MaxPosition().x;
    auto fmin = mMode.FieldOffset().x;
    auto fmax = mMode.FieldSize().x + fmin;

    std::string namestr(sheet.GetName());
    std::string numberstr(sheet.GetNumber());

    Coord::units clip_s, clip_n;
    short step_offset;
    if (split_sheet) {
        buffer << "%%Page: " << namestr << "(N)\n";
        if (!sheet.GetNumber().empty()) {
            buffer << "/pagenumtext (" << numberstr << "N) def\n";
        } else {
            buffer << "/pagenumtext () def\n";
        }
        step_offset = CoordUnits2Int(fieldsize.x) - step_width;
        /* south yardline */
        clip_s = Int2CoordUnits(step_offset) + fmin;
        clip_n = pmax;
    } else {
        /* find bounds */
        auto max_s = fmax;
        auto max_n = fmin;
        for (auto i = 0; i < mShow.GetNumPoints(); i++) {
            if (sheet.GetPoint(i).GetPos().x < max_s)
                max_s = sheet.GetPoint(i).GetPos().x;
            if (sheet.GetPoint(i).GetPos().x > max_n)
                max_n = sheet.GetPoint(i).GetPos().x;
        }
        /* make sure bounds are on field */
        if (max_s < fmin)
            max_s = fmin;
        if (max_n > fmax)
            max_n = fmax;

        if (SplitSheet(sheet)) {
            /* Need to split into two pages */
            buffer << "%%Page: " << namestr << "(S)\n";
            if (!sheet.GetNumber().empty()) {
                buffer << "/pagenumtext (" << numberstr << "S) def\n";
            } else {
                buffer << "/pagenumtext () def\n";
            }
            step_offset = 0;
            clip_s = pmin;
            clip_n = Int2CoordUnits(step_width) + fmin; /* north yardline */
        } else {
            buffer << "%%Page: " << namestr << "\n";
            if (!sheet.GetNumber().empty()) {
                buffer << "/pagenumtext (" << numberstr << ") def\n";
            } else {
                buffer << "/pagenumtext () def\n";
            }
            step_offset = (CoordUnits2Int(mMode.FieldSize().x) - step_width) / 2;
            step_offset = (step_offset / 8) * 8;
            clip_s = pmin;
            clip_n = pmax;
            short x_s = CoordUnits2Int(max_s) - CoordUnits2Int(fmin);
            short x_n = CoordUnits2Int(max_n) - CoordUnits2Int(fmin);
            if ((x_s < step_offset) || (x_n > (step_offset + step_width))) {
                /* Recenter formation */
                step_offset = x_s - (step_width - (x_n - x_s)) / 2;
                if (step_offset < 0)
                    step_offset = 0;
                else if ((step_offset + step_width) > CoordUnits2Int(mMode.FieldSize().x))
                    step_offset = CoordUnits2Int(mMode.FieldSize().x) - step_width;
                step_offset = (step_offset / 8) * 8;
            }
        }
    }
    print_start_page(buffer, mPrintLandscape, field_x, field_y);

    /* Draw field */
    buffer << "drawfield\n";
    buffer << "/mainfont findfont " << (step_size * mYardsSize)
           << " scalefont setfont\n";

    for (short j = 0; j <= step_width; j += 8) {
        buffer << "/lmargin " << (step_size * j) << " def /rmargin "
               << (step_size * j) << " def\n";
        buffer << "/y " << (field_h + (step_size / 2)) << " def\n";
        std::string yardstr(mYardText.at(
            (step_offset + (kYardTextValues - 1) * 4 + CoordUnits2Int(fieldoff.x) + j) / 8));
        buffer << "(" << yardstr << ") dup centerText\n";
        buffer << "/y " << (-(step_size * 2)) << " def\n";
        buffer << "centerText\n";
    }

    float dot_w = step_size / 2 * mDotRatio;
    {
        RAII_setprecision<std::ostream> tmp_(buffer);
        buffer.precision(4);
        buffer << "/w " << dot_w << " def\n";
        buffer << "/plinew " << (dot_w * mPLineRatio) << " def\n";
        buffer << "/slinew " << (dot_w * mSLineRatio) << " def\n";
    }
    buffer << "/numberfont findfont " << (dot_w * 2 * mNumRatio)
           << " scalefont setfont\n";
    for (auto i = 0; i < mShow.GetNumPoints(); i++) {
        if ((sheet.GetPoint(i).GetPos().x > clip_n) || (sheet.GetPoint(i).GetPos().x < clip_s))
            continue;
        auto fieldheight = CoordUnits2Float(fieldsize.y);
        auto fieldoffx = CoordUnits2Float(fieldoff.x);
        auto fieldoffy = CoordUnits2Float(fieldoff.y);
        auto dot_x = (CoordUnits2Float(sheet.GetPoint(i).GetPos().x) - fieldoffx - step_offset) / step_width * field_w;
        auto dot_y = (1.0 - (CoordUnits2Float(sheet.GetPoint(i).GetPos().y) - fieldoffy) / fieldheight) * field_h;
        buffer << dot_x << " " << dot_y << " "
               << dot_routines[sheet.GetSymbol(i)] << "\n";
        buffer << "(" << mShow.GetPointLabel(i) << ") " << dot_x << " " << dot_y
               << " " << (sheet.GetPoint(i).GetFlip() ? "donumber2" : "donumber")
               << "\n";
    }
    if (mPrintDoCont) {
        buffer << (-field_x) << " " << (-field_y) << " translate\n";
        PrintContSections(buffer, sheet);
    }
}

void PrintShowToPS::PrintOverview(std::ostream& buffer,
    const Sheet& sheet) const
{
    buffer << "%%Page: " << sheet.GetName() << "\n";

    print_start_page(buffer, mPrintLandscape, field_x, field_y);

    buffer << "drawfield\n";
    auto fieldoff = mMode.FieldOffset();
    auto fieldsize = mMode.FieldSize();
    auto fieldwidth = CoordUnits2Float(fieldsize.x);
    buffer << "/w " << (width / fieldwidth * 2.0 / 3.0) << " def\n";

    auto fieldx = CoordUnits2Float(fieldoff.x);
    auto fieldy = CoordUnits2Float(fieldoff.y);
    auto fieldheight = CoordUnits2Float(fieldsize.y);

    for (auto i = 0; i < mShow.GetNumPoints(); i++) {
        buffer << ((CoordUnits2Float(sheet.GetPoint(i).GetPos().x) - fieldx) / fieldwidth * width)
               << " " << ((1.0 - (CoordUnits2Float(sheet.GetPoint(i).GetPos().y) - fieldy) / fieldheight) * height)
               << " dotbox\n";
    }
}
}
