#pragma once
/*
 * HostAppInterface.cpp
 * Object for communicating across different app instances
 */

#include <functional>
#include <memory>

#include <wx/string.h>

class CalChartApp;

using StartStopFunc_t = std::pair<std::function<void()>, std::function<void()>>;

// The HostAppInterface will determine if the App is a server, client or
// independent app.
// Hand it the functions to call if it's a server or client; it determines which
// to call.
class HostAppInterface {
public:
    static std::unique_ptr<HostAppInterface> Make(CalChartApp* app, StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop);
    virtual ~HostAppInterface(); // calls the exit function

    virtual bool OnInit() = 0;
    virtual void OpenFile(const wxString& filename) = 0;

protected:
    HostAppInterface(StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop);
    StartStopFunc_t m_serverStartStop;
    StartStopFunc_t m_clientStartStop;
};
