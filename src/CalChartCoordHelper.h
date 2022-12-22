#pragma once
/*
 * CalChartCoordHelper.h
 * Extensions for the CalChartCoord
 */

/*
   Copyright (C) 1995-2022  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartCoord.h"
#include <wx/gdicmn.h>

namespace wxCalChart {

inline auto to_wxPoint(CalChart::Coord c)
{
    return wxPoint{ c.x, c.y };
}

inline auto to_wxSize(CalChart::Coord c)
{
    return wxSize{ c.x, c.y };
}

}