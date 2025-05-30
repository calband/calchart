#pragma once
/*
 * BackgroundImage.h
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

#include <array>
#include <memory>
#include <optional>
#include <wx/dc.h>
#include <wx/event.h>

namespace CalChart {
struct ImageInfo;
}

class BackgroundImage;

class BackgroundImages {
public:
    BackgroundImages();
    ~BackgroundImages();

    void SetBackgroundImages(std::vector<CalChart::ImageInfo> const& images);

    [[nodiscard]] auto GetAdjustBackgroundMode() const { return mAdjustBackgroundMode; }
    void SetAdjustBackgroundMode(bool adjustBackgroundMode) { mAdjustBackgroundMode = adjustBackgroundMode; }

    [[nodiscard]] auto GetCurrentIndex() const { return mWhichBackgroundIndex; }

    void OnMouseLeftDown(wxMouseEvent const& event, wxDC const& dc);
    [[nodiscard]] auto OnMouseLeftUp(wxMouseEvent const& event, wxDC const& dc) -> std::optional<std::tuple<int, std::array<int, 4>>>;
    void OnMouseMove(wxMouseEvent const& event, wxDC const& dc);

    void OnPaint(wxDC& dc) const;

private:
    std::vector<BackgroundImage> mBackgroundImages;
    bool mAdjustBackgroundMode{};
    std::optional<std::size_t> mWhichBackgroundIndex = std::nullopt;
};
