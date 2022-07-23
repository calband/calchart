#pragma once
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

#include <wx/dc.h>

namespace SaveAndRestore {
// helper classes for saving and restoring state
class DeviceOrigin {
    wxDC& dc;
    wxCoord origX, origY;

public:
    DeviceOrigin(wxDC& dc_)
        : dc(dc_)
    {
        dc.GetDeviceOrigin(&origX, &origY);
    }
    ~DeviceOrigin() { dc.SetDeviceOrigin(origX, origY); }
};

class UserScale {
    wxDC& dc;
    double origXscale, origYscale;

public:
    UserScale(wxDC& dc_)
        : dc(dc_)
    {
        dc.GetUserScale(&origXscale, &origYscale);
    }
    ~UserScale() { dc.SetUserScale(origXscale, origYscale); }
};

class TextForeground {
    wxDC& dc;
    wxColour origForegroundColor;

public:
    TextForeground(wxDC& dc_)
        : dc(dc_)
        , origForegroundColor(dc.GetTextForeground())
    {
    }
    ~TextForeground()
    {
        dc.SetTextForeground(origForegroundColor);
    }
};

class Font {
    wxDC& dc;
    wxFont origFont;

public:
    Font(wxDC& dc_)
        : dc(dc_)
        , origFont(dc.GetFont())
    {
    }
    ~Font() { dc.SetFont(origFont); }
};

class Brush {
    wxDC& dc;
    wxBrush origBrush;

public:
    Brush(wxDC& dc_)
        : dc(dc_)
        , origBrush(dc.GetBrush())
    {
    }
    ~Brush() { dc.SetBrush(origBrush); }
};

// class for saving and restoring
class BrushAndPen {
    wxDC& mDC;
    wxBrush const& mBrush;
    wxPen const& mPen;

public:
    BrushAndPen(wxDC& dc)
        : mDC(dc)
        , mBrush(dc.GetBrush())
        , mPen(dc.GetPen())
    {
    }
    ~BrushAndPen()
    {
        mDC.SetBrush(mBrush);
        mDC.SetPen(mPen);
    }
};

}
