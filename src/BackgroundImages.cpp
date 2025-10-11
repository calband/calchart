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
#include "CalChartDrawing.h"
#include "CalChartImage.h"
#include "CalChartRanges.h"
#include "CalChartTypes.h"
#include "CalChartUtils.h"
#include <algorithm>

class BackgroundImage {
public:
    explicit BackgroundImage(CalChart::ImageInfo const& image);

    [[nodiscard]] auto MouseClickIsHit(wxMouseEvent const& event, wxDC const& dc) const -> bool;
    void OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc);

    // returns left, top, width, height
    [[nodiscard]] auto OnMouseLeftUp(wxMouseEvent const& event, wxDC const& dc) -> std::array<int, 4>;

    void OnMouseMove(wxMouseEvent const& event, wxDC const& dc);
    void OnPaint(wxDC& dc, bool drawPicAdjustDots, bool selected) const;

private:
    static constexpr auto kCircleSize = 6;

    CalChart::Coord mPosition{};
    CalChart::Coord mScaledSize{};
    wxImage mRawImage;
    wxBitmap mBitmap;

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
        kLowerRight
    };
    using BackgroundAdjustTypeIterator = CalChart::Iterator<BackgroundAdjustType, BackgroundAdjustType::kUpperLeft, BackgroundAdjustType::kLowerRight>;
    std::optional<BackgroundAdjustType> mBackgroundAdjustType = std::nullopt;
    [[nodiscard]] auto WhereIsMouseClick(const wxMouseEvent& event, const wxDC& dc) const -> std::optional<BackgroundAdjustType>;

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
    std::optional<CalculateScaleAndMove> mScaleAndMove;
};

BackgroundImage::BackgroundImage(CalChart::ImageInfo const& image)
    : mPosition{ image.left, image.top }
    , mScaledSize{ image.scaledWidth, image.scaledHeight }
    , mRawImage{ wxCalChart::towxImage(image.data) }
    , mBitmap{ mRawImage.Scale(mScaledSize.x, mScaledSize.y, wxIMAGE_QUALITY_HIGH) }
{
}

auto BackgroundImage::WhereIsMouseClick(const wxMouseEvent& event, const wxDC& dc) const -> std::optional<BackgroundAdjustType>
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    // where are we?
    auto bitmapSize = wxCalChart::toSize(mScaledSize);
    auto middle = wxCalChart::toPoint(mPosition) + bitmapSize / 2;
    for (auto where : BackgroundAdjustTypeIterator()) {
        if (where == BackgroundAdjustType::kMove && wxRect{ wxCalChart::toPoint(mPosition), wxCalChart::toSize(mScaledSize) }.Contains(x, y)) {
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
    return std::nullopt;
}

auto BackgroundImage::MouseClickIsHit(const wxMouseEvent& event, const wxDC& dc) const -> bool
{
    return WhereIsMouseClick(event, dc).has_value();
}

void BackgroundImage::OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc)
{
    auto point = event.GetPosition();
    auto x = dc.DeviceToLogicalX(point.x);
    auto y = dc.DeviceToLogicalY(point.y);

    auto where = WhereIsMouseClick(event, dc);
    if (!where.has_value()) {
        return;
    }
    mBackgroundAdjustType = where;
    mScaleAndMove = { wxPoint{ x, y }, wxRect{ wxCalChart::toPoint(mPosition), wxCalChart::toSize(mScaledSize) }, *mBackgroundAdjustType };
}

std::array<int, 4> BackgroundImage::OnMouseLeftUp(const wxMouseEvent&, const wxDC&)
{
    if (mScaleAndMove) {
        // done moving, lock down the picture and make it pretty:
        mBitmap = wxBitmap(mRawImage.Scale(mBitmap.GetWidth(), mBitmap.GetHeight(), wxIMAGE_QUALITY_HIGH));
        auto [width, height] = wxCalChart::toSize(mScaledSize);
        auto [x, y] = wxCalChart::toPoint(mPosition);
        std::array<int, 4> data{ { x, y, width, height } };
        mScaleAndMove.reset();
        mBackgroundAdjustType = std::nullopt;
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
        auto rect = (*mScaleAndMove)(x, y, { wxCalChart::toPoint(mPosition), wxCalChart::toSize(mScaledSize) });
        mPosition = wxCalChart::toCoord(rect.GetPosition());
        mScaledSize = wxCalChart::toCoord(rect.GetSize());
        mBitmap = wxBitmap(mRawImage.Scale(mScaledSize.x, mScaledSize.y));
    }
}

