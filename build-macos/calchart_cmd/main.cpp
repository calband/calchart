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
	CC_show p;
	Animation a(p, [](const std::string& notice) { std::cout<<notice<<"\n"; }, [](const std::vector<ErrorMarker>&, unsigned, const std::string&) { return true; });
	std::cout << "Hello, World!\n";
    return 0;
}

