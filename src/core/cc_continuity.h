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

#include <boost/serialization/access.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/vector.hpp>

namespace CalChart {

class ContProcedure;

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
    Continuity(std::string const& s = "");
    Continuity(std::vector<std::unique_ptr<ContProcedure>>);
    ~Continuity();

    Continuity(Continuity const&);
    Continuity& operator=(Continuity const&);
    Continuity(Continuity&&) noexcept;
    Continuity& operator=(Continuity&&) noexcept;

    auto GetText() const noexcept { return text; }

    std::vector<std::unique_ptr<ContProcedure>> const& GetParsedContinuity() const noexcept { return m_parsedContinuity; }

    static std::vector<std::unique_ptr<ContProcedure>> ParseContinuity(std::string const& s);

    friend void swap(Continuity& lhs, Continuity& rhs)
    {
        using std::swap;
        swap(lhs.text, rhs.text);
        swap(lhs.m_parsedContinuity, rhs.m_parsedContinuity);
    }
    friend bool operator==(Continuity const& lhs, Continuity const& rhs);

private:
    friend class boost::serialization::access;
    template <class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar& text;
        ar& m_parsedContinuity;
    }

    std::string text;
    std::vector<std::unique_ptr<ContProcedure>> m_parsedContinuity;

    friend bool Check_Continuity(const Continuity&, const struct Continuity_values&);
    friend void Continuity_serialize_test();
    friend void Continuity_UnitTests();
};

bool Check_Continuity(const Continuity&,
    const struct Continuity_values&);

void Continuity_UnitTests();
}
