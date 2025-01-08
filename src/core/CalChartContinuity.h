#pragma once
/*
 * CalChartContinuity.h
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

/**
 * CalChartContinuity
 *
 * CalChart::Continuity represents the continuity for a specific squad of marchers.  In CalChart3.6 and earlier, Continuity was represent
 * as a list of marching commands in text that would then be parsed into an abstract syntax tree of ContProcedures.  In newer versions
 * the data structure is created via the Continuity Composer.
 *
 * CalChartContinuity requires that the ContProcedures it holds to be valid objects.  Because the CalChart string represented in older
 * calchart files may not parse correctly, we provide a way that upon detection of error that the procedure can be "re-written".  This allows
 * us to have the user attempt to correct a unusal CalChart syntax before we "give up".
 *
 */

#include "CalChartFileFormat.h"

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace CalChart {

namespace Cont {
    class Procedure;
}
struct ParseErrorHandlers;

class Continuity {
public:
    // this could throw runtime_error on bad parses.
    Continuity(std::string const& s = "", ParseErrorHandlers const* correction = nullptr);
    Continuity(std::vector<std::unique_ptr<Cont::Procedure>>);
    Continuity(Reader);
    ~Continuity();

    Continuity(Continuity const&);
    Continuity& operator=(Continuity const&);
    Continuity(Continuity&&) noexcept;
    Continuity& operator=(Continuity&&) noexcept;

    [[nodiscard]] auto Serialize() const -> std::vector<std::byte>;

    std::vector<std::unique_ptr<Cont::Procedure>> const& GetParsedContinuity() const noexcept { return m_parsedContinuity; }
    [[nodiscard]] auto HasParsedContinuity() const { return !m_parsedContinuity.empty(); }
    auto GetText() const { return m_legacyText; }

    friend void swap(Continuity& lhs, Continuity& rhs)
    {
        using std::swap;
        swap(lhs.m_parsedContinuity, rhs.m_parsedContinuity);
    }
    friend bool operator==(Continuity const& lhs, Continuity const& rhs);

private:
    std::vector<std::unique_ptr<Cont::Procedure>> m_parsedContinuity;
    std::string m_legacyText;

    friend bool Check_Continuity(const Continuity&, const struct Continuity_values&);
    friend void Continuity_serialize_test();
    friend void Continuity_UnitTests();
};

bool Check_Continuity(const Continuity&,
    const struct Continuity_values&);

void Continuity_UnitTests();
}
