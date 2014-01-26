/*
 * toolbar.h
 * Header for adding toolbars to windows
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

#ifndef _TOOLBAR_H_
#define _TOOLBAR_H_

#include <wx/wx.h>
#include <wx/toolbar.h>

#include <vector>

struct ToolBarEntry
{
	wxItemKind kind;
	wxBitmap *bm;
	wxString desc;
	int id;
	bool space;
};

void AddCoolToolBar(const std::vector<ToolBarEntry> &entries, wxFrame& frame);
std::vector<ToolBarEntry> GetSymbolsToolBar();
std::vector<ToolBarEntry> GetMainToolBar();
std::vector<ToolBarEntry> GetAnimationToolBar();

#endif // _TOOLBAR_H_
