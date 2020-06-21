#pragma once
/*
 * CalChartSizes.h
 * Central place for all UI Sizes
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

#include <wx/wx.h>

#include "cc_coord.h"

template <typename T>
static auto fDIP(T const& t) { return wxWindow::FromDIP(t, nullptr); }

template <typename T>
static auto tDIP(T const& t) { return wxWindow::ToDIP(t, nullptr); }

static inline auto fDIP(CalChart::Coord const& t) { return wxWindow::FromDIP(wxSize{ t.x, t.y }, nullptr); }

// Generic sizes
static inline auto GetBitmapButtonSize() { return fDIP(wxSize{ 16, 16 }); };
static inline auto ScaleButtonBitmap(wxBitmap const& data) { return data.ConvertToImage().Scale(GetBitmapButtonSize().x, GetBitmapButtonSize().y); };

// Top frame related sizes
static inline auto GetLogoSize() { return fDIP(wxSize(150, 150)); }
static inline auto GetLogoLineSize() { return fDIP(150); }
static inline auto GetTitleFontSize() { return 16; }
static inline auto GetSubTitleFontSize() { return 14; }
static inline auto GetSubSubTitleFontSize() { return 11; }

// Generic color box size that shows up in color palette, etc
static inline auto GetColorBoxSize() { return fDIP(wxSize(16, 16)); }

// size of ColorSetupCanvas
static inline auto GetColorSetupCanvas() { return fDIP(wxSize(640, 240)); }

// Animation Canvas values
static inline auto GetAnimationCanvasMinY() { return fDIP(100); }
static inline auto GetAnimationViewTempoSpinnerMinX() { return fDIP(48); }
static inline auto GetAnimationViewBeatSliderInNonMinimode() { return fDIP(800); }

// color palette Canvas values
static inline auto GetColorPaletteBoxSize() { return fDIP(16); }
static inline auto GetColorPaletteBoxBorderSize() { return fDIP(2); }
static inline auto GetColorPaletteBoxRadiusSize() { return fDIP(4); }

// CalChartFrame sizes that shows up in color palette, etc
static inline auto GetToolBarFontSize() { return 12; }
static inline auto GetContinuityBrowserConstructSize() { return wxSize{ fDIP(180), -1 }; }
static inline auto GetContinuityBrowserSize() { return fDIP(wxSize{ 180, 90 }); }
static inline auto GetFieldThumbnailBrowserConstructSize() { return wxSize{ fDIP(180), -1 }; }
static inline auto GetFieldThumbnailBrowserSize() { return fDIP(wxSize{ 180, 360 }); }
static inline auto GetAnimationConstructSize() { return wxSize{ fDIP(180), -1 }; }
static inline auto GetAnimationSize() { return fDIP(wxSize{ 180, 360 }); }
static inline auto GetAnimationErrorsConstructSize() { return wxSize{ fDIP(180), -1 }; }
static inline auto GetAnimationErrorsSize() { return fDIP(wxSize{ 180, 360 }); }
static inline auto GetPrintContinuityConstructSize() { return wxSize{ fDIP(180), -1 }; }
static inline auto GetPrintContinuitySize() { return fDIP(wxSize{ 180, 360 }); }

// Field Thumbnail sizes
static inline auto GetThumbnailFontSize() { return 16; };

// toolbar sizers
static inline auto GetToolBarControlsPadding() { return fDIP(32); }
static inline auto GetToolBarControlsZoomSize() { return fDIP(64); }
