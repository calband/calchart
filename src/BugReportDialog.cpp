/*
 * BugReportDialog.cpp
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

#include "BugReportDialog.h"
#include "CalChartApp.h"
#include "CalChartConfiguration.h"
#include "CalChartDoc.h"
#include "DiagnosticInfo.h"
#include "SystemConfiguration.h"
#include <wx/hyperlink.h>
#include <wx/msgdlg.h>

namespace {

int PromptForTokenOrBrowser(wxWindow* parent)
{
    wxMessageDialog dlg(parent,
        "GitHub API Token Not Configured or Expired.\n\n"
        "You have two options:\n\n"
        "Option 1: Enter Your GitHub Token\n"
        "  Create a classic token and submit the report directly to GitHub.\n\n"
        "Option 2: Open in Browser\n"
        "  We'll open GitHub in your browser with a pre-filled issue form.\n"
        "  You can review and submit it there.\n\n"
        "Which would you prefer?",
        "Submit Bug Report",
        wxYES_NO | wxCANCEL | wxICON_INFORMATION);

    dlg.SetYesNoLabels("Enter token", "Open in browser");
    return dlg.ShowModal();
}

void OpenBugReportInBrowser(CalChart::BugReport const& report)
{
    // Create formatted markdown for the issue
    auto formatted = CalChart::FormatBugReportAsMarkdown(report);

    // URL-encode the body for the GitHub issue URL
    std::string body = formatted;
    std::string encoded_body;
    for (char c : body) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded_body += c;
        } else if (c == '\n') {
            encoded_body += "%0A";
        } else if (c == ' ') {
            encoded_body += "%20";
        } else {
            // URL-encode other characters
            char buf[4];
            snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
            encoded_body += buf;
        }
    }

    // Build the GitHub issue URL
    wxString url = wxString::FromUTF8(
        "https://github.com/calband/calchart/issues/new"
        "?title="
        + report.title + "&body=" + encoded_body);

    // Open the URL in the default browser
    wxLaunchDefaultBrowser(url);

    wxMessageBox(
        "Your browser is opening a new GitHub issue form with your bug report pre-filled.\n\n"
        "You can review the content and click 'Submit new issue' to file the report.",
        "Opening GitHub",
        wxOK | wxICON_INFORMATION);
}

} // namespace

// Custom dialog for entering GitHub token with clickable hyperlinks
class GitHubTokenDialog : public wxDialog {
public:
    explicit GitHubTokenDialog(wxFrame* parent);
    [[nodiscard]] auto GetToken() const -> std::string;

private:
    wxUI::TextCtrl::Proxy mTokenCtrl{};
};

GitHubTokenDialog::GitHubTokenDialog(wxFrame* parent)
    : wxDialog(parent, wxID_ANY, "Enter GitHub Token", wxDefaultPosition)
{
    // auto* scrolled = new wxScrolledWindow(this, wxID_ANY);
    // scrolled->SetScrollRate(0, 5);

    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 5).Expand(),
        wxUI::Text{ "Create a GitHub Personal Access Token" }
            .withFont(wxFontInfo(12).Bold()),
        wxUI::Text{ "To submit bug reports directly, create a classic token:" },
        wxUI::VSizer{
            wxSizerFlags{}.Border(wxALL, 5).Align(wxLEFT | wxRIGHT | wxTOP),
            wxUI::Hyperlink{
                "1. Go to GitHub Token Settings",
                "https://github.com/settings/tokens/new?scopes=public_repo&description=CalChart%20FileABug%20Token" },
            wxUI::Text{ "2. Click 'Generate new token (classic)'" },
            wxUI::Text{ "3. Select only the 'public_repo' scope" },
            wxUI::Text{ "4. Scroll to bottom and press 'Generate token'" },
            wxUI::Text{ "5. Paste the token below" },
        },
        wxUI::Text{ "" }, // spacer
        wxUI::Text{ "Paste your token here:" },
        wxUI::TextCtrl{}
            .withProxy(mTokenCtrl)
            .withWidth(500)
            .withFlags(wxSizerFlags{}.Expand()),
        wxUI::Line{},
        wxUI::HSizer{
            wxSizerFlags{}.Border(wxALL, 5).Left(),
            wxUI::Button{ wxID_OK, "Submit" },
            wxUI::Button{ wxID_CANCEL, "Cancel" },
        },
    }
        .fitTo(this);
}

auto GitHubTokenDialog::GetToken() const -> std::string
{
    return mTokenCtrl->GetValue().ToStdString();
}

BEGIN_EVENT_TABLE(BugReportDialog, wxDialog)
EVT_BUTTON(wxID_OK, BugReportDialog::OnSubmit)
END_EVENT_TABLE()

IMPLEMENT_CLASS(BugReportDialog, wxDialog)

BugReportDialog::BugReportDialog(
    CalChart::Configuration& config,
    const CalChartDoc* doc,
    wxFrame* parent,
    std::string const& title)
    : super{ parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU }
    , mConfig{ config }
    , mDoc(doc)
    , mDiagnosticInfo{ wxCalChart::CollectDiagnosticInfo(mDoc, wxGetApp().GetLogBuffer()) }
    , mShowHasLoaded{ doc != nullptr }
{
    constexpr auto minLargeWidth = 400;
    constexpr auto minLargeHeight = 80;
    constexpr auto scrollableHeight = 500;

    // Create a scrolled window for the entire dialog content
    auto* scrolled = new wxScrolledWindow(this, wxID_ANY);
    scrolled->SetScrollRate(0, 5); // Scroll 5 pixels per wheel event vertically

    // Create the UI structure inside the scrolled window
    wxUI::VSizer{
        wxSizerFlags{}.Border(wxALL, 2).Expand(),
        // Privacy notice
        wxUI::Text{ "Privacy Notice: Your bug report will be submitted to the CalChart GitHub issue tracker.\n"
                    "The following information will be included unless you uncheck below:\n"
                    "  • System information (OS, version, architecture, CalChart version)\n"
                    "  • Display information (screen resolution, DPI)\n"
                    "  • Optionally: Current show file information (number of sheets, marchers, etc.)\n"
                    "\n"
                    "Please do NOT include passwords, personal data, or confidential information in your bug report." },
        wxUI::Text{ "" }, // spacer
        // Title
        wxUI::Text{ "Title*:" },
        wxUI::TextCtrl{}
            .withWidth(minLargeWidth)
            .withFlags(wxSizerFlags{}.Expand())
            .withProxy(text_title),
        // Description
        wxUI::Text{ "Description*:" },
        wxUI::TextCtrl{}
            .withHeight(minLargeHeight)
            .withWidth(minLargeWidth)
            .withFlags(wxSizerFlags{}.Expand())
            .setStyle(wxTE_MULTILINE)
            .withProxy(text_description),
        // Steps to reproduce
        wxUI::Text{ "Steps to Reproduce:" },
        wxUI::TextCtrl{}
            .withHeight(minLargeHeight)
            .withWidth(minLargeWidth)
            .withFlags(wxSizerFlags{}.Expand())
            .setStyle(wxTE_MULTILINE)
            .withProxy(text_steps),
        // Email (optional)
        wxUI::Text{ "Email (optional, for follow-up):" },
        wxUI::TextCtrl{}
            .withWidth(minLargeWidth)
            .withFlags(wxSizerFlags{}.Expand())
            .withProxy(text_email),
        wxUI::Line{},
        // System Information
        wxUI::Text{ "Diagnostic Information (checked = included in report):" },
        wxUI::CheckBox{ "Include system information (OS, version, CalChart version, etc.)" }
            .withProxy(check_system_info)
            .bind(
                [this]() { UpdateDiagnosticInfoDisplay(); }),
        wxUI::CheckBox{ "Include current show information (sheets, marchers, modes, etc.)" }
            .withProxy(check_show_info)
            .bind(
                [this]() { UpdateDiagnosticInfoDisplay(); }),
        wxUI::Text{ "Preview of information to be sent:" },
        wxUI::TextCtrl{}
            .withHeight(minLargeHeight)
            .withWidth(minLargeWidth)
            .withFlags(wxSizerFlags{}.Expand())
            .setStyle(wxTE_MULTILINE | wxTE_READONLY)
            .withProxy(text_diagnostic_info),
        wxUI::Line{},
        // Buttons
        wxUI::HSizer{
            wxSizerFlags{}.Border(wxALL, 5).Left(),
            wxUI::Button{ wxID_OK, "Submit" },
            wxUI::Button{ wxID_CANCEL, "Cancel" }
                .setDefault(),
        },
    }
        .fitTo(scrolled);

    // Create the main dialog layout with scrolled window and buttons at bottom
    auto* dialogSizer = new wxBoxSizer(wxVERTICAL);
    dialogSizer->Add(scrolled, 1, wxEXPAND | wxALL, 0);

    SetSizer(dialogSizer);

    // Set a reasonable default size for the dialog
    SetClientSize(minLargeWidth + 40, scrollableHeight);

    if (GetSizer()) {
        GetSizer()->SetSizeHints(this);
    }

    Centre();
}

auto BugReportDialog::TransferDataToWindow() -> bool
{
    if (!wxDialog::TransferDataToWindow()) {
        return false;
    }

    // Set default values
    mBugReportData.include_system_info = true;
    mBugReportData.include_show = mShowHasLoaded;

    check_system_info->SetValue(mBugReportData.include_system_info);
    check_show_info->SetValue(mBugReportData.include_show);
    check_show_info->Enable(mShowHasLoaded);

    // Update diagnostic info display
    UpdateDiagnosticInfoDisplay();

    return true;
}

auto BugReportDialog::TransferDataFromWindow() -> bool
{
    if (!wxDialog::TransferDataFromWindow()) {
        return false;
    }

    // Get values from controls
    mBugReportData.title = text_title->GetValue().ToStdString();
    mBugReportData.description = text_description->GetValue().ToStdString();
    mBugReportData.steps_to_reproduce = text_steps->GetValue().ToStdString();
    mBugReportData.email = text_email->GetValue().ToStdString();
    mBugReportData.include_system_info = check_system_info->GetValue();
    mBugReportData.include_show = check_show_info->GetValue();

    return true;
}

auto BugReportDialog::Validate() -> bool
{
    // Check that title is not empty
    if (text_title->GetValue().IsEmpty()) {
        wxLogError("Please enter a title for your bug report.");
        return false;
    }

    // Check that description is not empty
    if (text_description->GetValue().IsEmpty()) {
        wxLogError("Please enter a description of the bug.");
        return false;
    }

    // Validate email if provided
    if (!text_email->GetValue().IsEmpty()) {
        auto email = text_email->GetValue();
        // Simple email validation - check for @ and at least one dot
        if (email.Find('@') == wxNOT_FOUND || email.Find('.') == wxNOT_FOUND) {
            wxLogError("Please enter a valid email address or leave it blank.");
            return false;
        }
    }

    return true;
}

void BugReportDialog::UpdateDiagnosticInfoDisplay()
{
    // Update the diagnostic info display based on checkbox state
    std::string info_text;

    if (check_system_info->GetValue()) {
        info_text = mDiagnosticInfo.toString();
    } else {
        info_text = "System information will not be included.";
    }

    if (check_show_info->GetValue() && mShowHasLoaded) {
        info_text += "\n[Show information will be included]";
    }

    text_diagnostic_info->SetValue(info_text);
}

void BugReportDialog::EnableControls(bool enable)
{
    text_title->Enable(enable);
    text_description->Enable(enable);
    text_steps->Enable(enable);
    text_email->Enable(enable);
    check_system_info->Enable(enable);
    if (mShowHasLoaded) {
        check_show_info->Enable(enable);
    }
    wxDialog::FindWindow(wxID_OK)->Enable(enable);
    wxDialog::FindWindow(wxID_CANCEL)->Enable(enable);
}

void BugReportDialog::OnSubmit(wxCommandEvent&)
{
    // Validate the input
    if (!Validate()) {
        return;
    }

    // Transfer data from window
    if (!TransferDataFromWindow()) {
        return;
    }

    // Create the bug report structure for submission
    CalChart::BugReport report;
    report.title = mBugReportData.title;
    report.description = mBugReportData.description;
    report.steps_to_reproduce = mBugReportData.steps_to_reproduce;
    report.email = mBugReportData.email;
    report.include_system_info = mBugReportData.include_system_info;
    report.include_show = mBugReportData.include_show;
    report.diagnostic_info = mDiagnosticInfo;

    // Check for token in order: environment variable → configuration → prompt user
    std::string token;

    // 1. Try environment variable first
    if (auto* token_env = std::getenv("CALCHART_GITHUB_TOKEN");
        token_env && std::string(token_env).length() > 0) {
        token = token_env;
    }

    // 2. Try configuration if doc is available
    if (token.empty()) {
        token = mConfig.Get_GitHubToken();
    }

    // 3. If still no token, ask user
    if (token.empty()) {

        // No token configured - show user their options
        switch (PromptForTokenOrBrowser(this)) {
        case wxID_YES: {
            // User wants to enter token - show custom entry dialog with clickable link
            GitHubTokenDialog tokenDlg(dynamic_cast<wxFrame*>(this->GetParent()));
            if (tokenDlg.ShowModal() != wxID_OK) {
                return; // User cancelled token entry
            }
            token = tokenDlg.GetToken();
            if (token.empty()) {
                wxLogError("Token cannot be empty.");
                return;
            }

            // Save token to configuration for future use
            mConfig.Set_GitHubToken(token);
            mConfig.FlushWriteQueue();
            // Fall through to submission with provided token
            break;
        }
        case wxID_NO:
            OpenBugReportInBrowser(report);
            EndModal(wxID_OK);
            return;
        case wxID_CANCEL: // fallthrough
        default:
            return; // User cancelled
        }
    }

    // Disable controls during submission
    EnableControls(false);
    wxLogStatus("Submitting bug report...");

    // Start background submission with optional provided token
    // Note: The callback will be called on the UI thread via wxApp::CallAfter()
    // so we can directly update UI from it without posting events
    CalChart::StartBackgroundIssueSubmission(
        report,
        [this](CalChart::IssueSubmissionStatus status, std::string const& issue_url, std::string const& error_message) {
            // This callback is already running on the UI thread (via wxApp::CallAfter)
            // So we can directly execute UI code here
            EnableControls(true);

            switch (status) {
            case CalChart::IssueSubmissionStatus::Success:
                if (!issue_url.empty()) {
                    wxMessageBox(
                        wxString::Format("Bug report submitted successfully!\n\nIssue: %s", issue_url),
                        "Success",
                        wxOK | wxICON_INFORMATION);
                    EndModal(wxID_OK);
                } else {
                    wxMessageBox(
                        error_message,
                        "Fallback: Copied to Clipboard",
                        wxOK | wxICON_INFORMATION);
                    EndModal(wxID_OK);
                }
                break;

            case CalChart::IssueSubmissionStatus::NetworkError:
                wxMessageBox(
                    wxString::Format("Network error: %s\n\nTip: Make sure you have internet access and the GitHub API is reachable.", error_message),
                    "Network Error",
                    wxOK | wxICON_ERROR);
                break;

            case CalChart::IssueSubmissionStatus::ApiError: {
                // Check if this is an expired token error
                auto is_expired = error_message.find("token") != std::string::npos && (error_message.find("expir") != std::string::npos || error_message.find("expired") != std::string::npos);
                auto is_auth_error = error_message.find("Authentication failed") != std::string::npos;

                if (is_expired || is_auth_error) {
                    // Clear the stored token and let them enter a new one
                    mConfig.Clear_GitHubToken();
                    mConfig.FlushWriteQueue();
                    // Recursively call OnSubmit to show token entry dialog
                    wxCommandEvent evt;
                    OnSubmit(evt);
                } else {
                    wxMessageBox(
                        wxString::Format("GitHub API error: %s\n\nTip: Try using 'Open in browser' instead to file the issue manually.", error_message),
                        "API Error",
                        wxOK | wxICON_ERROR);
                }
                break;
            }

            case CalChart::IssueSubmissionStatus::InvalidInput:
                wxMessageBox(
                    wxString::Format("Invalid input: %s", error_message),
                    "Invalid Input",
                    wxOK | wxICON_ERROR);
                break;

            case CalChart::IssueSubmissionStatus::UnknownError:
                wxMessageBox(
                    wxString::Format("Unknown error: %s", error_message),
                    "Error",
                    wxOK | wxICON_ERROR);
                break;
            }
        },
        token);
}
