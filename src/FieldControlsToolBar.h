#pragma once
/*
 * FieldControlsToolBar
 * Pane for the field frame controls window
 */

/*
   Copyright (C) 1995-2024  Garrick Brian Meeker, Richard Michael Powell

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
#include "CalChartTypes.h"
#include <wx/wx.h>

class wxAuiToolBar;
namespace CalChart {
class Configuration;
}

namespace FieldControls {

wxAuiToolBar* CreateToolBar(wxWindow* parent, wxWindowID idenity, long style, CalChart::Configuration& config);
std::pair<CalChart::Coord::units, CalChart::Coord::units> GridChoice(wxWindow* target);
std::pair<CalChart::Coord::units, CalChart::Coord::units> ToolGridChoice(wxWindow* target);
double GetZoomAmount(wxWindow* target);
void SetZoomAmount(wxWindow* target, double zoom);
int GetRefChoice(wxWindow* target);
int GetGhostChoice(wxWindow* target);
void SetGhostChoice(wxWindow* target, int which);
void SetInstrumentsInUse(wxWindow* target, std::vector<std::string> const& instruments);
void SetLabelsInUse(wxWindow* target, std::vector<std::string> const& labels);

}
