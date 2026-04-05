/*
 * BeatMapDialog.cpp
 * Dialog box for mapping beats
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

#include "BeatMapDialog.hpp"

#include "CalChartConfiguration.h"
#include "CalChartRanges.h"

bool IsValidIntFloatMap(std::string_view input, CalChart::Beats beats)
{
    // Reject empty or whitespace-only strings explicitly
    if (input.empty()) {
        return true;
    }
    if (input.find_first_not_of(" \t") == std::string_view::npos) {
        return false;
    }

    auto map = CalChart::ToFermatas(input);
    if (map.empty()) {
        return false;
    }
    return std::all_of(map.begin(), map.end(), [beats](auto&& pair) {
        return pair.first < beats;
    });
}

BeatMapDialog::BeatMapDialog(
    std::vector<BeatSheetInfo> const& beatSheetInfo,
    wxFrame* parent)
    : wxDialog{
        parent,
        wxID_ANY,
        "Beat Map Dialog",
        wxDefaultPosition,
        { 100, 100 },
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER
    }
    , mBeatSheetProxyInfo(beatSheetInfo.size())
{
    // Create a scrolled window for the entire dialog content
    auto* scrolled = new wxScrolledWindow(this, wxID_ANY);
    scrolled->SetScrollRate(0, 5); // Scroll 5 pixels per wheel event vertically

    wxUI::VSizer{
        wxUI::VForEach(
            wxSizerFlags{}.Border(wxALL, 2),
            CalChart::Ranges::enumerate_view(beatSheetInfo),
            [this](auto const& pair) {
                auto const& [index, info] = pair;
                auto const& [beatSheetInfo, beats] = info;
                auto const& [tempo, fermatas] = beatSheetInfo;
                std::get<2>(mBeatSheetProxyInfo.at(index)) = beats;
                std::string fermataString = CalChart::ToString(fermatas);
                return wxUI::HSizer{
                    std::format("Sheet {}, beats {}", index, beats),
                    wxUI::HSizer{
                        wxUI::Text{ "Tempo" },
                        wxUI::SpinCtrl{ std::pair{ 1, 320 }, tempo }.withProxy(std::get<0>(mBeatSheetProxyInfo.at(index))),
                    },
                    wxUI::HSizer{
                        wxUI::Text{ "Fermatas" },
                        wxUI::TextCtrl{ std::format("{}", fermataString) }
                            .withSize({ 100, -1 })
                            .withStyle(wxTE_PROCESS_ENTER)
                            .withProxy(std::get<1>(mBeatSheetProxyInfo.at(index)))
                            .bind([this]() {
                                runValidator();
                            }),
                    },
                };
            }),
        wxUI::HSizer{
            wxUI::Button{ wxID_OK }.setDefault().bind([this]() {
                if (validate()) {
                    EndModal(wxID_OK);
                }
            }),
            wxUI::Button{ wxID_CANCEL }.bind([this]() {
                EndModal(wxID_CANCEL);
            }),
        }
    }
        .fitTo(scrolled);
    auto* dialogSizer = new wxBoxSizer(wxVERTICAL);
    dialogSizer->Add(scrolled, 1, wxEXPAND | wxALL, 0);

    SetSizer(dialogSizer);

    if (GetSizer()) {
        GetSizer()->SetSizeHints(this);
    }

    // Centre();
}

auto BeatMapDialog::validate() const -> bool
{
    try {
        return std::all_of(mBeatSheetProxyInfo.begin(), mBeatSheetProxyInfo.end(), [](auto&& proxies) {
            auto&& [spin, fermatas, beats] = proxies;
            if (!IsValidIntFloatMap(std::string{ *fermatas }, beats)) {
                wxLogDebug("fermatas \"%s\" failed", std::string{ *fermatas }.c_str());
                return false;
            }

            return true;
        });
    } catch (const std::invalid_argument&) {
        return false;
    } catch (const std::out_of_range&) {
        return false;
    }
}

void BeatMapDialog::runValidator() const
{
    auto valid = validate();
    if (wxButton* btn = wxDynamicCast(FindWindow(wxID_OK), wxButton); btn) {
        btn->Enable(valid);
    }
    if (wxButton* btn = wxDynamicCast(FindWindow(wxID_CANCEL), wxButton); btn) {
        btn->Enable(!valid);
    }
}

auto BeatMapDialog::GetBeatSheetInfo() const -> std::vector<BeatSheetInfo>
{
    auto rawInfo = GetBeatSheetRawInfo();
    std::vector<BeatSheetInfo> result;
    result.reserve(rawInfo.size());

    std::transform(rawInfo.begin(), rawInfo.end(), std::back_inserter(result), [](auto&& raw) {
        auto&& [spin, fermatas, beats] = raw;
        auto parsedFermatas = CalChart::ToFermatas(fermatas);
        return BeatSheetInfo{
            CalChart::SheetBeatInfo{
                static_cast<CalChart::Tempo>(spin),
                parsedFermatas },
            beats
        };
    });
    return result;
}

auto BeatMapDialog::GetBeatSheetRawInfo() const -> std::vector<BeatSheetRawInfo>
{
    std::vector<BeatSheetRawInfo> result;
    result.reserve(mBeatSheetProxyInfo.size());

    std::transform(mBeatSheetProxyInfo.begin(), mBeatSheetProxyInfo.end(), std::back_inserter(result), [](auto&& proxies) {
        auto&& [spin, fermatas, beats] = proxies;
        return BeatSheetRawInfo{
            *spin,
            *fermatas,
            beats
        };
    });
    return result;
}

auto BeatMapPlaygroundData() -> std::vector<BeatSheetInfo>
{
    return std::vector<BeatSheetInfo>{
        BeatSheetInfo{ CalChart::SheetBeatInfo{ 120, std::map<CalChart::Beats, CalChart::Seconds>{} }, 16 },
        BeatSheetInfo{ CalChart::SheetBeatInfo{ 130, std::map<CalChart::Beats, CalChart::Seconds>{ std::pair<CalChart::Beats, CalChart::Seconds>{ 0, 1.5 } } }, 16 },
        BeatSheetInfo{ CalChart::SheetBeatInfo{ 115, std::map<CalChart::Beats, CalChart::Seconds>{} }, 16 },
        BeatSheetInfo{ CalChart::SheetBeatInfo{ 120, std::map<CalChart::Beats, CalChart::Seconds>{} }, 16 },
        BeatSheetInfo{ CalChart::SheetBeatInfo{ 120, std::map<CalChart::Beats, CalChart::Seconds>{} }, 16 },
    };
}