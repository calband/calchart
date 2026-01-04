/*
 * WebViewDemoDialog.cpp
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

#include <wx/setup.h>

#if wxUSE_WEBVIEW

#include "WebViewDemoDialog.h"
#include <wx/button.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

WebViewDemoDialog::WebViewDemoDialog(wxWindow* parent)
    : super(parent, wxID_ANY, "wxWebView Demo", wxDefaultPosition, wxSize(800, 600))
{
    // Create main sizer
    auto* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Create control panel with URL bar
    auto* controlPanel = new wxPanel(this);
    auto* controlSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* urlLabel = new wxStaticText(controlPanel, wxID_ANY, "URL:");
    auto* urlInput = new wxTextCtrl(controlPanel, wxID_ANY, "https://www.example.com", wxDefaultPosition, wxSize(400, -1));
    auto* goButton = new wxButton(controlPanel, wxID_ANY, "Go");

    controlSizer->Add(urlLabel, 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
    controlSizer->Add(urlInput, 1, wxALL | wxEXPAND, 5);
    controlSizer->Add(goButton, 0, wxALL, 5);

    controlPanel->SetSizer(controlSizer);
    mainSizer->Add(controlPanel, 0, wxEXPAND);

    // Create the WebView control
    mWebView = wxWebView::New(this, wxID_ANY, "https://www.example.com");
    if (!mWebView) {
        auto* errorMsg = new wxStaticText(this, wxID_ANY,
            "Error: wxWebView is not available on this platform.\n"
            "This requires wxWidgets to be built with webview support.");
        mainSizer->Add(errorMsg, 1, wxALL | wxEXPAND, 10);
    } else {
        mainSizer->Add(mWebView, 1, wxEXPAND);

        // Bind the Go button to load URL
        goButton->Bind(wxEVT_BUTTON, [this, urlInput](wxCommandEvent&) {
            wxString url = urlInput->GetValue();
            if (!url.StartsWith("http://") && !url.StartsWith("https://")) {
                url = "https://" + url;
            }
            mWebView->LoadURL(url);
        });
    }

    SetSizer(mainSizer);
}

#endif // wxUSE_WEBVIEW
