/*
 * BackgroundImages.cpp
 * Maintains the background image data
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

#include "BackgroundImages.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartImage.h"
#include "CalChartTypes.h"
#include <algorithm>

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
        kLast
    };
    using BackgroundAdjustTypeIterator = CalChart::Iterator<BackgroundAdjustType, BackgroundAdjustType::kUpperLeft, BackgroundAdjustType::kLowerRight>;
    BackgroundAdjustType mBackgroundAdjustType;
    BackgroundAdjustType WhereIsMouseClick(const wxMouseEvent& event, const wxDC& dc) const;

    static auto toOffsetX(BackgroundAdjustType where)
    {
        switch (where) {
        case BackgroundAdjustType::kUpperLeft:
        case BackgroundAdjustType::kLeft:
        case BackgroundAdjustType::kLowerLeft:
            return -1;
        case BackgroundAdjustType::kUpper:
        case BackgroundAdjustType::kMove:
        case BackgroundAdjustType::kLower:
            return 0;
        default:
            return 1;
        }
    }
    static auto toOffsetY(BackgroundAdjustType where)
    {
        switch (where) {
        case BackgroundAdjustType::kUpperLeft:
        case BackgroundAdjustType::kUpper:
        case BackgroundAdjustType::kUpperRight:
            return -1;
        case BackgroundAdjustType::kLeft:
        case BackgroundAdjustType::kMove:
        case BackgroundAdjustType::kRight:
            return 0;
        default:
            return 1;
        }
    }

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

BackgroundImage::BackgroundImage(const wxImage& image, int x, int y, int scaled_width, int scaled_height)
    : mImage(image)
    , mBitmapPoint(x, y)
    , // always adjust when we get created
    mBackgroundAdjustType(BackgroundAdjustType::kLast)
{
    mBitmap = wxBitmap(mImage.Scale(scaled_width, scaled_height, wxIMAGE_QUALITY_HIGH));
}

BackgroundImage::BackgroundAdjustType BackgroundImage::WhereIsMouseClick(const wxMouseEvent& event, const wxDC& dc) const
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    // where are we?
    auto bitmapSize = mBitmap.GetSize();
    auto middle = mBitmapPoint + bitmapSize / 2;
    for (auto where : BackgroundAdjustTypeIterator()) {
        if (where == BackgroundAdjustType::kMove && wxRect{ mBitmapPoint, mBitmap.GetSize() }.Contains(x, y)) {
            return where;
        }
        auto offsetX = toOffsetX(where);
        auto offsetY = toOffsetY(where);
        auto grabPoint = wxRect{
            middle.x + (offsetX * (bitmapSize.x / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            middle.y + (offsetY * (bitmapSize.y / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))) - dc.DeviceToLogicalXRel(kCircleSize),
            dc.DeviceToLogicalXRel(kCircleSize * 2),
            dc.DeviceToLogicalXRel(kCircleSize * 2)
        };

        if (grabPoint.Contains(x, y)) {
            return where;
        }
    }
    return BackgroundAdjustType::kLast;
}

bool BackgroundImage::MouseClickIsHit(const wxMouseEvent& event, const wxDC& dc) const
{
    return WhereIsMouseClick(event, dc) != BackgroundAdjustType::kLast;
}

void BackgroundImage::OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc)
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    auto where = WhereIsMouseClick(event, dc);
    if (where == BackgroundAdjustType::kLast) {
        return;
    }
    mBackgroundAdjustType = where;
    mScaleAndMove = std::make_unique<CalculateScaleAndMove>(wxPoint{ x, y }, wxRect{ mBitmapPoint, mBitmap.GetSize() }, mBackgroundAdjustType);
}

std::array<int, 4> BackgroundImage::OnMouseLeftUp(const wxMouseEvent&, const wxDC&)
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
        for (auto where : BackgroundAdjustTypeIterator()) {
            dc.SetBrush(*wxBLUE_BRUSH);
            if (where == BackgroundAdjustType::kMove) {
                continue;
            }
            auto offsetX = toOffsetX(where);
            auto offsetY = toOffsetY(where);
            if (mBackgroundAdjustType == where) {
                dc.SetBrush(*wxRED_BRUSH);
            }
            dc.DrawCircle(
                middle.x + (offsetX * (bitmapSize.x / 2 + dc.DeviceToLogicalXRel(kCircleSize / 3))),
                middle.y + (offsetY * (bitmapSize.y / 2 + dc.DeviceToLogicalYRel(kCircleSize / 3))),
                dc.DeviceToLogicalXRel(kCircleSize));
            if (selected && mBackgroundAdjustType != where) {
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

BackgroundImages::BackgroundImages() = default;
BackgroundImages::~BackgroundImages() = default;

void BackgroundImages::SetBackgroundImages(std::vector<CalChart::ImageInfo> const& images)
{
    mBackgroundImages.clear();
    mWhichBackgroundIndex = -1;
    for (auto&& image : images) {
        mBackgroundImages.emplace_back(wxCalChart::ConvertTowxImage(image.data), image.left, image.top, image.scaled_width, image.scaled_height);
    }
}

void BackgroundImages::OnPaint(wxDC& dc) const
{
    for (auto i = 0; i < static_cast<int>(mBackgroundImages.size()); ++i) {
        mBackgroundImages[i].OnPaint(dc, mAdjustBackgroundMode, mWhichBackgroundIndex == i);
    }
}

void BackgroundImages::OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc)
{
    if (!mAdjustBackgroundMode) {
        return;
    }
    mWhichBackgroundIndex = -1;
    for (auto i = 0; i < static_cast<int>(mBackgroundImages.size()); ++i) {
        if (mBackgroundImages[i].MouseClickIsHit(event, dc)) {
            mWhichBackgroundIndex = i;
        }
    }
    if (mWhichBackgroundIndex != -1) {
        mBackgroundImages[mWhichBackgroundIndex].OnMouseLeftDown(event, dc);
    }
}

std::optional<std::tuple<int, std::array<int, 4>>> BackgroundImages::OnMouseLeftUp(wxMouseEvent const& event, wxDC const& dc)
{
    if (!mAdjustBackgroundMode) {
        return {};
    }
    if (mWhichBackgroundIndex >= 0 && mWhichBackgroundIndex < static_cast<int>(mBackgroundImages.size())) {
        return { { mWhichBackgroundIndex, mBackgroundImages[mWhichBackgroundIndex].OnMouseLeftUp(event, dc) } };
    }
    return {};
}

void BackgroundImages::OnMouseMove(wxMouseEvent const& event, wxDC const& dc)
{
    if (!mAdjustBackgroundMode) {
        return;
    }
    if (mWhichBackgroundIndex >= 0 && mWhichBackgroundIndex < static_cast<int>(mBackgroundImages.size())) {
        mBackgroundImages[mWhichBackgroundIndex].OnMouseMove(event, dc);
    }
}
