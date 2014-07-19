#include <functional>

#include "single_instance_ipc.h"
#include "calchartapp.h"

#include <wx/utils.h>
#include <wx/stdpaths.h>

const wxString OPEN_FILE = "OpenFile";


std::unique_ptr<HostAppInterface>
HostAppInterface::Make(CalChartApp* app,
					   StartStopFunc_t serverStartStop,
					   StartStopFunc_t clientStartStop)
{
#ifdef __APPLE__
	return IndependentHostAppInterface::Make(app, serverStartStop, clientStartStop);
#else
	auto client = ClientSideHostAppInterface::Make(serverStartStop, clientStartStop);
	if (client == nullptr) {
		mHostInterface = ServerSideHostAppInterface::Make(app, serverStartStop, clientStartStop);
	} else {
		mHostInterface = client;
	}
#endif
}

HostAppInterface::HostAppInterface(StartStopFunc_t serverStartStop, StartStopFunc_t clientStartStop) :
m_serverStartStop(serverStartStop),
m_clientStartStop(clientStartStop)
{
}

HostAppInterface::~HostAppInterface()
{
}

std::unique_ptr<HostAppInterface> ServerSideHostAppInterface::Make(CalChartApp* app,
																   StartStopFunc_t serverStartStop,
																   StartStopFunc_t clientStartStop)
{
	auto server = CCAppServer::MakeServer(app);
	if (server != nullptr) {
		return std::unique_ptr<HostAppInterface>{new ServerSideHostAppInterface(std::move(server), serverStartStop, clientStartStop)};
	}
	return nullptr;
}

bool ServerSideHostAppInterface::OnInit() {
	m_serverStartStop.first();
	return true;
}

void ServerSideHostAppInterface::OpenFile(const wxString& filename) {
	mServer->OpenFile(filename);
}

ServerSideHostAppInterface::ServerSideHostAppInterface(std::unique_ptr<CCAppServer> server,
													   StartStopFunc_t serverStartStop,
													   StartStopFunc_t clientStartStop) :
HostAppInterface(serverStartStop, clientStartStop),
mServer(std::move(server))
{}

ServerSideHostAppInterface::~ServerSideHostAppInterface()
{
	m_serverStartStop.second();
}

std::unique_ptr<HostAppInterface> ClientSideHostAppInterface::Make(StartStopFunc_t serverStartStop,
																   StartStopFunc_t clientStartStop) {
	auto client = CCAppClient::MakeClient();
	if (client != nullptr) {
		return std::unique_ptr<HostAppInterface>{new ClientSideHostAppInterface(std::move(client), serverStartStop, clientStartStop)};
	}
	return nullptr;
}

bool ClientSideHostAppInterface::OnInit() {
	m_clientStartStop.first();
	return false;
}

void ClientSideHostAppInterface::OpenFile(const wxString& filename) {
	mClient->OpenFile(filename);
}

ClientSideHostAppInterface::ClientSideHostAppInterface(std::unique_ptr<CCAppClient> client,
													   StartStopFunc_t serverStartStop,
													   StartStopFunc_t clientStartStop) :
HostAppInterface(serverStartStop, clientStartStop),
mClient(std::move(client))
{}

ClientSideHostAppInterface::~ClientSideHostAppInterface()
{
	m_clientStartStop.second();
}

std::unique_ptr<HostAppInterface> IndependentHostAppInterface::Make(CalChartApp* app,
																	StartStopFunc_t serverStartStop,
																	StartStopFunc_t clientStartStop)
{
	return std::unique_ptr<HostAppInterface>{ new IndependentHostAppInterface(app, serverStartStop, clientStartStop) };
}

IndependentHostAppInterface::IndependentHostAppInterface(CalChartApp* app,
														 StartStopFunc_t serverStartStop,
														 StartStopFunc_t clientStartStop) :
HostAppInterface(serverStartStop, clientStartStop),
mApp(app)
{}

bool IndependentHostAppInterface::OnInit() {
	m_serverStartStop.first();
	return true;
}

void IndependentHostAppInterface::OpenFile(const wxString& filename) {
	mApp->OpenFile(filename);
}

