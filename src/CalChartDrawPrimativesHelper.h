#pragma once
/*
 * CalChartDrawPrimativesHelper.h
 * Functions for converting CalChartDrawPrimatives to wxWidgets primatives.
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

/**
 * CalChart Draw Primatives to wxWidgets
 * Helper functions for creating wxWidgets from drawing primatives
 */

#include "CalChartDrawPrimatives.h"
#include "CalChartImage.h"
#include "CalChartSizes.h"
#include <regex>
#include <wx/brush.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/pen.h>

// this needs to be in the same namespace as wxSize, which would be the global namespace
inline auto operator<(wxSize const& p1, wxSize const& p2)
{
    return p1.x == p2.x ? p1.y < p2.y : p1.x < p2.y;
}

namespace wxCalChart {

inline auto make_wxSize(CalChart::Coord coord) { return wxSize{ coord.x, coord.y }; }

inline auto toColour(CalChart::Color::ColorRGB c) -> wxColour
{
    return { c.red, c.green, c.blue, c.alpha };
}

inline auto toColour(std::string const& str) -> wxColour
{
    return { wxString{ str.data() } };
}

inline auto toColour(CalChart::Color color) -> wxColour
{
    return std::visit(
        CalChart::overloaded{
            [](CalChart::Color::ColorRGB c) { return toColour(c); },
            [](std::string const& str) { return toColour(str); } },
        color.mColor);
}

inline auto toRGB(CalChart::Color color) -> std::tuple<uint8_t, uint8_t, uint8_t>
{
    return std::visit(
        CalChart::overloaded{
            [](CalChart::Color::ColorRGB c) { return std::tuple{ c.red, c.green, c.blue }; },
            [](std::string const& str) { auto colour = toColour(str); return std::tuple{ colour.Red(), colour.Green(), colour.Blue()}; } },
        color.mColor);
}

inline auto toBrush(CalChart::Color c)
{
    return *wxTheBrushList->FindOrCreateBrush(toColour(c));
}

inline auto toBrush(CalChart::Brush b)
{
    if (b.style == CalChart::Brush::Style::Transparent) {
        return *wxTRANSPARENT_BRUSH;
    }
    return *wxTheBrushList->FindOrCreateBrush(toColour(b.color));
}

inline auto toBrush(CalChart::BrushAndPen b)
{
    if (b.brushStyle == CalChart::Brush::Style::Transparent) {
        return *wxTRANSPARENT_BRUSH;
    }
    return *wxTheBrushList->FindOrCreateBrush(toColour(b.color));
}

inline auto toPen(CalChart::Pen p)
{
    return *wxThePenList->FindOrCreatePen(toColour(p.color), p.width);
}

inline auto toPen(CalChart::BrushAndPen b)
{
    if (b.brushStyle == CalChart::Brush::Style::Transparent) {
        return *wxTRANSPARENT_PEN;
    }
    auto penStyle = b.penStyle == CalChart::Pen::Style::ShortDash ? wxPENSTYLE_SHORT_DASH
                                                                  : wxPENSTYLE_SOLID;

    return *wxThePenList->FindOrCreatePen(toColour(b.color), b.width, penStyle);
}

inline auto toColor(wxColour const& c) -> CalChart::Color
{
    return CalChart::Color{ c.Red(), c.Green(), c.Blue(), c.Alpha() };
}

inline auto toColor(std::string_view s) -> CalChart::Color
{
    return CalChart::Color{ std::string{ s } };
}

inline auto toBrush(wxBrush const& b)
{
    return CalChart::Brush{ toColor(b.GetColour()), CalChart::Brush::Style::Solid };
}

inline auto toPen(wxPen const& p)
{
    auto penStyle = p.GetStyle() == wxPENSTYLE_SHORT_DASH
        ? CalChart::Pen::Style::ShortDash
        : CalChart::Pen::Style::Solid;
    return CalChart::Pen{ toColor(p.GetColour()), penStyle, p.GetWidth() };
}

inline auto toBrushAndPen(wxColour const& c, int width)
{
    return CalChart::BrushAndPen{ toColor(c), CalChart::Brush::Style::Solid, CalChart::Pen::Style::Solid, width };
}

inline auto toBrushAndPen(std::string_view s, int width)
{
    return CalChart::BrushAndPen{ toColor(s), CalChart::Brush::Style::Solid, CalChart::Pen::Style::Solid, width };
}

inline auto toBrushAndPen(wxPen const& p)
{
    return CalChart::BrushAndPen{ toColor(p.GetColour()), CalChart::Brush::Style::Solid, CalChart::Pen::Style::Solid, p.GetWidth() };
}

inline auto setBackground(wxDC& dc, CalChart::Color color)
{
    dc.SetBackground(wxCalChart::toBrush(color));
}

inline auto setBackground(wxDC& dc, CalChart::Brush brush)
{
    dc.SetBackground(wxCalChart::toBrush(brush));
}

inline auto setBackground(wxDC& dc, CalChart::BrushAndPen brushAndPen)
{
    dc.SetBackground(wxCalChart::toBrush(brushAndPen));
}

inline auto setBrush(wxDC& dc, CalChart::Color color)
{
    dc.SetBrush(wxCalChart::toBrush(color));
}

inline auto setBrush(wxDC& dc, CalChart::Brush brush)
{
    dc.SetBrush(wxCalChart::toBrush(brush));
}

inline auto setBrush(wxDC& dc, CalChart::BrushAndPen brushAndPen)
{
    dc.SetBrush(wxCalChart::toBrush(brushAndPen));
}

inline auto setPen(wxDC& dc, CalChart::Pen pen)
{
    dc.SetPen(wxCalChart::toPen(pen));
}

inline auto setPen(wxDC& dc, CalChart::BrushAndPen brushAndPen)
{
    dc.SetPen(wxCalChart::toPen(brushAndPen));
}

inline auto setBrushAndPen(wxDC& dc, CalChart::BrushAndPen brushAndPen)
{
    dc.SetBrush(wxCalChart::toBrush(brushAndPen));
    dc.SetPen(wxCalChart::toPen(brushAndPen));
}

inline auto setTextForeground(wxDC& dc, CalChart::BrushAndPen brushAndPen)
{
    dc.SetTextForeground(wxCalChart::toColour(brushAndPen.color));
}

inline auto setFont(wxDC& dc, CalChart::Font font)
{
    auto family = [font] {
        switch (font.family) {
        case CalChart::Font::Family::Swiss:
            return wxFONTFAMILY_SWISS;
        case CalChart::Font::Family::Roman:
            return wxFONTFAMILY_ROMAN;
        case CalChart::Font::Family::Modern:
            return wxFONTFAMILY_MODERN;
        }
    }();
    auto style = [font] {
        switch (font.style) {
        case CalChart::Font::Style::Normal:
            return wxFONTSTYLE_NORMAL;
        case CalChart::Font::Style::Italic:
            return wxFONTSTYLE_ITALIC;
        }
    }();
    auto weight = [font] {
        switch (font.weight) {
        case CalChart::Font::Weight::Normal:
            return wxFONTWEIGHT_NORMAL;
        case CalChart::Font::Weight::Bold:
            return wxFONTWEIGHT_BOLD;
        }
    }();

    auto size = wxSize{ 0, fDIP(font.size) };
    dc.SetFont(wxFont(size, family, style, weight));
}

inline auto ConvertToImageData(wxImage const& image) -> CalChart::ImageData
{
    auto width = image.GetWidth();
    auto height = image.GetHeight();
    auto data = std::vector<unsigned char>(width * height * 3);
    auto* d = image.GetData();
    std::copy(d, d + width * height * 3, data.data());
    auto alpha = std::vector<unsigned char>{};
    auto* a = image.GetAlpha();
    if (a) {
        alpha.resize(width * height);
        std::copy(a, a + width * height, alpha.data());
    }

    return { width, height, data, alpha };
}

inline auto ConvertToImageInfo(wxImage const& image, int x = 0, int y = 0) -> CalChart::ImageInfo
{
    auto width = image.GetWidth();
    auto height = image.GetHeight();
    return CalChart::ImageInfo{ x, y, width, height, ConvertToImageData(image) };
}

inline auto ConvertTowxImage(CalChart::ImageData const& image) -> wxImage
{
    auto data = image.data;
    if (image.alpha.size()) {
        auto alpha = image.alpha;
        return { image.image_width, image.image_height, data.data(), alpha.data(), true };
    }
    return { image.image_width, image.image_height, data.data(), true };
}
}
