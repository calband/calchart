#pragma once
/*
 * BeatMapper.h
 * Dialog box for mapping beats
 */

/*
   Copyright (C) 1995-2026  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartTypes.h"
#include <set>
#include <wx/dialog.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class CalChartDoc;
namespace CalChart {
class Configuration;
}

// Use BeatMapDialog to adjust all the sheet's beat infomration.   That is:
// Tempo
// Start Delay
// Fermatas
using BeatSheetInfo = std::tuple<CalChart::SheetBeatInfo, CalChart::Beats>;

using BeatSheetProxyInfo = std::tuple<wxUI::SpinCtrl::Proxy, wxUI::TextCtrl::Proxy, CalChart::Beats>;

using BeatSheetRawInfo = std::tuple<int, std::string, CalChart::Beats>;

auto IsValidIntFloatMap(std::string_view input, CalChart::Beats beats) -> bool;

auto IsValidRawInfo(BeatSheetRawInfo const& input) -> bool;

class BeatMapDialog : public wxDialog {
public:
    BeatMapDialog(
        std::vector<BeatSheetInfo> const& beatSheetInfo,
        wxFrame* parent);
    ~BeatMapDialog() override = default;

    std::vector<BeatSheetInfo> GetBeatSheetInfo() const;

private:
    std::vector<BeatSheetRawInfo> GetBeatSheetRawInfo() const;
    bool validate() const;
    void runValidator() const;

    std::vector<BeatSheetProxyInfo> mBeatSheetProxyInfo;
};

auto BeatMapPlaygroundData() -> std::vector<BeatSheetInfo>;