IndependentHostAppInterface::~IndependentHostAppInterface()
{
	m_serverStartStop.second();
}



std::unique_ptr<CCAppClient> CCAppClient::MakeClient() {
	std::unique_ptr<CCAppClient> newClient { new CCAppClient() };
	newClient->Connect("localhost", CCAppServer::GetServerName(), "Client");
	if (!newClient->IsConnected()) {
		return nullptr;
	}
	return newClient;
}

void CCAppClient::OpenFile(const wxString& filename) {
	if (mConnection != nullptr) {
		mConnection->OpenFile(filename);
	}
}

bool CCAppClient::IsConnected() {
	return mConnection != nullptr;
}


void CCAppClient::Disconnect() {
	if (mConnection != nullptr) {
		mConnection->Disconnect();
	}
}

bool CCAppClient::Connect(const wxString& host, const wxString& service, const wxString& topic) {
	mConnection = (CCAppClientConnection*)MakeConnection(host, service, topic);
	return IsConnected();
}

wxConnectionBase* CCAppClient::OnMakeConnection() {
	return new CCAppClientConnection(this);
}

void CCAppClient::DestroyConnection() {
	mConnection = nullptr;
}

CCAppClient::CCAppClient() 
: mConnection(nullptr)
{}

CCAppClient::~CCAppClient() {
	Disconnect();
}

wxString CCAppServer::GetServerName() {
	wxString name = wxGetUserId() + wxStandardPaths::Get().GetExecutablePath();
	std::hash<std::string> nameHasher;
	unsigned int hash = nameHasher(name.ToStdString());
	int16_t sizedHash = hash;
	wxString returnVal;
	returnVal << sizedHash;
	return returnVal;
}

std::unique_ptr<CCAppServer> CCAppServer::MakeServer(CalChartApp* app) {
	std::unique_ptr<CCAppServer> newServer { new CCAppServer(app) };
	if (!newServer->Create(GetServerName())) {
		return nullptr;
	}
	return newServer;
}

CCAppServer::CCAppServer(CalChartApp* app)
: mApp(app)
{}

CCAppServer::~CCAppServer() {
	DisconnectAll();
}

void CCAppServer::OpenFile(const wxString& filename) {
	mApp->OpenFile(filename);
}

wxConnectionBase *CCAppServer::OnAcceptConnection(const wxString& topic) {
	CCAppServerConnection* newConnection = new CCAppServerConnection(this);
	mActiveConnections.push_back(newConnection);
	return newConnection;
}

void CCAppServer::DisconnectAll() {
	while (mActiveConnections.size() > 0) {
		mActiveConnections.front()->Disconnect();
	}
}

void CCAppServer::DestroyConnection(CCAppServerConnection* connection) {
	int connectionIndex = GetConnectionIndex(connection);
	if (connectionIndex >= 0) {
		mActiveConnections.erase(mActiveConnections.begin() + GetConnectionIndex(connection));
	}
}

int CCAppServer::GetConnectionIndex(CCAppServerConnection* connection) {
	for (int index = 0; index < mActiveConnections.size(); index++) {
		if (mActiveConnections[index] == connection) {
			return index;
		}
	}
	return -1;
}



CCAppServerConnection::CCAppServerConnection(CCAppServer* server)
{
	mServer = server;
}

void CCAppServerConnection::OpenFile(const wxString& filename) {
	mServer->OpenFile(filename);
}

bool CCAppServerConnection::OnPoke(const wxString& topic, const wxString& item, const void *data, size_t size, wxIPCFormat format) {
	if (item == OPEN_FILE) {
		OpenFile(wxString((const char*)data, size - 1));
	}
	return super::OnPoke(topic, item, data, size, format);
}

bool CCAppServerConnection::OnDisconnect() {
	mServer->DestroyConnection(this);
	return super::OnDisconnect();
}

CCAppClientConnection::CCAppClientConnection(CCAppClient* client)
{
	mClient = client;
}

void CCAppClientConnection::OpenFile(const wxString& filename) {
	Poke(OPEN_FILE, filename);
}

bool CCAppClientConnection::OnDisconnect() {
	mClient->DestroyConnection();
	return super::OnDisconnect();
}

