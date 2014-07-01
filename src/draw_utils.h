/*
 * draw.cpp
 * Member functions for drawing stuntsheets
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

#ifndef __DRAW_UTILS_H__
#define __DRAW_UTILS_H__

#include <wx/dc.h>

// helper classes for saving and restoring state
class SaveAndRestore_DeviceOrigin
{
	wxDC& dc;
	wxCoord origX, origY;
public:
	SaveAndRestore_DeviceOrigin(wxDC& dc_) : dc(dc_) { dc.GetDeviceOrigin(&origX, &origY); }
	~SaveAndRestore_DeviceOrigin() { dc.SetDeviceOrigin(origX, origY); }
};

class SaveAndRestore_UserScale
{
	wxDC& dc;
	double origXscale, origYscale;
public:
	SaveAndRestore_UserScale(wxDC& dc_) : dc(dc_) { dc.GetUserScale(&origXscale, &origYscale); }
	~SaveAndRestore_UserScale() { dc.SetUserScale(origXscale, origYscale); }
};

class SaveAndRestore_TextForeground
{
	wxDC& dc;
	wxColour origForegroundColor;
public:
	SaveAndRestore_TextForeground(wxDC& dc_) : dc(dc_), origForegroundColor(dc.GetTextForeground()) {}
	~SaveAndRestore_TextForeground() { dc.SetTextForeground(origForegroundColor); }
};

class SaveAndRestore_Font
{
	wxDC& dc;
	wxFont origFont;
public:
	SaveAndRestore_Font(wxDC& dc_) : dc(dc_), origFont(dc.GetFont()) {}
	~SaveAndRestore_Font() { dc.SetFont(origFont); }
};

#endif // __DRAW_UTILS_H__
