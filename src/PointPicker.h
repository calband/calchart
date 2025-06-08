#pragma once
/*
 * MarcherPicker.hpp
 * Dialog for picking Marcher
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

#include "CalChartTypes.h"
#include <optional>
#include <vector>

class CalChartDoc;
class wxWindow;

auto PromptUserToPickMarchers(wxWindow* parent, CalChartDoc const& show, CalChart::SelectionList const& marchersToUse, CalChart::SelectionList const& selected) -> std::optional<std::vector<std::string>>;
auto PromptUserToPickMarchers(wxWindow* parent, CalChartDoc const& show) -> std::optional<std::vector<std::string>>;
