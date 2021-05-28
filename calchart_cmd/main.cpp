//
//  main.cpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "CalChartShow.h"
#include "CalChartSheet.h"
#include "CalChartAnimation.h"
#include "CalChartAnimationCompile.h"
#include "CalChartAnimationErrors.h"
#include "print_ps.h"
#include "modes.h"
#include "cont.h"
#include "docopt.h"
#include "cc_continuity.h"

#include <iostream>
#include <fstream>
#include <iterator>

static const char USAGE[] =
R"(calchart_cmd

    Usage:
      calchart_cmd parse [-acdpDU] <shows>...
      calchart_cmd [--landscape --cont --contsheet --overview] print_to_postscript <show> <ps_file>
      calchart_cmd (-h | --help)
      calchart_cmd --version

    Options:
      -p, --print_show    Print out the show.
      -c, --check_flag  Drifting mine.
      -d, --dump_continuity  Drifting mine.
      -D, --dump_continuity_text  Drifting mine.
      -a, --animate_show  Drifting mine.
      -U, --unit_test_continuity  Drifting mine.
      -h --help     Show this screen.
      --version     Show version.
)";

static const auto version = "calchart_cmd 1.0";

using namespace CalChart;

void AnimateShow(const char* show)
{
    std::ifstream input(show);
    std::unique_ptr<Show> p(Show::Create(ShowMode::GetDefaultShowMode(), input));
    Animation a(*p);
    for (auto&& errors : a.GetAnimationErrors()) {
        for (auto&& [key, value] : errors.GetErrors()) {
            std::cout << "error " << key <<"\n";
        };
    };
}

void PrintShow(const char* show)
{
    std::ifstream input(show);
    std::unique_ptr<Show> p(Show::Create(ShowMode::GetDefaultShowMode(), input));
    Animation a(*p);
    for (auto&& errors : a.GetAnimationErrors()) {
        for (auto&& [key, value] : errors.GetErrors()) {
            std::cout << "error " << key <<"\n";
        };
    };
    a.GotoSheet(0);
    auto currentInfo = a.GetCurrentInfo();
    std::cout << currentInfo.first << "\n";
    std::copy(currentInfo.second.begin(), currentInfo.second.end(),
        std::ostream_iterator<std::string>(std::cout, "\n"));
    auto oldInfo = currentInfo;
    while (a.NextBeat()) {
        auto currentInfo = a.GetCurrentInfo();
        if (currentInfo.first != oldInfo.first) {
            std::cout << currentInfo.first << "\n";
        }
        for (auto i = 0ul; i < currentInfo.second.size(); ++i) {
            if (i < oldInfo.second.size() && oldInfo.second.at(i) == currentInfo.second.at(i)) {
                continue;
            }
            std::cout << currentInfo.second.at(i) << "\n";
        }
        oldInfo = currentInfo;
    }
}

void DumpContinuity(const char* show)
{
    std::ifstream input(show);
    std::unique_ptr<const Show> p(Show::Create(ShowMode::GetDefaultShowMode(), input));
    auto sheet_num = 0;
    for (auto i = p->GetSheetBegin(); i != p->GetSheetEnd(); ++i, ++sheet_num) {
        static const SYMBOL_TYPE k_symbols[] = {
            SYMBOL_PLAIN, SYMBOL_SOL, SYMBOL_BKSL, SYMBOL_SL,
            SYMBOL_X, SYMBOL_SOLBKSL, SYMBOL_SOLSL, SYMBOL_SOLX
        };
        for (auto& symbol : k_symbols) {
            if (i->ContinuityInUse(symbol)) {
                auto& cont = i->GetContinuityBySymbol(symbol);
                std::cout << "<--StartText sheet num " << sheet_num << ": symbol " << GetNameForSymbol(symbol) << "-->\n";
                std::cout << cont.GetText() << "\n";
                std::cout << "<--EndText sheet num " << sheet_num << ": symbol " << GetNameForSymbol(symbol) << "-->\n";

                AnimationErrors e;
                auto&& continuity = cont.GetParsedContinuity();
                std::cout << "<--Errors during compile-->\n";
                if (e.AnyErrors()) {
                    for (auto&& i : e.GetErrors()) {
                        std::cout << "Error at [" << i.second.line << "," << i.second.col << "] of type " << i.first << "\n";
                    }
                }
                std::cout << "<--End errors-->\n";
                std::cout << "<--StartParsed-->\n";
                for (auto& proc : continuity) {
                    std::cout << *proc << "\n";
                }
                std::cout << "<--EndParsed-->\n";
            }
        }
    }
}

