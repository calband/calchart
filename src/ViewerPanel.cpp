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
    if (mWebView) {
        // Trigger the viewer's loadCalChartShow() function to refresh show data
        // This function is exposed by the viewer's application.js
        mWebView->RunScript("if (typeof loadCalChartShow === 'function') { loadCalChartShow(); }");
    }
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

    // Intercept the AJAX call that loadFromCalChart() makes and inject our data
    std::string script = "(function() {\n"
                         "  try {\n"
                         "    if (typeof ApplicationController === 'undefined' || typeof $ === 'undefined') {\n"
                         "      return false;\n"
                         "    }\n"
                         "    \n"
                         "    var originalAjax = $.ajax;\n"
                         "    window.__calchart_injected_data = "
        + jsonData + ";\n"
                     "    \n"
                     "    $.ajax = function(options) {\n"
                     "      if (options && options.url === '/api/show') {\n"
                     "        $.ajax = originalAjax;\n"
                     "        \n"
                     "        setTimeout(function() {\n"
                     "          if (options.success) {\n"
                     "            options.success(window.__calchart_injected_data);\n"
                     "            \n"
                     "            // Remove loading screen and force UI sync\n"
                     "            setTimeout(function() {\n"
                     "              $('.loading').remove();\n"
                     "              var appController = ApplicationController.getInstance();\n"
                     "              if (appController.getShow()) {\n"
                     "                appController._syncWithDelegate();\n"
                     "              }\n"
                     "            }, 50);\n"
                     "          }\n"
                     "          delete window.__calchart_injected_data;\n"
                     "        }, 10);\n"
                     "        \n"
                     "        return {\n"
                     "          done: function(cb) { return this; },\n"
                     "          fail: function(cb) { return this; },\n"
                     "          always: function(cb) { return this; }\n"
                     "        };\n"
                     "      } else {\n"
                     "        $.ajax = originalAjax;\n"
                     "        return originalAjax.apply(this, arguments);\n"
                     "      }\n"
                     "    };\n"
                     "    \n"
                     "    setTimeout(function() {\n"
                     "      if (typeof loadCalChartShow === 'function') {\n"
                     "        loadCalChartShow();\n"
                     "      } else {\n"
                     "        $.ajax = originalAjax;\n"
                     "      }\n"
                     "    }, 100);\n"
                     "    \n"
                     "    return true;\n"
                     "  } catch (err) {\n"
                     "    console.error('InjectShowData error:', err);\n"
                     "    return false;\n"
                     "  }\n"
                     "})();\n";

    wxString result;
    if (!mWebView->RunScript(script, &result) || result != "true") {
        wxLogWarning("ViewerPanel: Failed to inject show data");
    }
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
    // Timer disabled - JavaScript in the page handles periodic updates
    // This allows the page to stay loaded and update content without full page reloads
}

#if CALCHART_HAS_WEBVIEW
void ViewerPanel::OnPageLoaded(wxWebViewEvent& event)
{
    wxLogDebug("ViewerPanel: Page loaded event fired for: %s", event.GetURL().c_str());
    mPageLoaded = true;

    // Don't automatically load from the server - let the caller decide
    // whether to use UpdateShowData() or InjectShowData()
    wxLogDebug("ViewerPanel: Page ready for data injection");
}
#endif
