//
//  main.cpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "cc_show.h"
#include "cc_sheet.h"
#include "animate.h"
#include "animatecompile.h"
#include "print_ps.h"
#include "modes.h"
#include "cont.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <getopt.h>

void AnimateShow(const char* show)
{
    std::ifstream input(show);
    std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
    Animation a(*p,
        [](const std::string& notice) { std::cout << notice << "\n"; },
        [](const std::map<AnimateError, ErrorMarker>&, unsigned,
            const std::string& error) {
            std::cout << "error" << error << "\n";
            return true;
        });
}

void PrintShow(const char* show)
{
    std::ifstream input(show);
    std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
    Animation a(*p,
        [](const std::string& notice) { std::cout << notice << "\n"; },
        [](const std::map<AnimateError, ErrorMarker>&, unsigned,
            const std::string& error) {
            std::cout << "error" << error << "\n";
            return true;
        });
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
    std::unique_ptr<const CC_show> p(CC_show::Create_CC_show(input));
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
                auto continuity = Animation::ParseContinuity(cont.GetText(), e, symbol);
				std::cout << "<--Errors during compile-->\n";
                if (e.AnyErrors()) {
					for (auto&& i : e.GetErrors())
					{
						std::cout<<"Error at ["<<i.second.line<<","<<i.second.col<<"] of type "<<i.first<<"\n";
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
	AnimationErrors e;
	auto continuity = Animation::ParseContinuity(text, e, SYMBOL_PLAIN);
	if (e.AnyErrors()) {
		std::cout << "Errors during compile\n";
	}
	for (auto& proc : continuity) {
		std::cout << *proc << "\n";
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
	while (!input.eof())
	{
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

		AnimationErrors e;
		auto continuity = Animation::ParseContinuity(text, e, SYMBOL_PLAIN);
		std::stringstream parsed_continuity;
		for (auto& proc : continuity) {
			parsed_continuity << *proc << "\n";
		}
		std::stringstream parse_errors;
		if (e.AnyErrors()) {
			for (auto&& i : e.GetErrors())
			{
				parse_errors<<"Error at ["<<i.second.line<<","<<i.second.col<<"] of type "<<i.first<<"\n";
			}
		}
		auto parsed_cont = parsed_continuity.str();
		if ((e.AnyErrors() && parse_errors.str() != errors) || !std::equal(parsed.begin(), parsed.end(), parsed_cont.begin(), parsed_cont.end())) {
			std::cout<<"parse failed!\n";
			std::cout<<"Found text: \n"<< text <<"\n";
			std::cout<<"Found parse: \n"<< parsed <<"\n";
			std::cout<<"Parse errors: \n"<< errors <<"\n";
			std::cout<<"parsed_continuity: \n"<< parsed_cont <<"\n";
			std::cout<<"has Parse Errors: \n"<<parse_errors.str()<<"\n";
		}
		else {
			++numTestsPassed;
		}
		++numTestsRun;
	}
	std::cout<<"ContinuityTest "<<test_cases<<" complete.  Passed "<<numTestsPassed<<" out of "<<numTestsRun<<"\n";
}

void PrintToPS(const char* show, bool landscape, bool cont, bool contsheet,
    bool overview, std::string const& outfile)
{
    std::ifstream input(show);
    std::unique_ptr<const CC_show> p(CC_show::Create_CC_show(input));

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
    auto Get_spr_line_text = Get_yard_text;

    auto mode = ShowModeStandard::CreateShowMode(
        "Standard", { { 32, 52, 8, 8, 8, 8, -80, -42, 160, 84 } });

    PrintShowToPS printShowToPS(
        *p, landscape, cont, contsheet, overview, 50, *mode,
        { { head_font_str, main_font_str, number_font_str, cont_font_str,
            bold_font_str, ital_font_str, bold_ital_font_str } },
        PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength, HeaderSize,
        YardsSize, TextSize, DotRatio, NumRatio, PLineRatio, SLineRatio,
        ContRatio, Get_yard_text, Get_spr_line_text);

    std::set<size_t> picked;
    for (auto i = 0; i < p->GetNumSheets(); ++i)
        picked.insert(i);

    printShowToPS(output, false, 0, picked, "show");
}

int IndexForContNames(const std::string& name);

bool ContinuityCountDifferentThanSymbol(const char* show)
{
    std::ifstream input(show);
    if (!input.is_open()) {
        throw std::runtime_error("could not open file");
    }
    std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
    return false;
}

void FindContinuityInconsistancies(const char* show)
{
    std::ifstream input(show);
    if (!input.is_open()) {
        throw std::runtime_error("could not open file");
    }
}

int verbose_flag = 1;
int print_flag = 0;
int animate_flag = 0;
int dump_continuity = 0;
int dump_continuity_text = 0;
int unit_test_continuity = 0;
int check_flag = 0;
int psprint_flag = 0;

static struct option long_options[] = {
    /* These options set a flag. */
    { "print_show", no_argument, &print_flag, 1 }, // p
    { "check_flag", no_argument, &check_flag, 1 }, // c
    { "dump_continuity", no_argument, &dump_continuity, 1 }, // d
    { "dump_continuity_text", no_argument, &dump_continuity_text, 1 }, // D
    { "unit_test_continuity", no_argument, &unit_test_continuity, 1 }, // U
    { "animate_flag", no_argument, &animate_flag, 1 }, // a
    { "psprint_flag", no_argument, &psprint_flag, 1 }, // P
    { 0, 0, 0, 0 }
};

namespace calchart {
int main(int argc, char* argv[]);
}

int main(int argc, char* argv[])
{
    opterr = 0;
    int c = 0;
    while ((c = getopt(argc, argv, "cpadDUP")) != -1)
        switch (c) {
        case 'p':
            print_flag = true;
            break;
        case 'a':
            animate_flag = true;
            break;
        case 'd':
            dump_continuity = true;
            break;
        case 'D':
            dump_continuity_text = true;
            break;
        case 'U':
            unit_test_continuity = true;
            break;
        case 'c':
            check_flag = true;
            break;
        case 'P':
            psprint_flag = true;
            break;
        }
    while (optind < argc) {
        try {
            if (print_flag) {
                PrintShow(argv[optind]);
            }
            if (animate_flag) {
                AnimateShow(argv[optind]);
            }
            if (dump_continuity) {
                DumpContinuity(argv[optind]);
            }
            if (dump_continuity_text) {
                DumpContinuityText(argv[optind]);
            }
            if (unit_test_continuity) {
                DoContinuityUnitTest(argv[optind]);
            }
            if (check_flag) {
                std::cout << "ContinuityCountDifferentThanSymbol ? "
                          << ContinuityCountDifferentThanSymbol(argv[optind]) << "\n";
                FindContinuityInconsistancies(argv[optind]);
            }
            if (psprint_flag) {
                PrintToPS(argv[optind], atoi(argv[optind + 1]), atoi(argv[optind + 2]),
                    atoi(argv[optind + 3]), atoi(argv[optind + 4]),
                    argv[optind + 5]);
                break;
            }
        }
        catch (const std::runtime_error& e) {
            std::cerr << "Error on file " << argv[optind] << ": " << e.what() << "\n";
        }
        ++optind;
    }

    return 0;
}
