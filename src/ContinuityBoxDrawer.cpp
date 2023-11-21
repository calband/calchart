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
#include "CalChartDrawCommand.h"
#include "CalChartDrawPrimativesHelper.h"
#include "CalChartDrawing.h"
#include "CalChartRanges.h"
#include "DCSaveRestore.h"
#include "basic_ui.h"

// Draw each ContToken and then all the children at the right location.
class ContinuityBoxSubPartDrawer {
public:
    explicit ContinuityBoxSubPartDrawer(CalChart::Cont::Drawable const& proc);
    [[nodiscard]] auto GetTextBoxSize(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent) const -> int;
    [[nodiscard]] auto GetChildrenBeginSize(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent) const -> std::vector<std::tuple<int, int>>;
    [[nodiscard]] auto GetDrawCommands(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, void const* highlight) const
    {
        auto box_padding = fDIP(config.Get_ContCellBoxPadding());
        return GetProcCellBoxDrawCommands(config, getTextExtent, highlight) + CalChart::Coord(box_padding, box_padding);
    }

    void OnClick(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, wxPoint const& point, std::function<void(CalChart::Cont::Drawable const&)> if_hit) const;

private:
    auto GetProcCellBoxDrawCommands(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, void const* highlight) const -> std::vector<CalChart::DrawCommand>;
    void HandleOnClick(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, wxPoint const& point, std::function<void(CalChart::Cont::Drawable const&)> if_hit) const;

    CalChart::Cont::Drawable mDrawCont;
    std::vector<ContinuityBoxSubPartDrawer> mChildren;
};

namespace {

auto ContTypeToCellColors(CalChart::Cont::Type contType) -> CalChart::ContinuityCellColors
{
    switch (contType) {
    case CalChart::Cont::Type::procedure:
        return CalChart::ContinuityCellColors::PROC;
    case CalChart::Cont::Type::value:
        return CalChart::ContinuityCellColors::VALUE;
    case CalChart::Cont::Type::function:
        return CalChart::ContinuityCellColors::FUNCTION;
    case CalChart::Cont::Type::direction:
        return CalChart::ContinuityCellColors::DIRECTION;
    case CalChart::Cont::Type::steptype:
        return CalChart::ContinuityCellColors::STEPTYPE;
    case CalChart::Cont::Type::point:
        return CalChart::ContinuityCellColors::POINT;
    case CalChart::Cont::Type::unset:
        return CalChart::ContinuityCellColors::UNSET;
    case CalChart::Cont::Type::outline:
        return CalChart::ContinuityCellColors::OUTLINE;
    case CalChart::Cont::Type::selected:
        return CalChart::ContinuityCellColors::SELECTED;
    }
    return CalChart::ContinuityCellColors::PROC;
}

auto GetBrush(CalChart::Cont::Type contType, CalChartConfiguration const& config)
{
    return CalChart::toBrush(config.Get_ContCellBrushAndPen(ContTypeToCellColors(contType)));
}

auto GetPen(CalChart::Cont::Type contType, bool highlighted, [[maybe_unused]] CalChartConfiguration const& config)
{
    if (highlighted) {
        return CalChart::toPen(config.Get_ContCellBrushAndPen(ContTypeToCellColors(CalChart::Cont::Type::selected)));
    }
    auto pen = CalChart::toPen(config.Get_ContCellBrushAndPen(ContTypeToCellColors(CalChart::Cont::Type::outline)));
    if (contType == CalChart::Cont::Type::unset) {
        pen.style = CalChart::Pen::Style::ShortDash;
    }
    return pen;
}

static auto SplitString(std::string_view input, std::string_view delim)
{
    return CalChart::Ranges::ToVector<std::string>(std::views::split(input, delim)
        | std::views::transform([](auto i) { return std::string{ i.begin(), i.end() }; }));
}

}

ContinuityBoxSubPartDrawer::ContinuityBoxSubPartDrawer(CalChart::Cont::Drawable const& proc)
    : mDrawCont(proc)
    , mChildren{ mDrawCont.args.begin(), mDrawCont.args.end() }
{
}

int ContinuityBoxSubPartDrawer::GetTextBoxSize(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent) const
{
    auto text_padding = fDIP(config.Get_ContCellTextPadding());

    auto format_sub_size = getTextExtent("%@").x;
    auto total_size = std::accumulate(mChildren.begin(), mChildren.end(), 0, [&config, text_padding, getTextExtent](auto&& acc, auto&& child) { return acc + child.GetTextBoxSize(config, getTextExtent) + text_padding * 2; });
    total_size += getTextExtent(config.Get_ContCellLongForm() ? mDrawCont.description : mDrawCont.short_description).x;
    total_size -= format_sub_size * mDrawCont.args.size();

    return total_size;
}

