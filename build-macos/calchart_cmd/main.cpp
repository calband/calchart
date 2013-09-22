//
//  main.cpp
//  calchart_cmd
//
//  Created by Richard Powell on 9/20/13.
//
//

#include "cc_show.h"
#include "animate.h"

#include <iostream>
#include <fstream>
#include <iterator>

void Notify(const std::string& notice)
{
	std::cout<<notice<<"\n";
}

bool NotifyError(const std::vector<ErrorMarker>& error_markers, unsigned sheetnum, const std::string& message)
{
	return true;
}

int main(int argc, const char * argv[])
{
	// insert code here...
	if (argc > 1)
	{
		std::ifstream input(argv[1]);
		CC_show p(input);
		Animation a(p, [](const std::string& notice) { std::cout<<notice<<"\n"; }, [](const std::vector<ErrorMarker>&, unsigned, const std::string& error) { std::cout<<"error"<<error<<"\n"; return true; });
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
	
    return 0;
}

