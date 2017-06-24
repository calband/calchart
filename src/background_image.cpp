/*
 * main_ui.cpp
 * Handle Background Image
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

#include "background_image.h"
#include <algorithm>

BackgroundImage::BackgroundImage(const wxImage& image, int x, int y, int scaled_width, int scaled_height)
    : mImage(image)
    , mBitmapX(x)
    , mBitmapY(y)
    , // always adjust when we get created
    mBackgroundAdjustType(kLast)
{
    mBitmap = wxBitmap(mImage.Scale(scaled_width, scaled_height, wxIMAGE_QUALITY_HIGH));
}

bool BackgroundImage::MouseClickIsHit(const wxMouseEvent& event,
    const wxDC& dc) const
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    // where are we?
    int height = mBitmap.GetHeight();
    int width = mBitmap.GetWidth();
    int middleX = mBitmapX + width / 2;
    int middleY = mBitmapY + height / 2;
    int where;
    for (where = kUpperLeft; where < kLast; ++where) {
        if (where == kMove) {
            wxRect bitmapSquare(wxPoint(mBitmapX, mBitmapY),
                wxSize(mBitmap.GetWidth(), mBitmap.GetHeight()));
            if (bitmapSquare.Contains(x, y)) {
                return true;
            }
        }
        int offsetX = (where % 3) - 1;
        int offsetY = (where / 3) - 1;
        wxRect grabPoint(
            middleX + (offsetX * (width / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            middleY + (offsetY * (height / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            dc.DeviceToLogicalXRel(kCircleSize * 2),
            dc.DeviceToLogicalXRel(kCircleSize * 2));

        if (grabPoint.Contains(x, y)) {
            return true;
        }
    }
    return false;
}

void BackgroundImage::OnMouseLeftDown(const wxMouseEvent& event,
    const wxDC& dc)
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    // where are we?
    int height = mBitmap.GetHeight();
    int width = mBitmap.GetWidth();
    int middleX = mBitmapX + width / 2;
    int middleY = mBitmapY + height / 2;
    int where;
    for (where = kUpperLeft; where < kLast; ++where) {
        if (where == kMove)
            continue;
        int offsetX = (where % 3) - 1;
        int offsetY = (where / 3) - 1;
        wxRect grabPoint(
            middleX + (offsetX * (width / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            middleY + (offsetY * (height / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            dc.DeviceToLogicalXRel(kCircleSize * 2),
            dc.DeviceToLogicalXRel(kCircleSize * 2));

        if (grabPoint.Contains(x, y)) {
            break;
        }
    }
    mBackgroundAdjustType = static_cast<eBackgroundAdjustType>(where);
    if (mBackgroundAdjustType == kLast) {
        wxRect bitmapSquare(wxPoint(mBitmapX, mBitmapY),
            wxSize(mBitmap.GetWidth(), mBitmap.GetHeight()));
        if (bitmapSquare.Contains(x, y)) {
            mBackgroundAdjustType = kMove;
        }
    }
    if (mBackgroundAdjustType != kLast) {
        mScaleAndMove.reset(new CalculateScaleAndMove(
            wxPoint(x, y), mBitmapX, mBitmapY, mBitmap.GetWidth(),
            mBitmap.GetHeight(), mBackgroundAdjustType));
    }
}

std::array<int, 4> BackgroundImage::OnMouseLeftUp(const wxMouseEvent& event, const wxDC& dc)
{
    if (mScaleAndMove) {
        // done moving, lock down the picture and make it pretty:
        mBitmap = wxBitmap(mImage.Scale(mBitmap.GetWidth(), mBitmap.GetHeight(),
            wxIMAGE_QUALITY_HIGH));
        std::array<int, 4> data{ { mBitmapX, mBitmapY, mBitmap.GetWidth(), mBitmap.GetHeight() } };
        mScaleAndMove.reset();
        mBackgroundAdjustType = kLast;
        return data;
    }
    return { { 0, 0, 0, 0 } };
}

void BackgroundImage::OnMouseMove(const wxMouseEvent& event, const wxDC& dc)
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    if (event.Dragging() && event.LeftIsDown() && mScaleAndMove) {
        wxCoord width = mBitmap.GetWidth();
        wxCoord height = mBitmap.GetHeight();
        (*mScaleAndMove)(x, y, mBitmapX, mBitmapY, width, height);
        mBitmap = wxBitmap(mImage.Scale(width, height));
    }
}

void BackgroundImage::OnPaint(wxDC& dc, bool drawPicAdjustDots, bool selected) const
{
    dc.DrawBitmap(mBitmap, mBitmapX, mBitmapY, true);
    if (drawPicAdjustDots) {
        // draw guide dots
        int height = mBitmap.GetHeight();
        int width = mBitmap.GetWidth();
        int middleX = mBitmapX + width / 2;
        int middleY = mBitmapY + height / 2;
        dc.SetBrush(*wxBLUE_BRUSH);
        dc.SetPen(*wxBLUE_PEN);
        for (int where = kUpperLeft; where < kLast; ++where) {
            dc.SetBrush(*wxBLUE_BRUSH);
            if (where == kMove)
                continue;
            int offsetX = (where % 3) - 1;
            int offsetY = (where / 3) - 1;
            if (mBackgroundAdjustType == where) {
                dc.SetBrush(*wxRED_BRUSH);
            }
            dc.DrawCircle(
                middleX + (offsetX * (width / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))),
                middleY + (offsetY * (height / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))),
                dc.DeviceToLogicalXRel(kCircleSize));
            if (selected && mBackgroundAdjustType != where) {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.DrawCircle(
                    middleX + (offsetX * (width / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))),
                    middleY + (offsetY * (height / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))),
                    dc.DeviceToLogicalXRel(kCircleSize * 0.75));
            }
        }
    }
}

BackgroundImage::CalculateScaleAndMove::CalculateScaleAndMove(
    const wxPoint& startClick, wxCoord x, wxCoord y, wxCoord width, wxCoord height,
    eBackgroundAdjustType adjustType)
    : mStartClick(startClick)
    , mLeft(x)
    , mTop(y)
    , mRight(x + width)
    , mBottom(y + height)
    , mAspectRatio(width / static_cast<float>(height))
    , mAdjustType(adjustType)
{
}

void BackgroundImage::CalculateScaleAndMove::
operator()(wxCoord x, wxCoord y, wxCoord& topX, wxCoord& topY, wxCoord& width,
    wxCoord& height)
{
    switch (mAdjustType) {
    case kUpper:
        // for upper: make sure we never go lower than the bottom
        // don't modify where the top is, but adjust the heigth
        if (y < mBottom) {
            topY = y;
            height = mBottom - y;
        }
        break;
    case kLower:
        // for lower: make sure we never go higher than the top
        // don't modify where the top is, but adjust the heigth
        if (topY < y)
            height = y - topY;
        break;
    case kLeft:
        if (x < mRight) {
            topX = x;
            width = mRight - x;
        }
        break;
    case kRight:
        if (topX < x)
            width = x - topX;
        break;
    // we should keep the aspect ratio
    case kUpperLeft:
        x = std::max<wxCoord>(x, mRight - std::abs(y - mBottom) * mAspectRatio / 1.0);
        y = std::max<wxCoord>(y, mBottom - std::abs(x - mRight) * 1.0 / mAspectRatio);
        if (x < mRight) {
            topX = x;
            width = mRight - x;
        }
        if (y < mBottom) {
            topY = y;
            height = mBottom - y;
        }
        break;
    case kUpperRight:
        x = std::min<wxCoord>(x, mLeft + std::abs(y - mBottom) * mAspectRatio / 1.0);
        y = std::max<wxCoord>(y, mBottom - std::abs(x - mLeft) * 1.0 / mAspectRatio);
        if (topX < x)
            width = x - topX;
        if (y < mBottom) {
            topY = y;
            height = mBottom - y;
        }
        break;
    case kLowerLeft:
        x = std::max<wxCoord>(x, mRight - std::abs(y - mTop) * mAspectRatio / 1.0);
        y = std::min<wxCoord>(y, mTop + std::abs(x - mRight) * 1.0 / mAspectRatio);
        if (x < mRight) {
            topX = x;
            width = mRight - x;
        }
        if (topY < y)
            height = y - topY;
        break;
    case kLowerRight:
        x = std::min<wxCoord>(x, mLeft + std::abs(y - mTop) * mAspectRatio / 1.0);
        y = std::min<wxCoord>(y, mTop + std::abs(x - mLeft) * 1.0 / mAspectRatio);
        if (topX < x)
            width = x - topX;
        if (topY < y)
            height = y - topY;
        break;
    case kMove:
        topX = mLeft + (x - mStartClick.x);
        topY = mTop + (y - mStartClick.y);
        break;
    default:
        break;
    }
}
