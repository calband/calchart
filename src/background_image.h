/*
 * background_image.h
 * Header for background image
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

#pragma once

#include <array>
#include <memory>
#include <wx/dc.h>
#include <wx/event.h>

class BackgroundImage {
public:
    BackgroundImage(const wxImage& image, int x, int y, int scaled_width, int scaled_height);

    bool MouseClickIsHit(const wxMouseEvent& event, const wxDC& dc) const;
    void OnMouseLeftDown(const wxMouseEvent& event, const wxDC& dc);
    // returns left, top, width, height
    std::array<int, 4> OnMouseLeftUp(const wxMouseEvent& event, const wxDC& dc);
    void OnMouseMove(const wxMouseEvent& event, const wxDC& dc);
    void OnPaint(wxDC& dc, bool drawPicAdjustDots, bool selected) const;

private:
    static const long kCircleSize = 6;

    wxImage mImage;
    wxBitmap mBitmap;
    wxCoord mBitmapX, mBitmapY;

    // what type of background adjustments could we do
    typedef enum {
        kUpperLeft = 0,
        kUpper,
        kUpperRight,
        kLeft,
        kMove,
        kRight,
        kLowerLeft,
        kLower,
        kLowerRight,
        kLast,
    } eBackgroundAdjustType;
    eBackgroundAdjustType mBackgroundAdjustType;

    class CalculateScaleAndMove {
    public:
        CalculateScaleAndMove(const wxPoint& startClick, wxCoord x, wxCoord y,
            wxCoord width, wxCoord height,
            eBackgroundAdjustType adjustType);
        void operator()(wxCoord x, wxCoord y, wxCoord& topX, wxCoord& topY,
            wxCoord& width, wxCoord& height);
        wxPoint mStartClick;
        wxCoord mLeft, mTop;
        wxCoord mRight, mBottom;
        float mAspectRatio;
        eBackgroundAdjustType mAdjustType;
    };
    std::shared_ptr<CalculateScaleAndMove> mScaleAndMove;
};