// We determine where we are going to draw by parsing out the continuity string, substituting the children sizes,
// and then laying out the text.  In order to know where things are, we determine where the children should be.
//
// Consider this example.  We are going to assume that strlen is the size for x, but in reality it is whatever the
// text extent of the current device context:
// "( %@ and %@ + %@ )"
// That has 3 children, and imagine they had these sizes:
// mChildren -> [ 8, 3, 6 ]
//
// if we were to "render" this text, it would be something like:
// 0123456789012345678901234567890
// ( cccccccc and ccc + cccccc )
// or we would draw child0 at 2, and it would be 8 long, child1 at 15, and it would be 3 long, and child2 at 21,
// and it would be 6.  So we would want to create the sequence of (start, size) that would look like:
// [(2, 8), (15, 3), (21, 6)]
//
// So first what we would do is break the description into its parts, and create running sum using exclusive scan, and drop 1st:
// "( %@ and %@ + %@ )" -> [ "( ", " and ", " + ", " )" ] -> [2, 5, 3, 2] -> [0, 2, 7, 10] -> [2, 7, 10]
//
// We then Calculate the mChildren sizes, and create an sum using exclusive scan:
// mChildren -> [ 8, 3, 6 ]
// Then what we do is  -> [8, 11, 17] or -> [0, 8, 11]
// We now add that to the first sequence:
// [2, 7, 10] + [0, 8, 11] -> [ 2, 15, 21 ]
// And then we zip it up with the mChildren sizes:
// [(2, 8), (15, 3), (21, 6)]
//
// so what we have is
// zip_view(transform(exclusive_sum(words) -> drop_first, exclusive_scan(children),+), childSizes)
auto ContinuityBoxSubPartDrawer::GetChildrenBeginSize(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent) const -> std::vector<std::tuple<int, int>>
{
    auto words = SplitString(config.Get_ContCellLongForm() ? mDrawCont.description : mDrawCont.short_description, "%@");
    auto text_padding = fDIP(config.Get_ContCellTextPadding());

    auto exwordsStarts = std::vector<int>{};
    std::exclusive_scan(words.begin(), words.end(), std::back_inserter(exwordsStarts), text_padding, [getTextExtent](auto acc, auto str) { return acc + getTextExtent(std::string{ str }).x; });

    auto childrenSizes = CalChart::Ranges::ToVector<int>(mChildren | std::views::transform([&config, text_padding, getTextExtent](auto&& child) { return child.GetTextBoxSize(config, getTextExtent) + 2 * text_padding; }));

    auto exchildrenSizes = std::vector<int>{};
    std::exclusive_scan(childrenSizes.begin(), childrenSizes.end(), std::back_inserter(exchildrenSizes), 0);

    return CalChart::Ranges::ToVector<std::tuple<int, int>>(CalChart::Ranges::zip_view(
        CalChart::Ranges::zip_view(
            exwordsStarts | std::views::drop(1),
            exchildrenSizes)
            | std::views::transform([](auto&& values) {
                  return std::get<0>(values) + std::get<1>(values);
              }),
        childrenSizes));
}

auto ContinuityBoxSubPartDrawer::GetProcCellBoxDrawCommands(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, void const* highlight) const -> std::vector<CalChart::DrawCommand>
{
    auto text_padding = fDIP(config.Get_ContCellTextPadding());
    CalChart::Coord::units rounding = config.Get_ContCellRounding();
    auto box_size_y = fDIP(config.Get_ContCellFontSize()) + 2 * text_padding;
    auto box_size_x = GetTextBoxSize(config, getTextExtent) + 2 * text_padding;

    auto words = SplitString(config.Get_ContCellLongForm() ? mDrawCont.description : mDrawCont.short_description, "%@");

    auto childInfo = GetChildrenBeginSize(config, getTextExtent);

    auto drawCmds = std::vector{
        CalChart::Draw::withBrush(
            GetBrush(mDrawCont.type, config),
            CalChart::Draw::withPen(
                GetPen(mDrawCont.type, mDrawCont.self_ptr == highlight, config),
                CalChart::Draw::Rectangle{ CalChart::Coord{}, CalChart::Coord(box_size_x, box_size_y), rounding }))
    };

    // The first text to draw is at the beginning
    CalChart::append(drawCmds,
        std::vector{ CalChart::Draw::Text{ CalChart::Coord(text_padding, text_padding), words.front() } });

    // draw the rest of the text after each of the children.
    CalChart::append(drawCmds,
        CalChart::Ranges::zip_view(childInfo, words | std::views::drop(1))
            | std::views::transform([text_padding](auto bundle) {
                  auto where = std::get<0>(bundle);
                  return CalChart::Draw::Text{ CalChart::Coord(std::get<0>(where) + std::get<1>(where), text_padding), std::string{ std::get<1>(bundle) } };
              }));

    // draw the children
    for (auto bundle : CalChart::Ranges::zip_view(mChildren, childInfo)) {
        CalChart::append(drawCmds, std::get<0>(bundle).GetProcCellBoxDrawCommands(config, getTextExtent, highlight) + CalChart::Coord(std::get<0>(std::get<1>(bundle)), 0));
    }
    return drawCmds;
}

