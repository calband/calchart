/*
 * HelpDialog.hpp
 * Dialog for displaying help content
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

#pragma once

#include <wx/setup.h>

#if wxUSE_WEBVIEW

#include "HelpManager.hpp"

#include <memory>
#include <stack>
#include <wx/webview.h>
#include <wx/wx.h>
#include <wxUI/wxUI.hpp>

/**
 * HelpDialog
 *
 * A modeless dialog that displays help content from the HelpManager using WebView.
 * Features:
 * - Modern HTML rendering via WebView
 * - Navigation (back/forward, TOC)
 * - Search functionality
 * - Persistent window size and position
 *
 * Usage:
 *   auto dialog = std::make_unique<HelpDialog>(parent, helpManager);
 *   dialog->DisplayTopic("introduction");
 *   dialog->Show();
 */
class HelpDialog : public wxFrame {

public:
    /**
     * Create a help dialog
     * @param parent Parent window
     * @param helpManager Reference to help manager (must outlive this dialog)
     */
    HelpDialog(wxWindow* parent, HelpManager& helpManager);

    ~HelpDialog() override = default;

    /**
     * Display a specific help topic
     * @param topicId Topic identifier
     */
    void DisplayTopic(const std::string& topicId);

    /**
     * Display the help index
     */
    void DisplayIndex();

    /**
     * Load and display a specific help topic
     * Internal method used for link navigation
     */
    void LoadTopic(const std::string& topicId);

private:
    // UI handlers
    void OnBack(wxCommandEvent& event);
    void OnForward(wxCommandEvent& event);
    void OnHome(wxCommandEvent& event);
    void OnSearch(wxCommandEvent& event);
    void OnSearchEnter(wxCommandEvent& event);
    void OnTableOfContents(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnChar(wxKeyEvent& event);
    void OnWebViewNavigating(wxWebViewEvent& event);
    void OnWebViewError(wxWebViewEvent& event);
    void OnWebViewLoaded(wxWebViewEvent& event);

    // Helper methods
    void UpdateNavigationButtons();

    // Member variables
    HelpManager& mHelpManager;
    wxUI::Generic<wxWebView>::Proxy mWebView{};
    wxUI::TextCtrl::Proxy mSearchBox{};
    wxUI::Button::Proxy mBackButton{};
    wxUI::Button::Proxy mForwardButton{};
    wxUI::Button::Proxy mHomeButton{};
    wxUI::Button::Proxy mTocButton{};

    std::stack<std::string> mBackStack{}; // Topics we've visited (for back button)
    std::stack<std::string> mForwardStack{}; // Topics we've navigated away from (for forward button)
    std::string mCurrentTopicId{}; // Currently displayed topic
};

#endif // wxUSE_WEBVIEW
