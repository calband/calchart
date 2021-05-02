/*
 * BackgroundImage.cpp
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

#include "BackgroundImage.h"
#include <algorithm>

BackgroundImage::BackgroundImage(const wxImage& image, int x, int y, int scaled_width, int scaled_height)
    : mImage(image)
    , mBitmapPoint(x, y)
    , // always adjust when we get created
    mBackgroundAdjustType(BackgroundAdjustType::kLast)
{
    mBitmap = wxBitmap(mImage.Scale(scaled_width, scaled_height, wxIMAGE_QUALITY_HIGH));
}

template<typename E>
constexpr auto toUType(E enumerator)
{
    return static_cast<std::underlying_type_t<E>>(enumerator);
}

bool BackgroundImage::MouseClickIsHit(const wxMouseEvent& event, const wxDC& dc) const
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    // where are we?
    auto bitmapSize = mBitmap.GetSize();
    auto middle = mBitmapPoint + bitmapSize / 2;
    for (auto where = toUType(BackgroundAdjustType::kUpperLeft); where < toUType(BackgroundAdjustType::kLast); ++where) {
        if (where == toUType(BackgroundAdjustType::kMove) && wxRect{mBitmapPoint, mBitmap.GetSize()}.Contains(x, y)) {
            return true;
        }
        auto offsetX = (where % 3) - 1;
        auto offsetY = (where / 3) - 1;
        auto grabPoint = wxRect {
            middle.x + (offsetX * (bitmapSize.x / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            middle.y + (offsetY * (bitmapSize.y / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            dc.DeviceToLogicalXRel(kCircleSize * 2),
            dc.DeviceToLogicalXRel(kCircleSize * 2)
        };

        if (grabPoint.Contains(x, y)) {
            return true;
        }
    }
    return false;
}

void BackgroundImage::OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc)
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    // where are we?
    auto bitmapSize = mBitmap.GetSize();
    auto middle = mBitmapPoint + bitmapSize / 2;
    auto where = toUType(BackgroundAdjustType::kUpperLeft);
    for (; where < toUType(BackgroundAdjustType::kLast); ++where) {
        if (where == toUType(BackgroundAdjustType::kMove)) {
            continue;
        }
        auto offsetX = (where % 3) - 1;
        auto offsetY = (where / 3) - 1;
        auto grabPoint = wxRect{
            middle.x + (offsetX * (bitmapSize.x / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            middle.y + (offsetY * (bitmapSize.y / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            dc.DeviceToLogicalXRel(kCircleSize * 2),
            dc.DeviceToLogicalXRel(kCircleSize * 2)
        };

        if (grabPoint.Contains(x, y)) {
            break;
        }
    }
    mBackgroundAdjustType = static_cast<BackgroundAdjustType>(where);
    if (mBackgroundAdjustType == BackgroundAdjustType::kLast) {
        if (wxRect{mBitmapPoint, wxSize{ mBitmap.GetWidth(), mBitmap.GetHeight() } }.Contains(x, y)) {
            mBackgroundAdjustType = BackgroundAdjustType::kMove;
        }
    }
    if (mBackgroundAdjustType != BackgroundAdjustType::kLast) {
        mScaleAndMove = std::make_unique<CalculateScaleAndMove>(wxPoint{x, y}, wxRect{ mBitmapPoint, mBitmap.GetSize() }, mBackgroundAdjustType);
    }
}

std::array<int, 4> BackgroundImage::OnMouseLeftUp(const wxMouseEvent& event, const wxDC& dc)
{
    if (mScaleAndMove) {
        // done moving, lock down the picture and make it pretty:
        mBitmap = wxBitmap(mImage.Scale(mBitmap.GetWidth(), mBitmap.GetHeight(), wxIMAGE_QUALITY_HIGH));
        std::array<int, 4> data{ { mBitmapPoint.x, mBitmapPoint.y, mBitmap.GetWidth(), mBitmap.GetHeight() } };
        mScaleAndMove.reset();
        mBackgroundAdjustType = BackgroundAdjustType::kLast;
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
        auto rect = (*mScaleAndMove)(x, y, { mBitmapPoint, mBitmap.GetSize() });
        mBitmapPoint = rect.GetPosition();
        mBitmap = wxBitmap(mImage.Scale(rect.width, rect.height));
    }
}

void BackgroundImage::OnPaint(wxDC& dc, bool drawPicAdjustDots, bool selected) const
{
    dc.DrawBitmap(mBitmap, mBitmapPoint.x, mBitmapPoint.y, true);
    if (drawPicAdjustDots) {
        // draw guide dots
        auto bitmapSize = mBitmap.GetSize();
        auto middle = mBitmapPoint + bitmapSize / 2;
        dc.SetBrush(*wxBLUE_BRUSH);
        dc.SetPen(*wxBLUE_PEN);
        for (auto where = toUType(BackgroundAdjustType::kUpperLeft); where < toUType(BackgroundAdjustType::kLast); ++where) {
            dc.SetBrush(*wxBLUE_BRUSH);
            if (where == toUType(BackgroundAdjustType::kMove)) {
                continue;
            }
            auto offsetX = (where % 3) - 1;
            auto offsetY = (where / 3) - 1;
            if (toUType(mBackgroundAdjustType) == where) {
                dc.SetBrush(*wxRED_BRUSH);
            }
            dc.DrawCircle(
                middle.x + (offsetX * (bitmapSize.x / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))),
                middle.y + (offsetY * (bitmapSize.y / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))),
                dc.DeviceToLogicalXRel(kCircleSize));
            if (selected && toUType(mBackgroundAdjustType) != where) {
                dc.SetBrush(*wxWHITE_BRUSH);
                dc.DrawCircle(
                    middle.x + (offsetX * (bitmapSize.x / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))),
                    middle.y + (offsetY * (bitmapSize.y / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))),
                    dc.DeviceToLogicalXRel(kCircleSize * 0.75));
            }
        }
    }
}

BackgroundImage::CalculateScaleAndMove::CalculateScaleAndMove(wxPoint startClick, wxRect rect, BackgroundAdjustType adjustType)
    : mStartClick(startClick)
    , mRect(rect)
    , mAspectRatio(rect.width / static_cast<float>(rect.height))
    , mAdjustType(adjustType)
{
}

wxRect BackgroundImage::CalculateScaleAndMove::operator()(wxCoord x, wxCoord y, wxRect rect)
{
    switch (mAdjustType) {
        case BackgroundAdjustType::kUpper:
        // for upper: make sure we never go lower than the bottom
        // don't modify where the top is, but adjust the heigth
        if (y < mRect.GetBottom()) {
            rect.y = y;
            rect.height = mRect.GetBottom() - y;
        }
            return rect;
        case BackgroundAdjustType::kLower:
        // for lower: make sure we never go higher than the top
        // don't modify where the top is, but adjust the heigth
            if (rect.y < y) {
            rect.height = y - rect.y;
            }
            return rect;
        case BackgroundAdjustType::kLeft:
        if (x < mRect.GetRight()) {
            rect.x = x;
            rect.width = mRect.GetRight() - x;
        }
            return rect;
        case BackgroundAdjustType::kRight:
            if (rect.x < x) {
                rect.width = x - rect.x;
            }
            return rect;
    // we should keep the aspect ratio
        case BackgroundAdjustType::kUpperLeft:
        x = std::max<wxCoord>(x, mRect.GetRight() - std::abs(y - mRect.GetBottom()) * mAspectRatio / 1.0);
        y = std::max<wxCoord>(y, mRect.GetBottom() - std::abs(x - mRect.GetRight()) * 1.0 / mAspectRatio);
        if (x < mRect.GetRight()) {
            rect.x = x;
            rect.width = mRect.GetRight() - x;
        }
        if (y < mRect.GetBottom()) {
            rect.y = y;
            rect.height = mRect.GetBottom() - y;
        }
            return rect;
        case BackgroundAdjustType::kUpperRight:
        x = std::min<wxCoord>(x, mRect.GetLeft() + std::abs(y - mRect.GetBottom()) * mAspectRatio / 1.0);
        y = std::max<wxCoord>(y, mRect.GetBottom() - std::abs(x - mRect.GetLeft()) * 1.0 / mAspectRatio);
        if (rect.x < x)
            rect.width = x - rect.x;
        if (y < mRect.GetBottom()) {
            rect.y = y;
            rect.height = mRect.GetBottom() - y;
        }
            return rect;
        case BackgroundAdjustType::kLowerLeft:
        x = std::max<wxCoord>(x, mRect.GetRight() - std::abs(y - mRect.GetTop()) * mAspectRatio / 1.0);
        y = std::min<wxCoord>(y, mRect.GetTop() + std::abs(x - mRect.GetRight()) * 1.0 / mAspectRatio);
        if (x < mRect.GetRight()) {
            rect.x = x;
            rect.width = mRect.GetRight() - x;
        }
            if (rect.y < y) {
                rect.height = y - rect.y;
            }
            return rect;
        case BackgroundAdjustType::kLowerRight:
        x = std::min<wxCoord>(x, mRect.GetLeft() + std::abs(y - mRect.GetTop()) * mAspectRatio / 1.0);
        y = std::min<wxCoord>(y, mRect.GetTop() + std::abs(x - mRect.GetLeft()) * 1.0 / mAspectRatio);
        if (rect.x < x)
            rect.width = x - rect.x;
            if (rect.y < y) {
                rect.height = y - rect.y;
            }
            return rect;
        case BackgroundAdjustType::kMove:
            rect.x = mRect.GetLeft() + (x - mStartClick.x);
            rect.y = mRect.GetTop() + (y - mStartClick.y);
            return rect;
    default:
            return rect;
    }
}
