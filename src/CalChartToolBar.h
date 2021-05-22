#pragma once

/*
 * CalChartToolBar.h
 * Header for adding toolbars to CalChart
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

#include <wx/toolbar.h>
#include <wx/wx.h>

#include <vector>

class wxAuiToolBar;
struct ToolBarEntry;

std::vector<wxBitmap> GetSymbolsBitmap();

wxAuiToolBar* CreateSelectAndMoves(wxWindow* parent, wxWindowID id = wxID_ANY, long style = 0);
wxAuiToolBar* CreateDotModifiers(wxWindow* parent, wxWindowID id = wxID_ANY, long style = 0);
