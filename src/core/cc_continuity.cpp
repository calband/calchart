/*
 * cc_continuity.cpp
 * Implementation of contiunity classes
 */

/*
   Copyright (C) 1995-2011  Garrick Brian Meeker, Richard Michael Powell

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "cc_continuity.h"
#include <boost/algorithm/string/case_conv.hpp>

using std::string;

static string Capitalize(const string &str)
{
	string Result = boost::to_lower_copy(str);
	if (!Result.empty())
		Result.at(0) = toupper(Result.at(0));

	return Result;
}

CC_continuity::CC_continuity(const string& s, unsigned n)
: name(Capitalize(s)), num(n)
{
}

CC_continuity::~CC_continuity()
{
}

const string& CC_continuity::GetName() const
{
	return name;
}

unsigned CC_continuity::GetNum() const
{
	return num;
}

void CC_continuity::SetText(const string& s)
{
	text = s;
}

void CC_continuity::AppendText(const string& s)
{
	text.append(s);
}

const string& CC_continuity::GetText() const
{
	return text;
}


// Test Suite stuff
struct CC_continuity_values
{
	string name;
	unsigned num;
	string text;
	string GetName;
	unsigned GetNum;
	string GetText;
};


bool Check_CC_continuity(const CC_continuity& underTest, const CC_continuity_values& values)
{
	return (underTest.num == values.num)
		&& (underTest.name == values.name)
		&& (underTest.text == values.text)
		&& (underTest.GetName() == values.GetName)
		&& (underTest.GetNum() == values.GetNum)
		&& (underTest.GetText() == values.GetText)
		;
}

void CC_continuity_UnitTests()
{
	// test some defaults:
	CC_continuity_values values;
	values.num = 0;
	values.name = "Unittests";
	values.text = "";
	values.GetName = values.name;
	values.GetNum = values.num;
	values.GetText = values.text;

	// test defaults
	CC_continuity underTest("UNITTESTS", 0);
	assert(Check_CC_continuity(underTest, values));

	// test defaults with different init
	CC_continuity underTest2("uNITTEST2", 10);
	values.num = 10;
	values.name = "Unittest2";
	values.GetName = values.name;
	values.GetNum = values.num;
	values.GetText = values.text;
	assert(Check_CC_continuity(underTest2, values));

	// Set some text
	underTest2.SetText("this is some text");
	values.text = "this is some text";
	values.GetName = values.name;
	values.GetNum = values.num;
	values.GetText = values.text;
	assert(Check_CC_continuity(underTest2, values));

	// Append some more text
	underTest2.AppendText("Adding more");
	values.text = "this is some textAdding more";
	values.GetName = values.name;
	values.GetNum = values.num;
	values.GetText = values.text;
	assert(Check_CC_continuity(underTest2, values));

	// Set some text
	underTest2.SetText("different words");
	values.text = "different words";
	values.GetName = values.name;
	values.GetNum = values.num;
	values.GetText = values.text;
	assert(Check_CC_continuity(underTest2, values));

	// Reset text
	underTest2.SetText("");
	values.text = "";
	values.GetName = values.name;
	values.GetNum = values.num;
	values.GetText = values.text;
	assert(Check_CC_continuity(underTest2, values));
}
