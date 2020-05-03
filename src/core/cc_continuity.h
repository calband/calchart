#pragma once
/*
 * cc_continuity.h
 * Definitions for the continuity classes
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

#include "cont.h"

#include <string>
#include <vector>

namespace CalChart {

class ContProcedure;
struct ParseErrorHandlers;

struct ParseError : public std::runtime_error {
    ParseError(std::string const& str, int l, int c)
        : std::runtime_error(std::string("ParseError of ") + str + " at " + std::to_string(l) + ", " + std::to_string(c))
        , line(l)
        , column(c)
    {
    }
    int line, column;
};

class Continuity {
public:

    // this could throw ParseError
    Continuity(std::string const& s = "", ParseErrorHandlers const* correction = nullptr);
    Continuity(std::vector<std::unique_ptr<ContProcedure>>);
    Continuity(std::vector<uint8_t> const&);
    ~Continuity();

    Continuity(Continuity const&);
    Continuity& operator=(Continuity const&);
    Continuity(Continuity&&) noexcept;
    Continuity& operator=(Continuity&&) noexcept;

    std::vector<uint8_t> Serialize() const;

    std::vector<std::unique_ptr<ContProcedure>> const& GetParsedContinuity() const noexcept { return m_parsedContinuity; }
    auto GetText() const { return m_legacyText; }

    friend void swap(Continuity& lhs, Continuity& rhs)
    {
        using std::swap;
        swap(lhs.m_parsedContinuity, rhs.m_parsedContinuity);
    }
    friend bool operator==(Continuity const& lhs, Continuity const& rhs);

private:
    static std::vector<std::unique_ptr<ContProcedure>> ParseContinuity(std::string const& s, ParseErrorHandlers const* correction = nullptr);
    static std::vector<std::unique_ptr<ContProcedure>> Deserialize(std::vector<uint8_t> const&);

    std::vector<std::unique_ptr<ContProcedure>> m_parsedContinuity;
    std::string m_legacyText;

    friend bool Check_Continuity(const Continuity&, const struct Continuity_values&);
    friend void Continuity_serialize_test();
    friend void Continuity_UnitTests();
};

bool Check_Continuity(const Continuity&,
    const struct Continuity_values&);

void Continuity_UnitTests();
}
