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
#include "print_ps.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <unistd.h>
#include <getopt.h>


void AnimateShow(const char* show)
{
	std::ifstream input(show);
	std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
	Animation a(*p, [](const std::string& notice) { std::cout<<notice<<"\n"; }, [](const std::vector<ErrorMarker>&, unsigned, const std::string& error) { std::cout<<"error"<<error<<"\n"; return true; });
}


void PrintShow(const char* show)
{
	std::ifstream input(show);
	std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
	Animation a(*p, [](const std::string& notice) { std::cout<<notice<<"\n"; }, [](const std::vector<ErrorMarker>&, unsigned, const std::string& error) { std::cout<<"error"<<error<<"\n"; return true; });
	a.GotoSheet(0);
	auto currentInfo = a.GetCurrentInfo();
	std::cout<<currentInfo.first<<"\n";
	std::copy(currentInfo.second.begin(), currentInfo.second.end(), std::ostream_iterator<std::string>(std::cout, "\n"));
	auto oldInfo = currentInfo;
	while (a.NextBeat())
	{
		auto currentInfo = a.GetCurrentInfo();
		if (currentInfo.first != oldInfo.first)
		{
			std::cout<<currentInfo.first<<"\n";
		}
		for (auto i = 0; i < currentInfo.second.size(); ++i)
		{
			if (i < oldInfo.second.size() && oldInfo.second.at(i) == currentInfo.second.at(i))
			{
				continue;
			}
			std::cout<<currentInfo.second.at(i)<<"\n";
		}
		oldInfo = currentInfo;
	}
}

void DumpContinuity(const char* show)
{
	std::ifstream input(show);
	std::unique_ptr<const CC_show> p(CC_show::Create_CC_show(input));
	auto sheet_num = 0;
	for (auto i = p->GetSheetBegin(); i != p->GetSheetEnd(); ++i, ++sheet_num)
	{
		static const SYMBOL_TYPE k_symbols[] = {
			SYMBOL_PLAIN, SYMBOL_SOL, SYMBOL_BKSL, SYMBOL_SL,
			SYMBOL_X, SYMBOL_SOLBKSL, SYMBOL_SOLSL, SYMBOL_SOLX
		};
		for (auto& symbol : k_symbols)
		{
			if (i->ContinuityInUse(symbol))
			{
				auto& cont = i->GetContinuityBySymbol(symbol);
				std::cout<<"sheet num "<<sheet_num<<": symbol "<<GetNameForSymbol(symbol)<<":\n";
				std::cout<<cont.GetText();
			}
		}
	}
}

void PrintToPS(const char* show)
{
	std::ifstream input(show);
	std::unique_ptr<const CC_show> p(CC_show::Create_CC_show(input));

	std::ostringstream buffer;
	bool doLandscape = 0;
	bool doCont = 0;
	bool doContSheet = 0;
	
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
			"N", "M", "L", "K", "J", "I", "H", "G", "F", "E", "D", "C", "B", "A",
			"-10", "-5", "0", "5", "10", "15", "20", "25", "30", "35", "40", "45", "50",
			"45", "40", "35", "30", "25", "20", "15", "10", "5", "0", "-5", "-10",
			"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N"
		};
		return yard_text[offset];
	};
	auto Get_spr_line_text = Get_yard_text;

	PrintShowToPS printShowToPS(*p, doLandscape, doCont, doContSheet, head_font_str, main_font_str, number_font_str, cont_font_str, bold_font_str, ital_font_str, bold_ital_font_str, PageWidth, PageHeight, PageOffsetX, PageOffsetY, PaperLength, HeaderSize, YardsSize, TextSize, DotRatio, NumRatio, PLineRatio, SLineRatio, ContRatio, Get_yard_text, Get_spr_line_text);
//	PrintShowToPS printShowToPS(*p, config, doLandscape, doCont, doContSheet);
//	int n = printShowToPS(buffer, eps, overview, mShow->GetCurrentSheetNum(), minyards, mIsSheetPicked);
//	// stream to file:
//	{
//		wxFFileOutputStream outstream(s);
//		outstream.Write(buffer.str().c_str(), buffer.str().size());
//	}
}



int IndexForContNames(const std::string& name);

bool ContinuityCountDifferentThanSymbol(const char* show)
{
	std::ifstream input(show);
	if (!input.is_open())
	{
		throw std::runtime_error("could not open file");
	}
	std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
	return false;
}

void FindContinuityInconsistancies(const char* show)
{
	std::ifstream input(show);
	if (!input.is_open())
	{
		throw std::runtime_error("could not open file");
	}
}

int verbose_flag = 1;
int print_flag = 0;
int animate_flag = 0;
int dump_continuity = 0;
int check_flag = 0;

static struct option long_options[] =
{
	/* These options set a flag. */
	{"print_show", no_argument,    &print_flag, 1}, // p
	{"check_flag", no_argument,    &check_flag, 1}, // c
	{"dump_continuity", no_argument,    &dump_continuity, 1}, // c
	{"animate_flag", no_argument,    &animate_flag, 1}, // a
	{0, 0, 0, 0}
};

int main(int argc, char * argv[])
{
    opterr = 0;
	int c = 0;
	while ((c = getopt (argc, argv, "cpad")) != -1)
		switch (c)
	{
		case 'p':
			print_flag = true;
			break;
		case 'a':
			animate_flag = true;
			break;
		case 'd':
			dump_continuity = true;
			break;
		case 'c':
			check_flag = true;
			break;
	}
	while (optind < argc)
	{
		try {
		if (print_flag)
		{
			PrintShow(argv[optind]);
		}
		if (animate_flag)
		{
			AnimateShow(argv[optind]);
		}
		if (dump_continuity)
		{
			DumpContinuity(argv[optind]);
		}
		if (check_flag)
		{
			std::cout<<"ContinuityCountDifferentThanSymbol ? "<<ContinuityCountDifferentThanSymbol(argv[optind])<<"\n";
			FindContinuityInconsistancies(argv[optind]);
		}
		} catch (const std::runtime_error& e) {
			std::cerr<<"Error on file "<<argv[optind]<<": "<<e.what()<<"\n";
		}
		++optind;
	}
	
    return 0;
}

