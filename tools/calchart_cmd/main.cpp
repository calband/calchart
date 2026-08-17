//
//  main.cpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "CalChartMeasure.h"
#include "CalChartPrintShowToPS.hpp"
#include "calchart_cmd_parse.hpp"
#include "calchart_cmd_parse_continuity_text.hpp"
#include "ccvers.h"
#include "docopt.h"

#include <fstream>
#include <iostream>

extern CalChart::MeasureDuration<1024> gAnimateMeasure;

constexpr auto USAGE =
    R"(calchart_cmd

Usage:
    calchart_cmd parse [options] <shows>...
    calchart_cmd print_to_postscript [--landscape --cont --contsheet --overview] <show> <ps_file>
    calchart_cmd parse_continuity_text <text>
    calchart_cmd (-h | --help)
    calchart_cmd --version

Options:
    --print_show            Parse option to print out the show.
    --print_show_out=<out>  Parse option to print out the show to <out>.
    --check_flag            Parse option to perform check and print results.
    --dump_continuity       Parse option to dump the continuity.
    --dump_cont_out=<out>   Parse option to dump the continuity to <out>.
    --dump_print_continuity     Parse option to dump the print continuity.
    --dump_print_cont_out=<out> Parse option to dump the print continuity to <out>.
    --animate_show          Parse option to print the animation.
    --animate_show_out=<out>    Parse option to print the animation to <out>.
    --json                  Parse option to dump the JSON for the viewer.
    --json_out=<out>        Parse option to dump the JSON for the viewer to <out>.
    --showjson              Parse option to dump the Show file format JSON.
    --showjson_out=<out>    Parse option to dump the Show file format JSON to <out>.
    --jsonwidth=<width>     Indention for JSON dumps [default: 4].
    --dump_beats            Parse option to dump downbeat times.
    --dump_beats_out=<out>  Parse option to dump downbeat times to <out>.
    --profile               Print profiling data.
    --landscape_out=<out>   Parse option to dump the postscript landscape version of the show to <out>.
    --cont_out=<out>        Parse option to dump the postscript continuity version of the show to <out>.
    --contsheet_out=<out>   Parse option to dump the postscript continuity on each sheet to <out>.
    --overview_out=<out>    Parse option to dump the postscript overview version of the show to <out>.
    -h, --help              Show this screen.
    --version               Show version.
)";

constexpr auto version = "calchart_cmd " CC_GIT_VERSION;

void PrintToPS(
    std::string_view showPath, bool landscape, bool cont, bool contsheet, bool overview, std::string_view outfile)
{
    auto show = OpenShow(showPath);
    return PrintToPS(*show, landscape, cont, contsheet, overview, outfile);
}

auto main(int argc, char* argv[]) -> int
{
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, { argv + 1, argv + argc }, true, version);

    if (args["parse"].asBool()) {
        CalChartCmd::Parse(args, std::cout);
    }
    if (args["print_to_postscript"].asBool()) {
        PrintToPS(args["<show>"].asString(), args["--landscape"].asBool(), args["--cont"].asBool(),
            args["--contsheet"].asBool(), args["--overview"].asBool(), args["<ps_file>"].asString());
    }
    if (args["parse_continuity_text"].asBool()) {
        ParseContinuityText(args["<text>"].asString(), std::cout);
    }
    if (args["--profile"].asBool()) {
        std::cout << gAnimateMeasure << "\n";
    }

    return 0;
}
