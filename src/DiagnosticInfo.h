#pragma once
/*
 * DiagnosticInfo.h
 * wxWidgets-specific diagnostic information collection for bug reporting
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

#include "CalChartDiagnosticInfo.hpp"
#include <memory>
#include <string>
#include <vector>
#include <wx/wx.h>

// Forward declarations
class CalChartDoc;
class wxConfigBase;

namespace wxCalChart {

// Collect all diagnostic information
// Returns a complete CalChart::DiagnosticInfo with both Core and system info
[[nodiscard]] auto CollectDiagnosticInfo(CalChartDoc const* doc = nullptr) -> CalChart::DiagnosticInfo;

} // namespace wxCalChart
