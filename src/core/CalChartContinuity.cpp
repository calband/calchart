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

#include "CalChartContinuity.h"
#include "CalChartContinuityToken.h"
#include "CalChartFileFormat.h"
#include "CalChartTypes.h"
#include "parse.h"
#include <cassert>
#include <sstream>

// These are the "magic" global variables that are in contgram and contscan.
// The are the data objects used by the lexer and parser of the older calchart syntax.
extern int parsecontinuity();
extern const char* yyinputbuffer;
extern std::vector<std::unique_ptr<CalChart::Cont::Procedure>> ParsedContinuity;

namespace CalChart {

// if any errors happen during parse, a ParseError may be thrown.
std::runtime_error ParseError(std::string const& str, int l, int c)
{
    return std::runtime_error{ std::string("ParseError of ") + str + " at " + std::to_string(l) + ", " + std::to_string(c) };
}

std::vector<std::unique_ptr<Cont::Procedure>> ParseContinuity(std::string const& s, ParseErrorHandlers const* correct)
{
    ParsedContinuity = std::vector<std::unique_ptr<CalChart::Cont::Procedure>>{};
    std::string thisParse = s;
    while (1) {
        yyinputbuffer = thisParse.c_str();
        // parse out the error
        if (parsecontinuity() == 0) {
            return std::move(ParsedContinuity);
        }
        if (correct && correct->mContinuityParseCorrectionHandler) {
            // give the user a chance to correct.
            thisParse = correct->mContinuityParseCorrectionHandler(std::string("Could not parse line ") + std::to_string(yylloc.first_line) + " at " + std::to_string(yylloc.first_column), thisParse, yylloc.first_line, yylloc.first_column);
        } else {
            throw ParseError(s, 0, 0);
        }
    }
}

std::vector<std::unique_ptr<Cont::Procedure>> Deserialize(Reader reader)
{
    auto result = std::vector<std::unique_ptr<Cont::Procedure>>{};

    while (reader.size() > 0) {
        auto [next_result, new_reader] = Cont::DeserializeProcedure(reader);
        result.push_back(std::move(next_result));
        reader = new_reader;
    }
    if (reader.size() != 0) {
        throw std::runtime_error("Error, did not parse all the data correctly");
    }
    return result;
}

Continuity::Continuity(std::string const& s, ParseErrorHandlers const* correction)
    : m_parsedContinuity(ParseContinuity(s, correction))
    , m_legacyText(s)
{
}

Continuity::~Continuity() = default;

Continuity::Continuity(Continuity const& other)
{
    for (auto&& i : other.m_parsedContinuity) {
        m_parsedContinuity.emplace_back(i->clone());
    }
}

Continuity::Continuity(std::vector<std::unique_ptr<Cont::Procedure>> from_cont)
    : m_parsedContinuity(std::move(from_cont))
{
}

Continuity::Continuity(Reader reader)
    : m_parsedContinuity(Deserialize(reader))
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

auto Continuity::Serialize() const -> std::vector<std::byte>
{
    std::vector<std::byte> result;
    for (auto&& i : m_parsedContinuity) {
        Parser::Append(result, i->Serialize());
    }
    return result;
}

// Test Suite stuff
struct Continuity_values {
    std::string text;
    std::string GetText;
};

bool Check_Continuity(Continuity const&, Continuity_values const&)
{
    return true;
}

void Continuity_serialize_test()
{
    // Set some text
    for (auto i : {
             "mt E REM",
             "BLAM",
             "close 1 0",
             "Countermarch R1 R2 1 N E 16",
             "DMCM SP NP 6 / 3",
             "DMHS NP",
             "EVEN 10 + 3 NP",
             "EWNS NP",
             "FM 10 - 3 N",
             "FMTO R3",
             "FOUNTAIN DIR(NP) DIRFROM(SP NP) DIST(NP) 3 NP",
             "FOUNTAIN DIR(NP) DIRFROM(SP NP) NP",
             "GRID DISTFROM(R1 R2)",
             "HSCM NP R1 EITHER(N S R1)",
             "HSDM NP",
             "MAGIC NP",
             "MARCH GV STEP(2 2 R1) OPP(S)",
             "MARCH GV STEP(2 2 R1) OPP(S) S",
             "MT 1 1",
             "MTRM 10.5",
             "NSEW SP",
             "ROTATE 90 SH R2",
             "ROTATE -90 SH R2",
             "A = 10 * 9",
             "  ",

         }) {
        auto uut1 = Continuity{ i };
        auto serialize_result = uut1.Serialize();
        auto reader = Reader({ serialize_result.data(), serialize_result.size() });
        auto uut2 = Continuity{ reader };
        assert(uut1 == uut2);
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
    values.text = "mt E REM";
    values.GetText = values.text;
    assert(Check_Continuity(underTest2, values));

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
