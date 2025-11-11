/*
 * CalChartApp.cpp
 * Central App for CalChart
 */

#include "HostAppInterface.h"
#include "CalChartApp.h"

#include <algorithm>
#include <vector>
#include <wx/ipc.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

const wxString OPEN_FILE = "OpenFile";

// because these are all implementation details, using structs here make sense.  No encapsulation needed.

struct CCAppClientConnection;
struct CCAppServerConnection;

struct CCAppServer : public wxServer {
    static auto GetServerName()
    {
        auto name = (wxGetUserId() + wxStandardPaths::Get().GetExecutablePath()).ToStdString();
        return std::to_string(static_cast<int16_t>(std::hash<std::string>{}(name)));
    }

    static std::unique_ptr<CCAppServer> MakeServer(CalChartApp* app)
    {
        auto newServer = std::make_unique<CCAppServer>(app);
        if (!newServer->Create(GetServerName())) {
            return nullptr;
        }
        return newServer;
    }

    CCAppServer(CalChartApp* app)
        : mApp(app)
    {
    }

    virtual ~CCAppServer() override;
    virtual wxConnectionBase* OnAcceptConnection(wxString const& topic) override;

    void OpenFile(wxString const& filename)
    {
        mApp->OpenFile(filename);
    }

    // should only be called by the connection
    void DestroyConnection(CCAppServerConnection* connection)
    {
        mActiveConnections.erase(std::remove(mActiveConnections.begin(), mActiveConnections.end(), connection));
    }

private:
    // maintain a list of connections to disconnect from on d-tor
    std::vector<CCAppServerConnection*> mActiveConnections;
    CalChartApp* mApp{};
};

struct CCAppClient : public wxClient {
    static std::unique_ptr<CCAppClient> MakeClient()
    {
        auto newClient = std::make_unique<CCAppClient>();
        if (newClient->Connect("localhost", CCAppServer::GetServerName(), "Client")) {
            return newClient;
        }
        return nullptr;
    }

    virtual ~CCAppClient() override { Disconnect(); }

    virtual wxConnectionBase* OnMakeConnection() override;

    void OpenFile(wxString const& filename);

    // should only be called by the connection
    void DestroyConnection() { mConnection = nullptr; }

private:
    bool Connect(wxString const& host, wxString const& service, wxString const& topic);
    auto IsConnected() { return mConnection != nullptr; }
    void Disconnect();

    // mConnection may be null, depending on the state of the connection
    CCAppClientConnection* mConnection{};
};

// Connection pair:
// Servers will get Poked, then they turn around and ask the server object to
// open it.
// Clients will Poke the servers to open files for them.
struct CCAppServerConnection : public wxConnection {
    using super = wxConnection;

    CCAppServerConnection(CCAppServer* server)
        : mServer(server)
    {
    }

    // ServerConnection: OnPoke, ask the server to open a file.
    virtual bool OnPoke(wxString const& topic, wxString const& item, void const* data, size_t size, wxIPCFormat format) override
    {
        if (item == OPEN_FILE) {
            mServer->OpenFile(std::string(static_cast<char const*>(data), size - 1));
        }
        return super::OnPoke(topic, item, data, size, format);
    }

    virtual bool OnDisconnect() override
    {
        mServer->DestroyConnection(this);
        return super::OnDisconnect();
    }

private:
    CCAppServer* mServer;
};

struct CCAppClientConnection : public wxConnection {
    using super = wxConnection;

    CCAppClientConnection(CCAppClient* client)
        : mClient(client)
    {
    }

    // ClientConnect: Poke the server to open a file.
    void OpenFile(wxString const& filename) { Poke(OPEN_FILE, filename); }

    virtual bool OnDisconnect() override
    {
        mClient->DestroyConnection();
        return super::OnDisconnect();
    }

private:
    CCAppClient* mClient;
};

CCAppServer::~CCAppServer()
{
    while (mActiveConnections.size() > 0) {
        // this should cause the connection to turn around and call
        // DestroyConnection()
        mActiveConnections.front()->Disconnect();
    }
}

wxConnectionBase* CCAppServer::OnAcceptConnection(wxString const&)
{
    mActiveConnections.push_back(new CCAppServerConnection(this));
    return mActiveConnections.back();
}

void CCAppClient::OpenFile(wxString const& filename)
{
    if (mConnection != nullptr) {
        mConnection->OpenFile(filename);
    }
}

void CCAppClient::Disconnect()
{
    if (mConnection != nullptr) {
        mConnection->Disconnect();
    }
}

