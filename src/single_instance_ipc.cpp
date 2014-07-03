#include <functional>

#include "single_instance_ipc.h"
#include "calchartapp.h"

#include <wx/utils.h>
#include <wx/stdpaths.h>

const wxString OPEN_FILE = "OpenFile";


ServerSideHostAppInterface* ServerSideHostAppInterface::Make(CalChartApp* app) {
	CCAppServer* server = CCAppServer::MakeServer(app);
	if (server != nullptr) {
		return new ServerSideHostAppInterface(server);
	} else {
		return nullptr;
	}
}

void ServerSideHostAppInterface::OpenFile(const wxString& filename) {
	mServer->OpenFile(filename);
}

ServerSideHostAppInterface::ServerSideHostAppInterface(CCAppServer* server)
: mServer(server)
{}

ServerSideHostAppInterface::~ServerSideHostAppInterface() {
	delete mServer;
}

ClientSideHostAppInterface* ClientSideHostAppInterface::Make() {
	CCAppClient* client = CCAppClient::MakeClient();
	if (client != nullptr) {
		return new ClientSideHostAppInterface(client);
	} else {
		return nullptr;
	}
}

void ClientSideHostAppInterface::OpenFile(const wxString& filename) {
	mClient->OpenFile(filename);
}

ClientSideHostAppInterface::ClientSideHostAppInterface(CCAppClient* client)
: mClient(client)
{}

ClientSideHostAppInterface::~ClientSideHostAppInterface() {
	delete mClient;
}

IndependentHostAppInterface::IndependentHostAppInterface(CalChartApp* app)
: mApp(app)
{}

void IndependentHostAppInterface::OpenFile(const wxString& filename) {
	mApp->OpenFile(filename);
}

CCAppClient* CCAppClient::MakeClient() {
	CCAppClient* newClient = new CCAppClient();
	newClient->Connect("localhost", CCAppServer::GetServerName(), "Client");
	if (!newClient->IsConnected()) {
		delete newClient;
		return nullptr;
	} else {
		return newClient;
	}
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

CCAppServer* CCAppServer::MakeServer(CalChartApp* app) {
	CCAppServer* newServer = new CCAppServer(app);
	if (!newServer->Create(GetServerName())) {
		delete newServer;
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

