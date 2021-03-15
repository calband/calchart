
#include "single_instance_ipc.h"
#include "CalChartApp.h"

#include <algorithm>
#include <vector>
#include <wx/ipc.h>
#include <wx/stdpaths.h>
#include <wx/utils.h>

const wxString OPEN_FILE = "OpenFile";

class CCAppClientConnection;
class CCAppServerConnection;

class CCAppServer : public wxServer {
public:
    static wxString GetServerName();

    static std::unique_ptr<CCAppServer> MakeServer(CalChartApp* app);
    virtual ~CCAppServer();
    virtual wxConnectionBase* OnAcceptConnection(const wxString& topic);

    void OpenFile(const wxString& filename);

    // should only be called by the connection
    void DestroyConnection(CCAppServerConnection* connection);

private:
    CCAppServer(CalChartApp* app)
        : mApp(app)
    {
    }

    // maintain a list of connections to disconnect from on d-tor
    std::vector<CCAppServerConnection*> mActiveConnections;

    CalChartApp* mApp;
};

class CCAppClient : public wxClient {
public:
    static std::unique_ptr<CCAppClient> MakeClient();
    virtual ~CCAppClient();
    virtual wxConnectionBase* OnMakeConnection();

    void OpenFile(const wxString& filename);

    // should only be called by the connection
    void DestroyConnection();

private:
    CCAppClient()
        : mConnection(nullptr)
    {
    }

    bool Connect(const wxString& host, const wxString& service,
        const wxString& topic);
    bool IsConnected();
    void Disconnect();

    // mConnection may be null, depending on the state of the connection
    CCAppClientConnection* mConnection;
};

// Connection pair:
// Servers will get Poked, then they turn around and ask the server object to
// open it.
// Clients will Poke the servers to open files for them.
class CCAppServerConnection : public wxConnection {
    using super = wxConnection;

public:
    CCAppServerConnection(CCAppServer* server)
        : mServer(server)
    {
    }

    // ServerConnection: OnPoke, ask the server to open a file.
    virtual bool OnPoke(const wxString& topic, const wxString& item,
        const void* data, size_t size, wxIPCFormat format)
    {
        if (item == OPEN_FILE) {
            mServer->OpenFile(wxString((const char*)data, size - 1));
        }
        return super::OnPoke(topic, item, data, size, format);
    }

    virtual bool OnDisconnect()
    {
        mServer->DestroyConnection(this);
        return super::OnDisconnect();
    }

private:
    CCAppServer* mServer;
};

class CCAppClientConnection : public wxConnection {
    using super = wxConnection;

public:
    CCAppClientConnection(CCAppClient* client)
        : mClient(client)
    {
    }

    // ClientConnect: Poke the server to open a file.
    void OpenFile(const wxString& filename) { Poke(OPEN_FILE, filename); }

    virtual bool OnDisconnect()
    {
        mClient->DestroyConnection();
        return super::OnDisconnect();
    }

private:
    CCAppClient* mClient;
};

wxString CCAppServer::GetServerName()
{
    wxString name = wxGetUserId() + wxStandardPaths::Get().GetExecutablePath();
    std::hash<std::string> nameHasher;
    auto hash = nameHasher(name.ToStdString());
    int16_t sizedHash = hash;
    wxString returnVal;
    returnVal << sizedHash;
    return returnVal;
}

std::unique_ptr<CCAppServer> CCAppServer::MakeServer(CalChartApp* app)
{
    std::unique_ptr<CCAppServer> newServer{ new CCAppServer(app) };
    if (!newServer->Create(GetServerName())) {
        return nullptr;
    }
    return newServer;
}

CCAppServer::~CCAppServer()
{
    while (mActiveConnections.size() > 0) {
        // this should cause the connection to turn around and call
        // DestroyConnection()
        mActiveConnections.front()->Disconnect();
    }
}

void CCAppServer::OpenFile(const wxString& filename)
{
    mApp->OpenFile(filename);
}

wxConnectionBase* CCAppServer::OnAcceptConnection(const wxString&)
{
    CCAppServerConnection* newConnection = new CCAppServerConnection(this);
    mActiveConnections.push_back(newConnection);
    return newConnection;
}

void CCAppServer::DestroyConnection(CCAppServerConnection* connection)
{
    mActiveConnections.erase(std::remove(mActiveConnections.begin(),
        mActiveConnections.end(), connection));
}

std::unique_ptr<CCAppClient> CCAppClient::MakeClient()
{
    std::unique_ptr<CCAppClient> newClient{ new CCAppClient() };
    if (newClient->Connect("localhost", CCAppServer::GetServerName(), "Client")) {
        return newClient;
    }
    return nullptr;
}

void CCAppClient::OpenFile(const wxString& filename)
{
    if (mConnection != nullptr) {
        mConnection->OpenFile(filename);
    }
}

bool CCAppClient::IsConnected() { return mConnection != nullptr; }

void CCAppClient::Disconnect()
{
    if (mConnection != nullptr) {
        mConnection->Disconnect();
    }
}

