#include "ViewerPanel.h"

#include <iostream>
#include <wx/timer.h>

#if CALCHART_HAS_WEBVIEW
#include <wx/webview.h>
#endif

#include "CalChartApp.h"
#include "CalChartDoc.h"

#if CALCHART_HAS_WEBVIEW
#include "ViewerServer.h"
#endif

#if CALCHART_HAS_WEBVIEW
static bool IsViewerUrl(const wxString& url)
{
    return url.StartsWith("http://localhost:1868");
}
#endif

wxBEGIN_EVENT_TABLE(ViewerPanel, wxPanel)
    EVT_TIMER(wxID_ANY, ViewerPanel::OnRefreshTimer)
#if CALCHART_HAS_WEBVIEW
        EVT_WEBVIEW_LOADED(wxID_ANY, ViewerPanel::OnPageLoaded)
#endif
            wxEND_EVENT_TABLE()

                ViewerPanel::ViewerPanel(wxWindow* parent, CalChartDoc* doc)
    : wxPanel(parent, wxID_ANY)
    , mDoc(doc)
    , mRefreshTimer(this)
{
#if CALCHART_HAS_WEBVIEW
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
#else
    // On platforms without wxWebView support, show a message
    wxLogDebug("ViewerPanel: wxWebView not available on this platform");
    auto sizer = new wxBoxSizer(wxVERTICAL);

    mMessageText = new wxStaticText(this, wxID_ANY,
        "CalChart Viewer is not available on this platform.\n\n"
        "The viewer requires wxWebView, which is not supported on this system.\n"
        "You can still export shows to .viewer files and view them\n"
        "in a web browser using the standalone CalChart Online Viewer.",
        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);

    sizer->AddStretchSpacer();
    sizer->Add(mMessageText, 0, wxALIGN_CENTER | wxALL, 20);
    sizer->AddStretchSpacer();
    SetSizer(sizer);
#endif
}

ViewerPanel::~ViewerPanel()
{
    mRefreshTimer.Stop();
#if CALCHART_HAS_WEBVIEW
    // Clear the server's current doc reference
    if (mDoc) {
        auto& app = *wxGetApp().GetInstance();
        CalChartApp* pApp = static_cast<CalChartApp*>(&app);
        pApp->GetViewerServer().SetCurrentDoc(nullptr);
        pApp->GetViewerServer().SetInjectedShowJson({});
    }
#endif
}

void ViewerPanel::RefreshViewer()
{
#if CALCHART_HAS_WEBVIEW
    if (mWebView) {
        mWebView->Reload();
    }
#endif
}

void ViewerPanel::UpdateShowData()
{
#if CALCHART_HAS_WEBVIEW
    if (!mWebView) {
        return;
    }

    if (!mPageLoaded) {
        mPendingUpdate = true;
        return;
    }

    mPendingUpdate = false;
    // Trigger the viewer's loadCalChartShow() function to refresh show data
    // This function is exposed by the viewer's application.js
    mWebView->RunScriptAsync("if (typeof loadCalChartShow === 'function') { loadCalChartShow(); }");
#endif
}

void ViewerPanel::InjectShowData(const std::string& jsonData)
{
#if CALCHART_HAS_WEBVIEW
    if (!mWebView) {
        wxLogWarning("ViewerPanel: Cannot inject show data - webview not initialized");
        return;
    }

    if (!mPageLoaded) {
        wxLogWarning("ViewerPanel: Page not loaded yet, injection may fail");
    }

    wxLogDebug("ViewerPanel: injecting shows data via server (%zu bytes)", jsonData.size());

    auto& app = *wxGetApp().GetInstance();
    CalChartApp* pApp = static_cast<CalChartApp*>(&app);
    pApp->GetViewerServer().SetInjectedShowJson(jsonData);

    UpdateShowData();
    wxLogDebug("ViewerPanel: injected shows data (server override)");
#endif
}

void ViewerPanel::InjectShowDataWhenReady(const std::string& jsonData)
{
#if CALCHART_HAS_WEBVIEW
    if (mPageLoaded) {
        InjectShowData(jsonData);
        return;
    }

    mPendingInjectedShowJson = jsonData;
    mPendingInjectAttempts = 0;
    if (!mRefreshTimer.IsRunning()) {
        mRefreshTimer.Start(200);
    }
#else
    (void)jsonData;
#endif
}

void ViewerPanel::GoHome()
{
#if CALCHART_HAS_WEBVIEW
    if (mWebView) {
        auto& app = *wxGetApp().GetInstance();
        CalChartApp* pApp = static_cast<CalChartApp*>(&app);
        auto url = pApp->GetViewerServer().GetViewerUrl();
        wxLogDebug("ViewerPanel: Navigating to URL: %s", url.c_str());
        mWebView->LoadURL(url);
    } else {
        wxLogWarning("ViewerPanel: mWebView is null in GoHome()");
    }
#endif
}

#if CALCHART_HAS_WEBVIEW
void ViewerPanel::OnDocumentChanged(wxNotifyEvent& event)
{
    RefreshViewer();
    event.Skip();
}
#endif

void ViewerPanel::OnRefreshTimer(wxTimerEvent&)
{
    if (!mPendingInjectedShowJson.has_value()) {
        mRefreshTimer.Stop();
        return;
    }

#if CALCHART_HAS_WEBVIEW
    if (mWebView && !mWebView->IsBusy()) {
        auto currentUrl = mWebView->GetCurrentURL();
        if (IsViewerUrl(currentUrl)) {
            mPageLoaded = true;
            auto pending = std::move(*mPendingInjectedShowJson);
            mPendingInjectedShowJson.reset();
            mRefreshTimer.Stop();
            InjectShowData(pending);
            return;
        }
    }
#endif

    if (++mPendingInjectAttempts >= kMaxPendingInjectAttempts) {
        wxLogWarning("ViewerPanel: Timed out waiting for page load; giving up on auto-inject");
        mRefreshTimer.Stop();
    }
}

#if CALCHART_HAS_WEBVIEW
void ViewerPanel::OnPageLoaded(wxWebViewEvent& event)
{
    wxLogDebug("ViewerPanel: Page loaded event fired for: %s", event.GetURL().c_str());

    if (!IsViewerUrl(event.GetURL())) {
        wxLogDebug("ViewerPanel: Ignoring non-viewer page load: %s", event.GetURL().c_str());
        return;
    }

    mPageLoaded = true;

    // Don't automatically load from the server - let the caller decide
    // whether to use UpdateShowData() or InjectShowData()
    wxLogDebug("ViewerPanel: Page ready for data injection");

    if (mPendingUpdate) {
        UpdateShowData();
    }

    if (mPendingInjectedShowJson.has_value()) {
        auto pending = std::move(*mPendingInjectedShowJson);
        mPendingInjectedShowJson.reset();
        mRefreshTimer.Stop();
        InjectShowData(pending);
    }
}
#endif
