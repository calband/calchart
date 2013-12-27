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

void Notify(const std::string& notice)
{
	std::cout<<notice<<"\n";
}

bool NotifyError(const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const std::string& message)
{
	return true;
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

int IndexForContNames(const std::string& name);

bool ContinuityCountDifferentThanSymbol(const char* show)
{
	std::ifstream input(show);
	if (!input.is_open())
	{
		throw std::runtime_error("could not open file");
	}
	std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
	for (auto i = p->GetSheetBegin(); i != p->GetSheetEnd(); ++i)
	{
		auto points = i->GetPoints();
		std::map<SYMBOL_TYPE, uint8_t> continity_for_symbol;
		std::map<uint8_t, SYMBOL_TYPE> symbol_for_continuity;
		for (auto& point : points)
		{
			// if we haven't seen this cont symbol yet, then save the continuity index
			// else, check the index, and if it's different, then return true;
			auto symbol = point.GetSymbol();
			auto cont_index = point.GetContinuityIndex();
			if (continity_for_symbol.count(symbol) == 0)
			{
				continity_for_symbol[symbol] = cont_index;
			}
			else
			{
				if (continity_for_symbol[symbol] != cont_index)
				{
					return true;
				}
			}
			if (symbol_for_continuity.count(cont_index) == 0)
			{
				symbol_for_continuity[cont_index] = symbol;
			}
			else
			{
				if (symbol_for_continuity[cont_index] != symbol)
				{
					return true;
				}
			}
		}
	}

	return false;
}

void FindContinuityInconsistancies(const char* show)
{
	std::ifstream input(show);
	if (!input.is_open())
	{
		throw std::runtime_error("could not open file");
	}
	
	std::unique_ptr<CC_show> p(CC_show::Create_CC_show(input));
	for (auto i = p->GetSheetBegin(); i != p->GetSheetEnd(); ++i)
	{
		auto points = i->GetPoints();
		std::map<uint8_t, SYMBOL_TYPE> symbol_for_continuity;
		for (auto point = points.begin(); point != points.end(); ++point)
		{
			// if we haven't seen this cont symbol yet, then save the continuity index
			// else, check the index, and if it's different, then return true;
			auto symbol = point->GetSymbol();
			auto cont_index = point->GetContinuityIndex();
			if (symbol_for_continuity.count(cont_index) == 0)
			{
				symbol_for_continuity[cont_index] = symbol;
			}
			else
			{
				if (symbol_for_continuity[cont_index] != symbol)
				{
					std::cout<<"on sheet "<<std::distance(p->GetSheetBegin(), i)<<": ";
					std::cout<<"point "<<std::distance(points.begin(), point)<<" inconsistancy: symbol "<<symbol<<" has index "<<(int)cont_index<<".  Index "<<(int)cont_index<<" should be symbol "<<symbol_for_continuity[cont_index]<<"\n";
				}
			}
		}
		auto theContContainer = i->GetAnimationContinuity();
		for (auto& j : theContContainer)
		{
			auto indexForCont = IndexForContNames(j.GetName());
			if (indexForCont < 0)
			{
				std::cout<<"on sheet "<<std::distance(p->GetSheetBegin(), i)<<": ";
				std::cout<<"continuity inconsistancy: name "<<j.GetName()<<" doesn't match any known names\n";
			}
			if (symbol_for_continuity.count(j.GetNum()) && (indexForCont >= 0) && (symbol_for_continuity[j.GetNum()] != indexForCont))
			{
				std::cout<<"on sheet "<<std::distance(p->GetSheetBegin(), i)<<": ";
				std::cout<<"continuity inconsistancy: continuity "<<j.GetName()<<" has name index "<<indexForCont<<", points have name index "<<symbol_for_continuity[j.GetNum()]<<"\n";
			}
		}
		
	}
}

int main(int argc, char * argv[])
{
    opterr = 0;
	int doPrint = 0;
	int doCheck = 0;
	int c = 0;
	while ((c = getopt (argc, argv, "cp")) != -1)
		switch (c)
	{
		case 'p':
			doPrint = true;
			break;
		case 'c':
			doCheck = true;
			break;
	}
	while (optind < argc)
	{
		try {
		if (doPrint)
		{
			PrintShow(argv[optind]);
		}
		if (doCheck)
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

