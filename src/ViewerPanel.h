#pragma once

#include <memory>
#include <optional>
#include <wx/wx.h>

// wxWebView is only available on platforms with webview support
#if wxUSE_WEBVIEW
#include <wx/webview.h>
#define CALCHART_HAS_WEBVIEW 1
#else
#define CALCHART_HAS_WEBVIEW 0
#endif

#if CALCHART_HAS_WEBVIEW
class wxWebView;
#endif
class CalChartDoc;

/**
 * ViewerPanel is a wxPanel that displays the CalChart Viewer in an embedded wxWebView.
 * It automatically refreshes when the current show changes.
 * On platforms without wxWebView support, this panel displays a message.
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
     * Inject JSON data directly into the viewer (bypasses server).
     * @param jsonData The JSON string representing the show data
     */
    void InjectShowData(const std::string& jsonData);

    /**
     * Inject JSON data when the page is ready. If the page isn't loaded yet,
     * the data is queued and injected after the load event.
     */
    void InjectShowDataWhenReady(const std::string& jsonData);

    /**
     * Navigate to the viewer homepage.
     */
    void GoHome();

    /**
     * Check if the page is fully loaded and ready for script execution
     */
    bool IsPageLoaded() const { return mPageLoaded; }

private:
    void OnRefreshTimer(wxTimerEvent& event);
#if CALCHART_HAS_WEBVIEW
    void OnDocumentChanged(wxNotifyEvent& event);
    void OnPageLoaded(wxWebViewEvent& event);

    wxWebView* mWebView = nullptr;
#else
    wxStaticText* mMessageText = nullptr;
#endif
    CalChartDoc* mDoc = nullptr;
    wxTimer mRefreshTimer;
    bool mPageLoaded = false;
    bool mPendingUpdate = false;
    std::optional<std::string> mPendingInjectedShowJson;
    int mPendingInjectAttempts = 0;
    static constexpr int kMaxPendingInjectAttempts = 50; // ~10s at 200ms intervals

    wxDECLARE_EVENT_TABLE();
};
