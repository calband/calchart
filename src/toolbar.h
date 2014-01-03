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

/** 
 * A structure containing all data required to add a tool to a toolbar
 * through wxWidgets.
 */
struct ToolBarEntry
{
	/** 
	 * The tool's type (e.g. a dropdown menu or toggle button).
	 * A list of tool types can be found at:
	 * http://docs.wxwidgets.org/trunk/defs_8h.html#abd0c640814a55e4adda3bce698646d2d
	 */
	wxItemKind kind;

	/**
	 * The image for the tool.
	 */
	wxBitmap *bm;

	/**
	 * The description of the tool. This appears when the
	 * user hovers his/her mouse over the tool.
	 */
	wxString desc;

	/**
	 * The identity of the tool. This id is attached through
	 * wxWidgets to something that can actually perform the
	 * behavior of the tool, like a method. This id, therefore,
	 * defines the behavior that results from working with this
	 * tool.
	 */
	int id;

	/**
	 * True if the tool should be separated (visually, in some way),
	 * from tools following it in the toolbar; false otherwise.
	 */
	bool space;
};

/**
 * Adds a new toolbar to a frame, where the toolbar has tools in it defined
 * by the ToolBarEntry objects.
 * @param entries A collection of ToolBarEntry objects that define all of the
 * tools that should appear in the final toolbar.
 * @param fram The frame to add the toolbar to.
 */
void AddCoolToolBar(const std::vector<ToolBarEntry> &entries, wxFrame& frame);

/**
 * Creates a collection of ToolBarEntry objects that describe all of the tools
 * that appear in the 'Main' toolbar (the toolbar containing tools related to
 * editing a CalChart show).
 * @return A collection of ToolBarEntry objects that describe all of the tools
 * that appear in the 'Main' toolbar.
 */
std::vector<ToolBarEntry> GetMainToolBar();

/**
 * Creates a collection of TooBarEntry objects that describe all of the tools
 * that appear in the 'Animation' toolbar (the toolbar that appears in the
 * CalChart show viewer).
 */
std::vector<ToolBarEntry> GetAnimationToolBar();

#endif // _TOOLBAR_H_