void DumpContinuityText(std::string const& text)
{
    try {
        auto&& continuity = Continuity(text);
        for (auto& proc : continuity.GetParsedContinuity()) {
            std::cout << *proc << "\n";
        }
    }
    catch (ParseError const& error) {
        AnimationErrors e;
        // Supply a generic parse error
        e.RegisterError(ANIMERR_SYNTAX, error.line, error.column, 0, SYMBOL_PLAIN);
        if (e.AnyErrors()) {
            std::cout << "Errors during compile: " << error.what() << "\n";
        }
    }
}

void DoContinuityUnitTest(const char* test_cases)
{
    static const std::string BeginText = "<--StartText";
    static const std::string EndText = "<--EndText";
    static const std::string BeginParsed = "<--StartParsed-->";
    static const std::string EndParsed = "<--EndParsed-->";
    static const std::string BeginErrors = "<--Errors during compile-->";
    static const std::string EndErrors = "<--End errors-->";

    size_t numTestsRun = 0, numTestsPassed = 0;
    std::ifstream input(test_cases);
    while (!input.eof()) {
        std::string d;
        do {
            getline(input, d);
        } while (!input.eof() && (d.size() < BeginText.size() || !std::equal(BeginText.begin(), BeginText.end(), d.begin())));
        std::string text;
        getline(input, d);
        bool firsttime = true;
        while (!input.eof() && (d.size() < EndText.size() || !std::equal(EndText.begin(), EndText.end(), d.begin()))) {
            if (!firsttime) {
                text += "\n";
            }
            firsttime = false;
            text += d;
            getline(input, d);
        }
        do {
            getline(input, d);
        } while (!input.eof() && (d.size() < BeginErrors.size() || !std::equal(BeginErrors.begin(), BeginErrors.end(), d.begin())));
        std::string errors;
        getline(input, d);
        while (!input.eof() && (d.size() < EndErrors.size() || !std::equal(EndErrors.begin(), EndErrors.end(), d.begin()))) {
            errors += d + "\n";
            getline(input, d);
        }

        do {
            getline(input, d);
        } while (!input.eof() && (d.size() < BeginParsed.size() || !std::equal(BeginParsed.begin(), BeginParsed.end(), d.begin())));
        std::string parsed;
        getline(input, d);
        while (!input.eof() && (d.size() < EndParsed.size() || !std::equal(EndParsed.begin(), EndParsed.end(), d.begin()))) {
            parsed += d + "\n";
            getline(input, d);
        }

        std::stringstream parsed_continuity;
        std::stringstream parse_errors;
        AnimationErrors e;
        try {
            auto&& continuity = Continuity(text);
            for (auto& proc : continuity.GetParsedContinuity()) {
                parsed_continuity << *proc << "\n";
            }
        }
        catch (ParseError const& error) {
            // Supply a generic parse error
            e.RegisterError(ANIMERR_SYNTAX, error.line, error.column, 0, SYMBOL_PLAIN);
            if (e.AnyErrors()) {
                for (auto&& i : e.GetErrors()) {
                    parse_errors << "Error at [" << i.second.line << "," << i.second.col << "] of type " << i.first << "\n";
                }
            }
        }
        auto parsed_cont = parsed_continuity.str();
        if ((e.AnyErrors() && parse_errors.str() != errors) || !std::equal(parsed.begin(), parsed.end(), parsed_cont.begin(), parsed_cont.end())) {
            std::cout << "parse failed!\n";
            std::cout << "Found text: \n" << text << "\n";
            std::cout << "Found parse: \n" << parsed << "\n";
            std::cout << "Parse errors: \n" << errors << "\n";
            std::cout << "parsed_continuity: \n" << parsed_cont << "\n";
            std::cout << "has Parse Errors: \n" << parse_errors.str() << "\n";
        }
        else {
            ++numTestsPassed;
        }
        ++numTestsRun;
    }
    std::cout << "ContinuityTest " << test_cases << " complete.  Passed " << numTestsPassed << " out of " << numTestsRun << "\n";
}