void ContinuityBoxSubPartDrawer::OnClick(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, wxPoint const& point, std::function<void(CalChart::Cont::Drawable const&)> onClickAction) const
{
    auto box_padding = fDIP(config.Get_ContCellBoxPadding());

    return HandleOnClick(config, getTextExtent, point - wxPoint(box_padding, box_padding), onClickAction);
}

void ContinuityBoxSubPartDrawer::HandleOnClick(CalChartConfiguration const& config, std::function<wxSize(std::string const&)> getTextExtent, wxPoint const& point, std::function<void(CalChart::Cont::Drawable const&)> onClickAction) const
{
    auto text_padding = fDIP(config.Get_ContCellTextPadding());
    auto box_size_y = fDIP(config.Get_ContCellFontSize()) + 2 * text_padding;
    auto childInfo = GetChildrenBeginSize(config, getTextExtent);

    for (auto bundle : CalChart::Ranges::zip_view(mChildren, childInfo)) {
        auto [startx, sizex] = std::get<1>(bundle);
        if ((point.x >= startx && point.x < (startx + sizex)) && (point.y >= 0 && point.y < box_size_y)) {
            std::get<0>(bundle).HandleOnClick(config, getTextExtent, point - wxPoint(startx, 0), onClickAction);
            return;
        }
    }
    auto box_size_x = GetTextBoxSize(config, getTextExtent) + 2 * text_padding;
    auto rectStart = wxPoint(0, 0);
    auto rectSize = wxSize(box_size_x, box_size_y);
    if (onClickAction && (point.x >= rectStart.x && point.x < (rectStart.x + rectSize.x)) && (point.y >= rectStart.y && point.y < (rectStart.y + rectSize.y))) {
        onClickAction(mDrawCont);
    }
}

ContinuityBoxDrawer::ContinuityBoxDrawer(CalChart::Cont::Drawable const& proc, CalChartConfiguration const& config, std::function<void(CalChart::Cont::Drawable const&)> action)
    : mConfig(config)
    , mContToken(std::make_unique<ContinuityBoxSubPartDrawer>(proc))
    , mClickAction(action)
{
}

ContinuityBoxDrawer::~ContinuityBoxDrawer() = default;

auto ContinuityBoxDrawer::GetDrawCommands(wxDC& dc) -> std::vector<CalChart::DrawCommand>
{
    // this is necessary so we calculate the correct text extents
    dc.SetFont(CreateFont(mConfig.Get_ContCellFontSize()));
    auto getTextExtent = [&dc](std::string str) { return dc.GetTextExtent(str); };
    return mContToken->GetDrawCommands(mConfig, getTextExtent, mHighlight);
}

int ContinuityBoxDrawer::Height() const
{
    return GetHeight(mConfig);
}

int ContinuityBoxDrawer::GetHeight(CalChartConfiguration const& config)
{
    return fDIP(config.Get_ContCellFontSize() + 2 * config.Get_ContCellBoxPadding() + 2 * config.Get_ContCellTextPadding());
}

int ContinuityBoxDrawer::GetMinWidth(CalChartConfiguration const& config)
{
    return fDIP(config.Get_ContCellFontSize() * 32 + 2 * config.Get_ContCellBoxPadding() + 2 * config.Get_ContCellTextPadding());
}

int ContinuityBoxDrawer::Width() const
{
    wxMemoryDC temp_dc;
    auto getTextExtent = [&temp_dc](std::string str) { return temp_dc.GetTextExtent(str); };
    return mContToken->GetTextBoxSize(mConfig, getTextExtent) + fDIP(2 * mConfig.Get_ContCellBoxPadding() + 2 * mConfig.Get_ContCellTextPadding());
}

void ContinuityBoxDrawer::OnClick(wxDC& dc, wxPoint const& point)
{
    dc.SetFont(CreateFont(mConfig.Get_ContCellFontSize()));
    auto getTextExtent = [&dc](std::string const& str) { return dc.GetTextExtent(str); };
    mContToken->OnClick(mConfig, getTextExtent, point, mClickAction);
}
