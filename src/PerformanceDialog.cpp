/*
 * PerformanceDialog.cpp
 * Dialog for displaying performance metrics
 */

/*
   Copyright (C) 2024  Richard Michael Powell

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

#include "PerformanceDialog.hpp"
#include "CalChartPerformanceRegistry.h"
#include "basic_ui.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <wx/listctrl.h>
#include <wxUI/wxUI.hpp>

namespace {

auto FormatDouble(double value, int precision = 2) -> std::string
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(precision) << value;
    return oss.str();
}
}

BEGIN_EVENT_TABLE(PerformanceDialog, wxDialog)
EVT_CLOSE(PerformanceDialog::OnClose)
END_EVENT_TABLE()

PerformanceDialog::PerformanceDialog(wxWindow* parent, CalChart::PerformanceRegistry& registry)
    : super(parent, wxID_ANY, "Draw Performance Metrics",
          wxDefaultPosition, wxSize(800, 600),
          wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    , mRegistry(registry)
{
    CreateControls();
    RefreshData();
}

void PerformanceDialog::CreateControls()
{
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 10).Expand(),
        wxUI::Text("")
            .withProxy(mSummaryText),
        wxUI::Factory<wxListCtrl>{
            wxSizerFlags{ 1 }.Border(wxALL, 10).Expand(),
            [](wxWindow* window) -> wxListCtrl* {
                auto listCtrl = new wxListCtrl(window, wxID_ANY,
                    wxDefaultPosition, wxDefaultSize,
                    wxLC_REPORT | wxLC_SINGLE_SEL | wxBORDER_SUNKEN);

                // Add columns
                listCtrl->InsertColumn(0, "Component", wxLIST_FORMAT_LEFT, 300);
                listCtrl->InsertColumn(1, "Calls", wxLIST_FORMAT_RIGHT, 80);
                listCtrl->InsertColumn(2, "Avg (ms)", wxLIST_FORMAT_RIGHT, 90);
                listCtrl->InsertColumn(3, "StdDev (ms)", wxLIST_FORMAT_RIGHT, 100);
                listCtrl->InsertColumn(4, "Min (ms)", wxLIST_FORMAT_RIGHT, 90);
                listCtrl->InsertColumn(5, "Max (ms)", wxLIST_FORMAT_RIGHT, 90);
                listCtrl->InsertColumn(6, "Last (ms)", wxLIST_FORMAT_RIGHT, 90);
                return listCtrl;
            } }
            .withProxy(mListCtrl),
        wxUI::HSizer{
            wxUI::Button("Refresh")
                .bind([this] {
                    RefreshData();
                }),
            wxUI::Button("Reset Statistics")
                .bind([this] {
                    ResetStats();
                }),
            wxUI::StretchSpacer{},
            wxUI::Button("Close")
                .bind([this] {
                    Close();
                }),
        }
    }.fitTo(this);
}

void PerformanceDialog::RefreshData()
{
    // Clear existing items
    mListCtrl->DeleteAllItems();

    // Get all statistics
    auto allStats = mRegistry.GetAllStats();

    // Sort by average time (slowest first)
    std::sort(allStats.begin(), allStats.end(),
        [](auto const& a, auto const& b) {
            return a.second.average > b.second.average;
        });

    // Update summary
    auto registeredCount = mRegistry.GetRegisteredCount();
    size_t totalCalls = 0;
    double totalTime = 0.0;

    for (auto&& [ptr, stats] : allStats) {
        totalCalls += stats.callCount;
        totalTime += stats.average.count() * 1000.0 * stats.callCount;
    }

    auto summaryText = std::string("Registered Components: ") + std::to_string(registeredCount) + "  |  Total Paint Calls: " + std::to_string(totalCalls) + "  |  Total Time: " + FormatDouble(totalTime) + " ms";

    if (totalCalls > 0) {
        summaryText += "  |  Average per Call: " + FormatDouble(totalTime / totalCalls) + " ms";
    }

    mSummaryText->SetLabel(summaryText);

    // Populate list
    long itemIndex = 0;
    for (auto&& [ptr, stats] : allStats) {
        auto index = mListCtrl->InsertItem(itemIndex, stats.name);

        mListCtrl->SetItem(index, 1, std::to_string(stats.callCount));
        mListCtrl->SetItem(index, 2, FormatDouble(stats.average.count() * 1000.0));
        mListCtrl->SetItem(index, 3, FormatDouble(stats.stdDev.count() * 1000.0));
        mListCtrl->SetItem(index, 4, FormatDouble(stats.min.count() * 1000.0));
        mListCtrl->SetItem(index, 5, FormatDouble(stats.max.count() * 1000.0));
        mListCtrl->SetItem(index, 6, FormatDouble(stats.last.count() * 1000.0));

        // Color code by performance (optional enhancement)
        if (stats.average.count() * 1000.0 > 16.67) { // Slower than 60fps
            mListCtrl->SetItemTextColour(index, *wxRED);
        } else if (stats.average.count() * 1000.0 > 8.33) { // Slower than 120fps
            mListCtrl->SetItemTextColour(index, wxColour(255, 140, 0)); // Orange
        }

        ++itemIndex;
    }

    // Auto-size columns to fit content
    for (int i = 0; i < 7; ++i) {
        mListCtrl->SetColumnWidth(i, wxLIST_AUTOSIZE);
        // Make sure column isn't too narrow
        if (mListCtrl->GetColumnWidth(i) < 80 && i > 0) {
            mListCtrl->SetColumnWidth(i, 80);
        }
    }

    // Ensure component name column is at least 200px
    if (mListCtrl->GetColumnWidth(0) < 200) {
        mListCtrl->SetColumnWidth(0, 200);
    }
}

void PerformanceDialog::ResetStats()
{
    auto result = wxMessageBox(
        "This will clear all performance measurements but keep the registered components.\n\n"
        "Are you sure?",
        "Reset Statistics",
        wxYES_NO | wxICON_QUESTION,
        this);

    if (result == wxYES) {
        mRegistry.ResetMeasurements();
        RefreshData();
    }
}

void PerformanceDialog::OnClose(wxCloseEvent& event)
{
    // Hide instead of destroy since this is modeless
    if (event.CanVeto()) {
        Hide();
        event.Veto();
    } else {
        event.Skip();
    }
}
