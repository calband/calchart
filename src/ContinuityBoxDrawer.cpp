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

#include "ContinuityBoxDrawer.h"
#include "CalChartConfiguration.h"
#include "CalChartContinuityToken.h"
#include "basic_ui.h"

#include <regex>

// Draw each ContToken and then all the children at the right location.
// Also handle
class ContinuityBoxSubPartDrawer {
public:
    ContinuityBoxSubPartDrawer(CalChart::DrawableCont const& proc);
    int GetTextBoxSize(wxDC const& dc, CalChartConfiguration const& config) const;
    void DrawCell(wxDC& dc, CalChartConfiguration const& config, void const* highlight, int x_start, int y_start) const;
    auto GetLastTextBoxSize() const { return std::pair<wxPoint, wxSize>(mRectStart, mRectSize); }
    void OnClick(wxPoint const& point, std::function<void(CalChart::DrawableCont const&)> if_hit) const;

private:
    void DrawProcCellBox(wxDC& dc, CalChartConfiguration const& config, void const* highlight, int x_start, int y_start) const;

    CalChart::DrawableCont mDrawCont;
    std::vector<ContinuityBoxSubPartDrawer> mChildren;
    mutable wxPoint mRectStart;
    mutable wxSize mRectSize;
};

auto GetBrush(CalChart::ContType contType, CalChartConfiguration const& config)
{
    switch (contType) {
    case CalChart::ContType::procedure:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_PROC).first;
    case CalChart::ContType::value:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_VALUE).first;
    case CalChart::ContType::function:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_FUNCTION).first;
    case CalChart::ContType::direction:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_DIRECTION).first;
    case CalChart::ContType::steptype:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_STEPTYPE).first;
    case CalChart::ContType::point:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_POINT).first;
    case CalChart::ContType::unset:
        return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_UNSET).first;
    }
    return config.Get_ContCellBrushAndPen(COLOR_CONTCELLS_PROC).first;
}

ContinuityBoxSubPartDrawer::ContinuityBoxSubPartDrawer(CalChart::DrawableCont const& proc)
    : mDrawCont(proc)
{
    std::copy(mDrawCont.args.begin(), mDrawCont.args.end(), std::back_inserter(mChildren));
}

int ContinuityBoxSubPartDrawer::GetTextBoxSize(wxDC const& dc, CalChartConfiguration const& config) const
{
    auto total_size = 0;
    auto text_padding = fDIP(config.Get_ContCellTextPadding());

    auto format_sub_size = dc.GetTextExtent("%@").x;
    for (auto&& i : mChildren) {
        total_size += i.GetTextBoxSize(dc, config) + text_padding * 2;
    }
    total_size += dc.GetTextExtent(config.Get_ContCellLongForm() ? mDrawCont.description : mDrawCont.short_description).x;
    total_size -= format_sub_size * mDrawCont.args.size();

    return total_size;
}