void BackgroundImage::OnPaint(wxDC& dc, bool drawPicAdjustDots, bool selected) const
{
    auto drawCmds = std::vector<CalChart::Draw::DrawCommand>{
        CalChart::Draw::Image{ mPosition, std::make_shared<wxCalChart::BitmapHolder>(mBitmap) },
    };

    if (drawPicAdjustDots) {
        auto brush = selected ? wxCalChart::toBrush(*wxWHITE_BRUSH) : wxCalChart::toBrush(*wxBLUE_BRUSH);
        auto middle1 = mPosition + mScaledSize / 2;
        CalChart::append(drawCmds,
            CalChart::Draw::withBrush(
                brush,
                CalChart::Draw::withPen(
                    wxCalChart::toPen(*wxBLUE_PEN).withWidth(4),
                    CalChart::Ranges::ToVector<CalChart::Draw::DrawCommand>(
                        BackgroundAdjustTypeIterator()
                        | std::views::filter([](auto where) {
                              return where != BackgroundAdjustType::kMove;
                          })
                        | std::views::transform([this, middle1](auto where) {
                              auto offsetX = toOffsetX(where);
                              auto offsetY = toOffsetY(where);
                              return CalChart::Draw::Circle{
                                  middle1.x + offsetX * (mScaledSize.x / 2),
                                  middle1.y + offsetY * (mScaledSize.y / 2),
                                  kCircleSize
                              };
                          })))));
        if (mBackgroundAdjustType.has_value() && *mBackgroundAdjustType != BackgroundAdjustType::kMove) {
            auto offsetX = toOffsetX(*mBackgroundAdjustType);
            auto offsetY = toOffsetY(*mBackgroundAdjustType);
            CalChart::append(drawCmds,
                CalChart::Draw::withBrush(
                    wxCalChart::toBrush(*wxRED_BRUSH),
                    CalChart::Draw::withPen(
                        wxCalChart::toPen(*wxBLUE_PEN).withWidth(4),
                        CalChart::Draw::Circle{
                            middle1.x + offsetX * (mScaledSize.x / 2),
                            middle1.y + offsetY * (mScaledSize.y / 2),
                            kCircleSize })));
        }
    }
    wxCalChart::Draw::DrawCommandList(dc,
        drawCmds);
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
    auto previousSize = mBackgroundImages.size();
    mBackgroundImages = CalChart::Ranges::ToVector<BackgroundImage>(
        images | std::views::transform([](auto&& image) { return BackgroundImage{ image }; }));
    if (mBackgroundImages.size() != previousSize) {
        mWhichBackgroundIndex = std::nullopt;
    }
}

void BackgroundImages::OnPaint(wxDC& dc) const
{
    for (auto&& [i, backgroundImages] : CalChart::Ranges::enumerate_view(mBackgroundImages)) {
        backgroundImages.OnPaint(dc, mAdjustBackgroundMode, mWhichBackgroundIndex.has_value() ? *mWhichBackgroundIndex == static_cast<size_t>(i) : false);
    }
}

void BackgroundImages::OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc)
{
    if (!mAdjustBackgroundMode) {
        return;
    }
    mWhichBackgroundIndex = std::nullopt;
    if (auto iter = std::find_if(
            mBackgroundImages.begin(),
            mBackgroundImages.end(),
            [&event, &dc](auto&& backgroundImage) {
                return backgroundImage.MouseClickIsHit(event, dc);
            });
        iter != mBackgroundImages.end()) {
        mWhichBackgroundIndex = std::distance(mBackgroundImages.begin(), iter);
    }
    if (mWhichBackgroundIndex.has_value()) {
        mBackgroundImages[*mWhichBackgroundIndex].OnMouseLeftDown(event, dc);
    }
}

std::optional<std::tuple<int, std::array<int, 4>>> BackgroundImages::OnMouseLeftUp(wxMouseEvent const& event, wxDC const& dc)
{
    if (!mAdjustBackgroundMode) {
        return {};
    }
    if (mWhichBackgroundIndex.has_value()) {
        return { { *mWhichBackgroundIndex, mBackgroundImages[*mWhichBackgroundIndex].OnMouseLeftUp(event, dc) } };
    }
    return {};
}

void BackgroundImages::OnMouseMove(wxMouseEvent const& event, wxDC const& dc)
{
    if (!mAdjustBackgroundMode) {
        return;
    }
    if (mWhichBackgroundIndex.has_value()) {
        mBackgroundImages[*mWhichBackgroundIndex].OnMouseMove(event, dc);
    }
}
