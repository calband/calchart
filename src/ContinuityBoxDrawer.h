#pragma once
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

#include "CustomListViewPanel.h"
#include <memory>

class CalChartConfiguration;
namespace CalChart::Cont {
class Token;
struct Drawable;
}

// class for drawing continuity in box form on a dc.

class ContinuityBoxSubPartDrawer;

class ContinuityBoxDrawer : public DrawableCell {
public:
    ContinuityBoxDrawer(CalChart::Cont::Drawable const& proc, CalChartConfiguration const& config, std::function<void(CalChart::Cont::Drawable const&)> action = nullptr);
    ~ContinuityBoxDrawer() override;
    void SetHighlight(void const* ptr) override { mHighlight = ptr; }
    auto GetDrawCommands(wxDC& dc) -> std::vector<CalChart::DrawCommand> override;
    auto Height() const -> int override;
    auto Width() const -> int override;
    void OnClick(wxDC& dc, wxPoint const&) override;

    static int GetHeight(CalChartConfiguration const&);
    static int GetMinWidth(CalChartConfiguration const&);

private:
    CalChartConfiguration const& mConfig;
    std::unique_ptr<ContinuityBoxSubPartDrawer> mContToken;
    std::function<void(CalChart::Cont::Drawable const&)> mClickAction;
    void const* mHighlight = nullptr;
};
