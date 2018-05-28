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
#include "cont.h"
#include <assert.h>
#include <sstream>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

extern int parsecontinuity();
extern const char* yyinputbuffer;
extern std::vector<std::unique_ptr<CalChart::ContProcedure>> ParsedContinuity;

namespace CalChart {

std::vector<std::unique_ptr<ContProcedure>>
Continuity::ParseContinuity(std::string const& s)
{
    yyinputbuffer = s.c_str();
    // parse out the error
    if (parsecontinuity() != 0) {
        ContToken dummy;
        throw ParseError(s, dummy.line, dummy.col);
    }
    return std::move(ParsedContinuity);
}

Continuity::Continuity(std::string const& s)
    : text(s)
    , m_parsedContinuity(ParseContinuity(s))
{
}

Continuity::~Continuity() = default;

Continuity::Continuity(Continuity const& other)
    : text(other.text)
{
    for (auto&& i : other.m_parsedContinuity) {
        m_parsedContinuity.emplace_back(i->clone());
    }
}

Continuity::Continuity(std::vector<std::unique_ptr<ContProcedure>> from_cont)
    : m_parsedContinuity(std::move(from_cont))
{
}

Continuity& Continuity::operator=(Continuity const& other)
{
    Continuity copy(other);
    swap(*this, copy);
    return *this;
}

Continuity::Continuity(Continuity&&) noexcept = default;
Continuity& Continuity::operator=(Continuity&&) noexcept = default;

bool operator==(Continuity const& lhs, Continuity const& rhs)
{
    return std::equal(lhs.m_parsedContinuity.begin(), lhs.m_parsedContinuity.end(), rhs.m_parsedContinuity.begin(), rhs.m_parsedContinuity.end(), [](auto&& a, auto&& b) {
        return *a == *b;
    });
}

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

void Continuity_serialize_test()
{
    // Set some text
    {
        std::ostringstream ofs("filename");

        ContValue* p1 = new ContFuncStep(new ContFuncStep(new ContValueVar(1), new ContValueNeg(new ContValueFloat(-1.5)), new ContPoint()),
            new ContFuncStep(new ContValueFloat(32.5), new ContValueFloat(3), new ContRefPoint(3)),
            new ContStartPoint());
        // save data to archive
        {
            boost::archive::text_oarchive oa(ofs);
            // write class instance to archive
            oa << p1;
        }
        ContValue* newp1 = nullptr;
        std::istringstream ifs(ofs.str());
        {
            boost::archive::text_iarchive ia(ifs);
            // read class state from archive
            ia >> newp1;
        }
        ContFuncStep* dyncast = dynamic_cast<ContFuncStep*>(newp1);
        assert(dyncast);
        assert(*p1 == *newp1);
    }
    // Set some text
    {
        std::ostringstream ofs("filename");

        Continuity cont1{ "mt E REM" };
        // save data to archive
        {
            boost::archive::text_oarchive oa(ofs);
            // write class instance to archive
            oa << cont1;
        }
        Continuity cont2;
        std::istringstream ifs(ofs.str());
        {
            boost::archive::text_iarchive ia(ifs);
            // read class state from archive
            ia >> cont2;
        }
        assert(cont1 == cont2);
    }
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
    underTest2 = Continuity{ "mt E REM" };
    values.text = "mt E REM";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Set some text
    underTest2 = Continuity{ "ewns np" };
    values.text = "ewns np";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    // Reset text
    underTest2 = Continuity{ "" };
    values.text = "";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

    Continuity_serialize_test();
}
}
