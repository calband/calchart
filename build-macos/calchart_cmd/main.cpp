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

namespace calchart {
int main(int argc, char * argv[]);
}

int main(int argc, char * argv[])
{
	calchart::main(argc, argv);
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

