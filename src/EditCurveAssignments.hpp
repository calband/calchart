#pragma once
/*
 * EditCurveAssignments.hpp
 * Dialog for Edting which Marchers are assigned to a curve
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartDoc.h"
#include <vector>
#include <wxUI/wxUI.hpp>

// Simple: Check the show to see what curves there are.
class EditCurveAssignments : public wxDialog {
    using super = wxDialog;

public:
    EditCurveAssignments(wxWindow* parent, CalChartDoc const& show, int whichCurve);
    ~EditCurveAssignments() = default;

    [[nodiscard]] auto GetCurveAssignment() const -> std::vector<std::string> { return mCurveAssignment; }

private:
    std::vector<std::string> mCurveAssignment;
};
