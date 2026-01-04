#pragma once
/*
 * WebViewDemoDialog.h
 * Simple dialog to demonstrate wxWebView functionality
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

#include <wx/webview.h>
#include <wx/wx.h>

// Simple dialog demonstrating wxWebView functionality
class WebViewDemoDialog : public wxDialog {
    using super = wxDialog;

public:
    WebViewDemoDialog(wxWindow* parent);
    ~WebViewDemoDialog() = default;

private:
    void SetupWebView();
    wxWebView* mWebView = nullptr;
};

#endif // wxUSE_WEBVIEW