void PrintToPS(const char* show, bool landscape, bool cont, bool contsheet,
    bool overview, std::string const& outfile)
{
    std::ifstream input(show);
    std::unique_ptr<const Show> p(Show::Create(ShowMode::GetDefaultShowMode(), input));

    std::ofstream output(outfile);

    std::string head_font_str = "Palatino-Bold";
    std::string main_font_str = "Helvetica";
    std::string number_font_str = "Helvetica-Bold";
    std::string cont_font_str = "Courier";
    std::string bold_font_str = "Courier-Bold";
    std::string ital_font_str = "Courier-Italic";
    std::string bold_ital_font_str = "Courier-BoldItalic";

    double PageWidth = 7.5;
    double PageHeight = 10.0;
    double PageOffsetX = 0.5;
    double PageOffsetY = 0.5;
    double PaperLength = 11.0;

    double HeaderSize = 3.0;
    double YardsSize = 1.5;
    double TextSize = 10.0;
    double DotRatio = 0.9;
    double NumRatio = 1.35;
    double PLineRatio = 1.2;
    double SLineRatio = 1.2;
    double ContRatio = 0.2;

    auto Get_yard_text = [](size_t offset) {
        static const std::string yard_text[] = {
            "N", "M", "L", "K", "J", "I", "H", "G", "F", "E", "D",
            "C", "B", "A", "-10", "-5", "0", "5", "10", "15", "20", "25",
            "30", "35", "40", "45", "50", "45", "40", "35", "30", "25", "20",
            "15", "10", "5", "0", "-5", "-10", "A", "B", "C", "D", "E",
            "F", "G", "H", "I", "J", "K", "L", "M", "N"
        };
        return yard_text[offset];
    };

    auto mode = ShowMode::GetDefaultShowMode();

    PrintShowToPS printShowToPS(
        *p, landscape, cont, contsheet, overview, 50, mode,
        { { head_font_str, main_font_str, number_font_str, cont_font_str,
            bold_font_str, ital_font_str, bold_ital_font_str } },
        PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength, HeaderSize,
        YardsSize, TextSize, DotRatio, NumRatio, PLineRatio, SLineRatio,
        ContRatio, Get_yard_text);

    std::set<size_t> picked;
    for (auto i = 0; i < p->GetNumSheets(); ++i)
        picked.insert(i);

    printShowToPS(output, picked, "show");
}

int IndexForContNames(const std::string& name);

bool ContinuityCountDifferentThanSymbol(const char* show)
{
    std::ifstream input(show);
    if (!input.is_open()) {
        throw std::runtime_error("could not open file");
    }
    std::unique_ptr<Show> p(Show::Create(ShowMode::GetDefaultShowMode(), input));
    return false;
}

void FindContinuityInconsistancies(const char* show)
{
    std::ifstream input(show);
    if (!input.is_open()) {
        throw std::runtime_error("could not open file");
    }
}

namespace calchart {
int main(int argc, char* argv[]);
}

int main(int argc, char* argv[])
{
    std::map<std::string, docopt::value> args
        = docopt::docopt(USAGE,
                         { argv + 1, argv + argc },
                         true,               // show help if requested
                         version);  // version string

    int print_flag = args["--print_show"].asBool();
    int animate_flag = args["--animate_show"].asBool();
    int dump_continuity = args["--dump_continuity"].asBool();
    int dump_continuity_text = args["--dump_continuity_text"].asBool();
    int unit_test_continuity = args["--unit_test_continuity"].asBool();
    int check_flag = args["--check_flag"].asBool();

    if (args["parse"].asBool()) {
        auto list_of_files = args["<shows>"].asStringList();
        for (auto file : list_of_files) {
            if (print_flag) {
                PrintShow(file.c_str());
            }
            if (animate_flag) {
                AnimateShow(file.c_str());
            }
            if (dump_continuity) {
                DumpContinuity(file.c_str());
            }
            if (dump_continuity_text) {
                DumpContinuityText(file.c_str());
            }
            if (unit_test_continuity) {
                DoContinuityUnitTest(file.c_str());
            }
            if (check_flag) {
                std::cout << "ContinuityCountDifferentThanSymbol ? "
                          << ContinuityCountDifferentThanSymbol(file.c_str()) << "\n";
                FindContinuityInconsistancies(file.c_str());
            }
        }
    }
    if (args["print_to_postscript"].asBool()) {
        PrintToPS(args["<show>"].asString().c_str(), args["--landscape"].asBool(), args["--cont"].asBool(), args["--contsheet"].asBool(), args["--overview"].asBool(), args["<ps_file>"].asString().c_str());
    }

    return 0;
}
