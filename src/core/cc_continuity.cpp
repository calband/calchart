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
#include <assert.h>

namespace CalChart {

Continuity::Continuity() {}

Continuity::~Continuity() {}

void Continuity::SetText(const std::string& s) { text = s; }

void Continuity::AppendText(const std::string& s) { text.append(s); }

const std::string& Continuity::GetText() const { return text; }

// Test Suite stuff
struct Continuity_values {
    std::string text;
    std::string GetText;
};

bool Check_Continuity(const Continuity& underTest,
    const Continuity_values& values)
{
    return (underTest.text == values.text) && (underTest.GetText() == values.GetText);
}

void Continuity_UnitTests()
{
    // test some defaults:
    Continuity_values values;
    values.text = "";
    values.GetText = values.text;

    // test defaults
    Continuity underTest;
    assert(Check_Continuity(underTest, values));

    // test defaults with different init
    Continuity underTest2;
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Set some text
    underTest2.SetText("this is some text");
    values.text = "this is some text";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Append some more text
    underTest2.AppendText("Adding more");
    values.text = "this is some textAdding more";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Set some text
    underTest2.SetText("different words");
    values.text = "different words";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Reset text
    underTest2.SetText("");
    values.text = "";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));
}
}