void ContinuityBoxSubPartDrawer::DrawProcCellBox(wxDC& dc, CalChartConfiguration const& config, void const* highlight, int x_start, int y_start) const
{
    auto text_padding = fDIP(config.Get_ContCellTextPadding());
    auto rounding = fDIP(config.Get_ContCellRounding());
    auto box_size_y = fDIP(config.Get_ContCellFontSize()) + 2 * text_padding;
    // first draw the box of the total size
    auto box_size_x = GetTextBoxSize(dc, config) + 2 * text_padding;
    {
        auto currentPen = dc.GetPen();
        auto restore = SaveAndRestoreBrushAndPen(dc);
        // fill with the color for this continuity
        dc.SetBrush(GetBrush(mDrawCont.type, config));
        if (mDrawCont.type == CalChart::ContType::unset) {
            dc.SetPen(*wxBLACK_DASHED_PEN);
        } else {
            dc.SetPen(*wxBLACK_PEN);
        }
        if (mDrawCont.self_ptr == highlight) {
            dc.SetPen(wxPen(wxColour(wxT("PINK")), 3));
        }
        mRectStart = wxPoint(x_start, y_start);
        mRectSize = wxSize(box_size_x, box_size_y);
        dc.DrawRoundedRectangle(mRectStart, mRectSize, rounding);
        dc.SetPen(currentPen);
    }
    auto AfterPen = dc.GetPen();

    // then render everything from left to right, text, then descend, then text...
    const static std::regex format_substr("%@");
    std::smatch format_match;
    auto string_to_print = config.Get_ContCellLongForm() ? mDrawCont.description : mDrawCont.short_description;
    auto count = 0u;
    auto textPoint = wxPoint{ x_start + text_padding, y_start + text_padding };
    while (regex_search(string_to_print, format_match, format_substr)) {
        auto t_str_to_print = wxString(format_match.prefix());
        dc.DrawText(t_str_to_print, textPoint);
        textPoint.x += dc.GetTextExtent(t_str_to_print).x;
        mChildren.at(count).DrawProcCellBox(dc, config, highlight, textPoint.x, y_start);
        textPoint.x += mChildren.at(count).GetTextBoxSize(dc, config) + 2 * text_padding;
        ++count;
        string_to_print = format_match.suffix();
    }
    dc.DrawText(string_to_print, textPoint);
}

void ContinuityBoxSubPartDrawer::DrawCell(wxDC& dc, CalChartConfiguration const& config, void const* highlight, int x_start, int y_start) const
{
    auto box_padding = fDIP(config.Get_ContCellBoxPadding());

    DrawProcCellBox(dc, config, highlight, x_start + box_padding, y_start + box_padding);
}

void ContinuityBoxSubPartDrawer::OnClick(wxPoint const& point, std::function<void(CalChart::DrawableCont const&)> onClickAction) const
{
    for (auto&& i : mChildren) {
        if ((point.x >= i.mRectStart.x && point.x < (i.mRectStart.x + i.mRectSize.x)) && (point.y >= i.mRectStart.y && point.y < (i.mRectStart.y + i.mRectSize.y))) {
            i.OnClick(point, onClickAction);
            return;
        }
    }

    if ((point.x >= mRectStart.x && point.x < (mRectStart.x + mRectSize.x)) && (point.y >= mRectStart.y && point.y < (mRectStart.y + mRectSize.y))) {
        if (onClickAction)
            onClickAction(mDrawCont);
    }
}

ContinuityBoxDrawer::ContinuityBoxDrawer(CalChart::DrawableCont const& proc, CalChartConfiguration const& config, std::function<void(CalChart::DrawableCont const&)> action)
    : mConfig(config)
    , mContToken(std::make_unique<ContinuityBoxSubPartDrawer>(proc))
    , mClickAction(action)
{
}

ContinuityBoxDrawer::~ContinuityBoxDrawer() = default;

void ContinuityBoxDrawer::DrawToDC(wxDC& dc)
{
    dc.SetFont(CreateFont(mConfig.Get_ContCellFontSize()));
    mContToken->DrawCell(dc, mConfig, mHighlight, 0, 0);
}

int ContinuityBoxDrawer::Height() const
{
    return GetHeight(mConfig);
}

int ContinuityBoxDrawer::GetHeight(CalChartConfiguration const& config)
{
    return fDIP(config.Get_ContCellFontSize() + 2 * config.Get_ContCellBoxPadding() + 2 * config.Get_ContCellTextPadding());
}

int ContinuityBoxDrawer::Width() const
{
    wxMemoryDC temp_dc;
    return mContToken->GetTextBoxSize(temp_dc, mConfig) + fDIP(2 * mConfig.Get_ContCellBoxPadding() + 2 * mConfig.Get_ContCellTextPadding());
}

void ContinuityBoxDrawer::OnClick(wxPoint const& point)
{
    mContToken->OnClick(point, mClickAction);
}