bool CCAppClient::Connect(wxString const& host, wxString const& service, wxString const& topic)
{
    auto oldLogLevel = wxLog::GetLogLevel();
    wxLog::SetLogLevel(wxLOG_FatalError); // Make sure that we don't throw errors
    // to the user if the connection fails;
    // we expect connections to fail when we
    // are testing whether or not there is a
    // server, and the user shouldn't be
    // informed when we cannot find a server
    mConnection = static_cast<CCAppClientConnection*>(MakeConnection(host, service, topic));
    wxLog::SetLogLevel(oldLogLevel);
    return IsConnected();
}

wxConnectionBase* CCAppClient::OnMakeConnection()
{
    return new CCAppClientConnection(this);
}

// Server, Client, and Independent are all derived from HostAppInterface.
// Mostly it's all boilerplate, with "smarts" done in the Make functions
struct ServerSideHostAppInterface : public HostAppInterface {
    static std::unique_ptr<HostAppInterface> Make(CalChartApp* app, StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop);

    ServerSideHostAppInterface(std::unique_ptr<CCAppServer> server,
        StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop)
        : HostAppInterface(serverStartStop, clientStartStop)
        , mServer(std::move(server))
    {
    }
    virtual ~ServerSideHostAppInterface() override { m_serverStartStop.second(); }

    virtual bool OnInit() override
    {
        m_serverStartStop.first();
        return true;
    }

    virtual void OpenFile(const wxString& filename) override
    {
        mServer->OpenFile(filename);
    }

private:
    std::unique_ptr<CCAppServer> mServer;
};

struct ClientSideHostAppInterface : public HostAppInterface {
    static std::unique_ptr<HostAppInterface> Make(StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop);

    ClientSideHostAppInterface(std::unique_ptr<CCAppClient> client,
        StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop)
        : HostAppInterface(serverStartStop, clientStartStop)
        , mClient(std::move(client))
    {
    }
    virtual ~ClientSideHostAppInterface() override { m_clientStartStop.second(); }

    virtual bool OnInit() override
    {
        m_clientStartStop.first();
        return false;
    }

    virtual void OpenFile(const wxString& filename) override
    {
        mClient->OpenFile(filename);
    }

private:
    std::unique_ptr<CCAppClient> mClient;
};

struct IndependentHostAppInterface : public HostAppInterface {
    static std::unique_ptr<HostAppInterface> Make(CalChartApp* app, StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop);

    IndependentHostAppInterface(CalChartApp* app, StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop)
        : HostAppInterface(serverStartStop, clientStartStop)
        , mApp(app)
    {
    }
    virtual ~IndependentHostAppInterface() { m_serverStartStop.second(); }

    virtual bool OnInit()
    {
        m_serverStartStop.first();
        return true;
    }

    virtual void OpenFile(const wxString& filename) { mApp->OpenFile(filename); }

private:
    CalChartApp* mApp;
};

// Linch-pin function that ties everything together
std::unique_ptr<HostAppInterface>
HostAppInterface::Make(CalChartApp* app, StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop)
{
#ifdef __APPLE__
    return IndependentHostAppInterface::Make(app, serverStartStop, clientStartStop);
#else
    auto client = ClientSideHostAppInterface::Make(serverStartStop, clientStartStop);
    if (client) {
        return client;
    }
    return ServerSideHostAppInterface::Make(app, serverStartStop, clientStartStop);
#endif
}

HostAppInterface::HostAppInterface(StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop)
    : m_serverStartStop(serverStartStop)
    , m_clientStartStop(clientStartStop)
{
}

HostAppInterface::~HostAppInterface() { }

std::unique_ptr<HostAppInterface>
ServerSideHostAppInterface::Make(CalChartApp* app, StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop)
{
    auto server = CCAppServer::MakeServer(app);
    if (server != nullptr) {
        return std::make_unique<ServerSideHostAppInterface>(std::move(server), serverStartStop, clientStartStop);
    }
    return nullptr;
}

std::unique_ptr<HostAppInterface>
ClientSideHostAppInterface::Make(StartStopFunc_t serverStartStop,
    StartStopFunc_t clientStartStop)
{
    auto client = CCAppClient::MakeClient();
    if (client != nullptr) {
        return std::make_unique<ClientSideHostAppInterface>(std::move(client), serverStartStop, clientStartStop);
    }
    return nullptr;
}

std::unique_ptr<HostAppInterface>
IndependentHostAppInterface::Make(CalChartApp* app, StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop)
{
    return std::make_unique<IndependentHostAppInterface>(app, serverStartStop, clientStartStop);
}
