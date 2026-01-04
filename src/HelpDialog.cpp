/*
 * HelpDialog.cpp
 * Implementation of help display dialog
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

#include "HelpDialog.hpp"

#if wxUSE_WEBVIEW

#include <wx/webview.h>
#include <wxUI/wxUI.hpp>

HelpDialog::HelpDialog(wxWindow* parent, HelpManager& helpManager)
    : wxFrame(parent, wxID_ANY, "CalChart Help", wxDefaultPosition, wxSize(800, 600))
    , mHelpManager(helpManager)
{
    wxUI::VSizer{
        wxSizerFlags{}.Expand().Border(wxALL, 0),
        // Toolbar with navigation buttons and search
        wxUI::HSizer{
            wxSizerFlags{}.Border(wxALL, 5),
            wxUI::Button{ wxID_BACKWARD, "← Back" }
                .withProxy(mBackButton)
                .bind([this](auto const& event) { OnBack(const_cast<wxCommandEvent&>(event)); }),
            wxUI::Button{ wxID_FORWARD, "Forward →" }
                .withProxy(mForwardButton)
                .bind([this](auto const& event) { OnForward(const_cast<wxCommandEvent&>(event)); }),
            wxUI::Button{ wxID_HOME, "Home" }
                .withProxy(mHomeButton)
                .bind([this](auto const& event) { OnHome(const_cast<wxCommandEvent&>(event)); }),
            wxUI::Button{ wxID_ANY, "Contents" }
                .withProxy(mTocButton)
                .bind([this](auto const& event) { OnTableOfContents(const_cast<wxCommandEvent&>(event)); }),
            wxUI::Text{ "" }, // Stretch spacer
            wxUI::Text{ "Search:" }.withFlags(wxSizerFlags{}.CenterVertical()),
            wxUI::TextCtrl{}
                .withProxy(mSearchBox)
                .withWidth(150)
                .withStyle(wxTE_PROCESS_ENTER)
                .bind(wxEVT_TEXT_ENTER, [this](auto const& event) { OnSearchEnter(const_cast<wxCommandEvent&>(event)); }),
        },
        // WebView for displaying help content
        mWebView = wxUI::Generic<wxWebView>{
            wxSizerFlags(1).Expand(),
            [this](wxWindow* parent) {
                auto* webView = wxWebView::New(parent, wxID_ANY);
                webView->SetMinSize(wxSize(-1, 400));
                webView->Bind(wxEVT_WEBVIEW_NAVIGATING, &HelpDialog::OnWebViewNavigating, this);
                webView->Bind(wxEVT_WEBVIEW_ERROR, &HelpDialog::OnWebViewError, this);
                webView->Bind(wxEVT_WEBVIEW_LOADED, &HelpDialog::OnWebViewLoaded, this);
                return webView;
            },
        },
    }
        .fitTo(this);

    Center();

    UpdateNavigationButtons();

    // Display the help index on startup
    DisplayIndex();
}

void HelpDialog::DisplayIndex()
{
    LoadTopic("index");
}

void HelpDialog::DisplayTopic(const std::string& topicId)
{
    LoadTopic(topicId);
}

void HelpDialog::LoadTopic(const std::string& topicId)
{
    // Save current topic to back stack if we're navigating away
    if (!mCurrentTopicId.empty() && mCurrentTopicId != topicId) {
        mBackStack.push(mCurrentTopicId);
        mForwardStack = std::stack<std::string>(); // Clear forward stack on new navigation
    }

    mCurrentTopicId = topicId;

    // Get the file path for the topic
    std::string filePath = mHelpManager.GetHelpTopicPath(topicId);
    
    if (filePath.empty()) {
        wxLogWarning("Could not find file path for topic: %s", topicId);
        // Fallback to SetPage with error message
        mWebView->SetPage("<html><body><h1>Help topic not found</h1><p>Topic: " + wxString(topicId) + "</p></body></html>", "");
    } else {
        // Convert file path to file:// URL
        wxString fileUrl = "file:///" + wxString(filePath);
        fileUrl.Replace("\\", "/");
        
        wxLogInfo("LoadTopic: topicId='%s', loading URL='%s'", topicId, fileUrl);
        
        // Use LoadURL instead of SetPage - more reliable on Windows
        mWebView->LoadURL(fileUrl);
    }

    UpdateNavigationButtons();
}

void HelpDialog::OnBack(wxCommandEvent&)
{
    if (mBackStack.empty()) {
        return;
    }
    // Push current topic to forward stack
    if (!mCurrentTopicId.empty()) {
        mForwardStack.push(mCurrentTopicId);
    }

    // Get and display the previous topic
    std::string topicId = mBackStack.top();
    mBackStack.pop();
    
    mCurrentTopicId = topicId;
    
    std::string filePath = mHelpManager.GetHelpTopicPath(topicId);
    if (!filePath.empty()) {
        wxString fileUrl = "file:///" + wxString(filePath);
        fileUrl.Replace("\\", "/");
        mWebView->LoadURL(fileUrl);
    }

    UpdateNavigationButtons();
}

void HelpDialog::OnForward(wxCommandEvent&)
{
    if (mForwardStack.empty()) {
        return;
    }
    // Push current topic to back stack
    if (!mCurrentTopicId.empty()) {
        mBackStack.push(mCurrentTopicId);
    }

    // Get and display the next topic
    std::string topicId = mForwardStack.top();
    mForwardStack.pop();
    
    mCurrentTopicId = topicId;
    
    std::string filePath = mHelpManager.GetHelpTopicPath(topicId);
    if (!filePath.empty()) {
        wxString fileUrl = "file:///" + wxString(filePath);
        fileUrl.Replace("\\", "/");
        mWebView->LoadURL(fileUrl);
    }

    UpdateNavigationButtons();
}

void HelpDialog::OnHome(wxCommandEvent&)
{
    LoadTopic("index");
}

void HelpDialog::OnTableOfContents(wxCommandEvent&)
{
    LoadTopic("index");
}

void HelpDialog::OnSearchEnter(wxCommandEvent&)
{
    wxString query = mSearchBox->GetValue();
    if (query.empty()) {
        return;
    }

    // Search the help
    auto results = mHelpManager.SearchHelp(std::string(query.mb_str()));

    // Display search results as HTML
    wxString resultsHtml = "<html><body>";
    resultsHtml.Append(wxString::Format("<h1>Search Results for '%s'</h1>", query.c_str()));

    if (results.empty()) {
        resultsHtml.Append("<p>No matching help topics found.</p>");
    } else {
        resultsHtml.Append("<ul>");
        for (const auto& result : results) {
            resultsHtml.Append(wxString::Format(
                "<li><strong>%s</strong> (relevance: %d)</li>",
                result.title.c_str(), result.relevanceScore));
        }
        resultsHtml.Append("</ul>");
    }

    resultsHtml.Append("</body></html>");
    mWebView->SetPage(resultsHtml, "");
}

void HelpDialog::OnClose(wxCloseEvent&)
{
    // Destroy the dialog window
    Destroy();
}

void HelpDialog::UpdateNavigationButtons()
{
    mBackButton->Enable(!mBackStack.empty());
    mForwardButton->Enable(!mForwardStack.empty());
}

void HelpDialog::OnChar(wxKeyEvent& event)
{
    // WebView handles text selection and copying natively
    event.Skip();
}

void HelpDialog::OnWebViewNavigating(wxWebViewEvent& event)
{
    wxString url = event.GetURL();

    // Handle help:// protocol for internal navigation
    if (url.StartsWith("help://")) {
        wxString topicId = url.substr(7); // Remove "help://" prefix
        LoadTopic(std::string(topicId.mb_str()));
        event.Veto(); // Prevent default navigation
    }
    // Handle external links - open in default browser (WebView will do this automatically)
    // No need to veto, WebView handles http/https correctly
}

void HelpDialog::OnWebViewError(wxWebViewEvent& event)
{
    wxLogError("WebView error: %s (URL: %s)", event.GetString(), event.GetURL());
}

void HelpDialog::OnWebViewLoaded(wxWebViewEvent& event)
{
    wxLogInfo("WebView loaded successfully: %s", event.GetURL());
}

#endif // wxUSE_WEBVIEW
