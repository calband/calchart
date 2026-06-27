#pragma once
/*
 * ViewHandlers.hpp
 */

/*
   Copyright (C) 1995-2026  Garrick Brian Meeker, Richard Michael Powell

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
#include "CalChartRanges.h"
#include "ContinuityBrowser.h"
#include "FieldThumbnailBrowser.h"

namespace CalChart {
class Show;
class Configuration;
}

auto CreateContinuityBrowserHandlers(CalChartView* view) -> ContinuityBrowser::Handlers;
auto GetDebugContinuityHandlers(
    CalChart::Continuity const& plainCont,
    CalChart::Continuity const& solCont,
    std::function<void(CalChart::SYMBOL_TYPE, CalChart::Continuity const&)> onUpdate,
    std::function<void(CalChart::SYMBOL_TYPE)> onSetSelection) -> ContinuityBrowser::Handlers;

auto CreateFieldThumbnailBrowserHandlers(CalChartView* view) -> FieldThumbnailBrowser::Handlers;
