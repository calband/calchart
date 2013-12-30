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

