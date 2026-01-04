#include "ViewerPanel.h"

#include <iostream>
#include <wx/timer.h>
#include <wx/webview.h>

#include "CalChartApp.h"
#include "CalChartDoc.h"
#include "ViewerServer.h"

wxBEGIN_EVENT_TABLE(ViewerPanel, wxPanel)
    EVT_TIMER(wxID_ANY, ViewerPanel::OnRefreshTimer)
        EVT_WEBVIEW_LOADED(wxID_ANY, ViewerPanel::OnPageLoaded)
            wxEND_EVENT_TABLE()

                ViewerPanel::ViewerPanel(wxWindow* parent, CalChartDoc* doc)
    : wxPanel(parent, wxID_ANY)
    , mDoc(doc)
    , mRefreshTimer(this)
{
    wxLogDebug("ViewerPanel: Constructing ViewerPanel");
    auto sizer = new wxBoxSizer(wxVERTICAL);

    // Create the wxWebView using the factory method with full parameters
    // Start with a placeholder URL - we'll navigate to the actual viewer after creation
    mWebView = wxWebView::New(this, wxID_ANY, wxWebViewDefaultURLStr, wxDefaultPosition, wxDefaultSize);
    if (!mWebView) {
        wxLogError("ViewerPanel: Failed to create wxWebView");
        return;
    }

    sizer->Add(mWebView, 1, wxEXPAND | wxALL, 0);
    SetSizer(sizer);
    wxLogDebug("ViewerPanel: wxWebView created successfully");

    // Don't auto-reload here - let the JavaScript in the page handle periodic updates
    wxLogDebug("ViewerPanel: JavaScript will handle periodic refresh");

    // Notify the server about the current doc
    if (mDoc) {
        auto& app = *wxGetApp().GetInstance();
        CalChartApp* pApp = static_cast<CalChartApp*>(&app);
        pApp->GetViewerServer().SetCurrentDoc(mDoc);
        wxLogDebug("ViewerPanel: Set current doc on server");
    } else {
        wxLogWarning("ViewerPanel: mDoc is null!");
    }

    // Navigate to the viewer
    GoHome();
}

ViewerPanel::~ViewerPanel()
{
    mRefreshTimer.Stop();
    // Clear the server's current doc reference
    if (mDoc) {
        auto& app = *wxGetApp().GetInstance();
        CalChartApp* pApp = static_cast<CalChartApp*>(&app);
        pApp->GetViewerServer().SetCurrentDoc(nullptr);
    }
}

void ViewerPanel::RefreshViewer()
{
    if (mWebView) {
        mWebView->Reload();
    }
}

void ViewerPanel::UpdateShowData()
{
    if (mWebView) {
        // Trigger the viewer's loadCalChartShow() function to refresh show data
        // This function is exposed by the viewer's application.js
        mWebView->RunScript("if (typeof loadCalChartShow === 'function') { loadCalChartShow(); }");
    }
}

void ViewerPanel::GoHome()
{
    if (mWebView) {
        auto& app = *wxGetApp().GetInstance();
        CalChartApp* pApp = static_cast<CalChartApp*>(&app);
        auto url = pApp->GetViewerServer().GetViewerUrl();
        wxLogDebug("ViewerPanel: Navigating to URL: %s", url.c_str());
        mWebView->LoadURL(url);
    } else {
        wxLogWarning("ViewerPanel: mWebView is null in GoHome()");
    }
}

void ViewerPanel::OnDocumentChanged(wxNotifyEvent& event)
{
    RefreshViewer();
    event.Skip();
}

void ViewerPanel::OnRefreshTimer(wxTimerEvent&)
{
    // Timer disabled - JavaScript in the page handles periodic updates
    // This allows the page to stay loaded and update content without full page reloads
}

void ViewerPanel::OnPageLoaded(wxWebViewEvent& event)
{
    wxLogDebug("ViewerPanel: Page loaded event fired for: %s", event.GetURL().c_str());

    // The viewer's HTML/JS is fully loaded, now trigger it to load the show data
    // from CalChart's /api/show endpoint
    UpdateShowData();
}