bool CCAppClient::Connect(const wxString& host, const wxString& service,
    const wxString& topic)
{
    auto oldLogLevel = wxLog::GetLogLevel();
    wxLog::SetLogLevel(wxLOG_FatalError); // Make sure that we don't throw errors
    // to the user if the connection fails;
    // we expect connections to fail when we
    // are testing whether or not there is a
    // server, and the user shouldn't be
    // informed when we cannot find a server
    mConnection = (CCAppClientConnection*)MakeConnection(host, service, topic);
    wxLog::SetLogLevel(oldLogLevel);
    return IsConnected();
}

wxConnectionBase* CCAppClient::OnMakeConnection()
{
    return new CCAppClientConnection(this);
}

void CCAppClient::DestroyConnection() { mConnection = nullptr; }

CCAppClient::~CCAppClient() { Disconnect(); }

// Server, Client, and Independent are all derived from HostAppInterface.
// Mostly it's all boilerplate, with "smarts" done in the Make functions
class ServerSideHostAppInterface : public HostAppInterface {
public:
    static std::unique_ptr<HostAppInterface>
    Make(CalChartApp* app, StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop);

    virtual ~ServerSideHostAppInterface() { m_serverStartStop.second(); }

    virtual bool OnInit()
    {
        m_serverStartStop.first();
        return true;
    }

    virtual void OpenFile(const wxString& filename)
    {
        mServer->OpenFile(filename);
    }

private:
    ServerSideHostAppInterface(std::unique_ptr<CCAppServer> server,
        StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop)
        : HostAppInterface(serverStartStop, clientStartStop)
        , mServer(std::move(server))
    {
    }

    std::unique_ptr<CCAppServer> mServer;
};

class ClientSideHostAppInterface : public HostAppInterface {
public:
    static std::unique_ptr<HostAppInterface>
    Make(StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop);

    virtual ~ClientSideHostAppInterface() { m_clientStartStop.second(); }

    virtual bool OnInit()
    {
        m_clientStartStop.first();
        return false;
    }

    virtual void OpenFile(const wxString& filename)
    {
        mClient->OpenFile(filename);
    }

private:
    ClientSideHostAppInterface(std::unique_ptr<CCAppClient> client,
        StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop)
        : HostAppInterface(serverStartStop, clientStartStop)
        , mClient(std::move(client))
    {
    }

    std::unique_ptr<CCAppClient> mClient;
};

class IndependentHostAppInterface : public HostAppInterface {
public:
    static std::unique_ptr<HostAppInterface>
    Make(CalChartApp* app, StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop);

    virtual ~IndependentHostAppInterface() { m_serverStartStop.second(); }

    virtual bool OnInit()
    {
        m_serverStartStop.first();
        return true;
    }

    virtual void OpenFile(const wxString& filename) { mApp->OpenFile(filename); }

private:
    IndependentHostAppInterface(CalChartApp* app, StartStopFunc_t serverStartStop,
        StartStopFunc_t clientStartStop)
        : HostAppInterface(serverStartStop, clientStartStop)
        , mApp(app)
    {
    }

    CalChartApp* mApp;
};

// Linch-pin function that ties everything together
std::unique_ptr<HostAppInterface>
HostAppInterface::Make(CalChartApp* app, StartStopFunc_t serverStartStop,
    StartStopFunc_t clientStartStop)
{
#ifdef __APPLE__
    return IndependentHostAppInterface::Make(app, serverStartStop,
        clientStartStop);
#else
    auto client = ClientSideHostAppInterface::Make(serverStartStop, clientStartStop);
    if (client) {
        return client;
    }
    return ServerSideHostAppInterface::Make(app, serverStartStop,
        clientStartStop);
#endif
}

HostAppInterface::HostAppInterface(StartStopFunc_t serverStartStop,
    StartStopFunc_t clientStartStop)
    : m_serverStartStop(serverStartStop)
    , m_clientStartStop(clientStartStop)
{
}

HostAppInterface::~HostAppInterface() { }

std::unique_ptr<HostAppInterface>
ServerSideHostAppInterface::Make(CalChartApp* app,
    StartStopFunc_t serverStartStop,
    StartStopFunc_t clientStartStop)
{
    auto server = CCAppServer::MakeServer(app);
    if (server != nullptr) {
        return std::unique_ptr<HostAppInterface>{ new ServerSideHostAppInterface(
            std::move(server), serverStartStop, clientStartStop) };
    }
    return nullptr;
}

std::unique_ptr<HostAppInterface>
ClientSideHostAppInterface::Make(StartStopFunc_t serverStartStop,
    StartStopFunc_t clientStartStop)
{
    auto client = CCAppClient::MakeClient();
    if (client != nullptr) {
        return std::unique_ptr<HostAppInterface>{ new ClientSideHostAppInterface(
            std::move(client), serverStartStop, clientStartStop) };
    }
    return nullptr;
}

std::unique_ptr<HostAppInterface>
IndependentHostAppInterface::Make(CalChartApp* app,
    StartStopFunc_t serverStartStop,
    StartStopFunc_t clientStartStop)
{
    return std::unique_ptr<HostAppInterface>{
        new IndependentHostAppInterface(app, serverStartStop, clientStartStop)
    };
}
