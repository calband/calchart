#pragma once

#include <memory>
#include <wx/webview.h>
#include <wx/wx.h>

class wxWebView;
class CalChartDoc;

/**
 * ViewerPanel is a wxPanel that displays the CalChart Viewer in an embedded wxWebView.
 * It automatically refreshes when the current show changes.
 */
class ViewerPanel : public wxPanel {
public:
    explicit ViewerPanel(wxWindow* parent, CalChartDoc* doc);
    ~ViewerPanel() override;

    ViewerPanel(const ViewerPanel&) = delete;
    ViewerPanel& operator=(const ViewerPanel&) = delete;

    /**
     * Refresh the viewer with the current show data (full page reload).
     */
    void RefreshViewer();

    /**
     * Update the show data without reloading the page.
     * This triggers the JavaScript loadShow() function to fetch fresh data.
     */
    void UpdateShowData();

    /**
     * Navigate to the viewer homepage.
     */
    void GoHome();

private:
    void OnDocumentChanged(wxNotifyEvent& event);
    void OnRefreshTimer(wxTimerEvent& event);
    void OnPageLoaded(wxWebViewEvent& event);

    wxWebView* mWebView = nullptr;
    CalChartDoc* mDoc = nullptr;
    wxTimer mRefreshTimer;

    wxDECLARE_EVENT_TABLE();
};
