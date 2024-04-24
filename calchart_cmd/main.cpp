//
//  main.cpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "calchart_cmd_parse.hpp"
#include "ccvers.h"
#include "docopt.h"
#include "print_ps.h"

#include <fstream>
#include <iostream>

constexpr auto USAGE =
    R"(calchart_cmd

Usage:
    calchart_cmd parse [options] <shows>...
    calchart_cmd print_to_postscript [--landscape --cont --contsheet --overview] <show> <ps_file>
    calchart_cmd (-h | --help)
    calchart_cmd --version

Options:
    --print_show            Parse option to print out the show.
    --check_flag            Parse option to perform check and print results.
    --dump_continuity       Parse option to dump the continuting.
    --dump_continuity_text  Parse option to dump the continuity text.
    --animate_show          Parse option to print the animation.
    --json                  Parse option to dump the JSON for the viewer.
    -h, --help              Show this screen.
    --version               Show version.
)";

constexpr auto version = "calchart_cmd " CC_VERSION;

void PrintToPS(std::string_view showPath, bool landscape, bool cont, bool contsheet, bool overview, std::string_view outfile)
{
    constexpr auto head_font_str = "Palatino-Bold";
    constexpr auto main_font_str = "Helvetica";
    constexpr auto number_font_str = "Helvetica-Bold";
    constexpr auto cont_font_str = "Courier";
    constexpr auto bold_font_str = "Courier-Bold";
    constexpr auto ital_font_str = "Courier-Italic";
    constexpr auto bold_ital_font_str = "Courier-BoldItalic";

    constexpr auto PageWidth = 7.5;
    constexpr auto PageHeight = 10.0;
    constexpr auto PageOffsetX = 0.5;
    constexpr auto PageOffsetY = 0.5;
    constexpr auto PaperLength = 11.0;

    constexpr auto HeaderSize = 3.0;
    constexpr auto YardsSize = 1.5;
    constexpr auto TextSize = 10.0;
    constexpr auto DotRatio = 0.9;
    constexpr auto NumRatio = 1.35;
    constexpr auto PLineRatio = 1.2;
    constexpr auto SLineRatio = 1.2;
    constexpr auto ContRatio = 0.2;

    auto mode = CalChart::ShowMode::GetDefaultShowMode();

    auto show = OpenShow(showPath);

    auto printShowToPS = CalChart::PrintShowToPS(
        *show, landscape, cont, contsheet, overview, 50, mode,
        { { head_font_str, main_font_str, number_font_str, cont_font_str,
            bold_font_str, ital_font_str, bold_ital_font_str } },
        PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength, HeaderSize,
        YardsSize, TextSize, DotRatio, NumRatio, PLineRatio, SLineRatio,
        ContRatio, CalChart::kDefaultYardLines);

    auto picked = std::set<size_t>{};
    for (auto i = 0; i < show->GetNumSheets(); ++i) {
        picked.insert(i);
    }

    auto output = std::ofstream(std::string(outfile));

    printShowToPS(output, picked, "show");
}

auto main(int argc, char* argv[]) -> int
{
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, version);

    if (args["parse"].asBool()) {
        CalChartCmd::Parse(args, std::cout);
    }
    if (args["print_to_postscript"].asBool()) {
        PrintToPS(args["<show>"].asString(), args["--landscape"].asBool(), args["--cont"].asBool(), args["--contsheet"].asBool(), args["--overview"].asBool(), args["<ps_file>"].asString());
    }

    return 0;
}
