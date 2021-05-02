#pragma once
/*
 * BackgroundImage.h
 * Maintains the background image data
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

#include <array>
#include <memory>
#include <wx/dc.h>
#include <wx/event.h>

class BackgroundImage {
public:
    BackgroundImage(wxImage const& image, int x, int y, int scaled_width, int scaled_height);

    bool MouseClickIsHit(wxMouseEvent const& event, wxDC const& dc) const;
    void OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc);
    // returns left, top, width, height
    std::array<int, 4> OnMouseLeftUp(wxMouseEvent const& event, wxDC const& dc);

    void OnMouseMove(wxMouseEvent const& event, wxDC const& dc);
    void OnPaint(wxDC& dc, bool drawPicAdjustDots, bool selected) const;

private:
    static constexpr auto kCircleSize = 6;

    wxImage mImage;
    wxBitmap mBitmap;
    wxPoint mBitmapPoint;

    // what type of background adjustments could we do
    enum class BackgroundAdjustType {
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
    };
    BackgroundAdjustType mBackgroundAdjustType;

    struct CalculateScaleAndMove {
    public:
        CalculateScaleAndMove(wxPoint startClick, wxRect rect, BackgroundAdjustType adjustType);
        
        wxRect operator()(wxCoord x, wxCoord y, wxRect wxRect);
        wxPoint mStartClick;
        wxRect mRect;
        float mAspectRatio;
        BackgroundAdjustType mAdjustType;
    };
    std::unique_ptr<CalculateScaleAndMove> mScaleAndMove;
};
