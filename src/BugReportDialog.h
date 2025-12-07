#pragma once
/*
 * BugReportDialog.h
 * Dialog for collecting and filing bug reports
 */

/*
   Copyright (C) 1995-2025  Garrick Brian Meeker, Richard Michael Powell

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

#include "CalChartDiagnosticInfo.hpp"
#include "GitHubIssueSubmitter.hpp"
#include <string>
#include <vector>
#include <wx/dialog.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

class CalChartDoc;

class BugReportDialog : public wxDialog {
    using super = wxDialog;
    DECLARE_CLASS(BugReportDialog)
    DECLARE_EVENT_TABLE()

public:
    BugReportDialog(
        CalChart::Configuration& config,
        CalChartDoc const* doc = nullptr,
        wxFrame* parent = nullptr,
        std::string const& title = "Report a Bug");
    ~BugReportDialog() override = default;

    // Data transfer methods
    auto TransferDataToWindow() -> bool override;
    auto TransferDataFromWindow() -> bool override;
    auto Validate() -> bool override;

    // Get the bug report data
    struct BugReportData {
        std::string title;
        std::string description;
        std::string steps_to_reproduce;
        std::string email;
        bool include_system_info = true;
        bool include_show = false;
    };

private:
    // Event handlers
    void OnSubmit(wxCommandEvent& event);

    // Helper methods
    void UpdateDiagnosticInfoDisplay();
    void EnableControls(bool enable);

    // UI control proxies
    wxUI::TextCtrl::Proxy text_title;
    wxUI::TextCtrl::Proxy text_description;
    wxUI::TextCtrl::Proxy text_steps;
    wxUI::TextCtrl::Proxy text_email;
    wxUI::CheckBox::Proxy check_system_info;
    wxUI::CheckBox::Proxy check_show_info;
    wxUI::TextCtrl::Proxy text_diagnostic_info;

    // Data members
    CalChart::Configuration& mConfig;
    CalChartDoc const* mDoc{};
    BugReportData mBugReportData{};
    CalChart::DiagnosticInfo mDiagnosticInfo{};
    bool mShowHasLoaded{};
    std::string mProvidedToken{}; // Token entered by user in dialog
};
