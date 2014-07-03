#ifndef _SINGLE_INSTANCE_IPC_H_
#define _SINGLE_INSTANCE_IPC_H_



#include <vector>
#include <wx/ipc.h>



class CalChartApp;
class CCAppClientConnection;
class CCAppServerConnection;

 

class CCAppClient : public wxClient {
public:
	~CCAppClient();

	static CCAppClient* MakeClient();

	void OpenFile(const wxString& filename);

	void Disconnect();

	bool IsConnected();

	bool Connect(const wxString& host, const wxString& service, const wxString& topic);
	wxConnectionBase *OnMakeConnection();
private:
	void DestroyConnection();

	CCAppClient();
	

	CCAppClientConnection* mConnection;

	friend class CCAppClientConnection;
};

class CCAppServer : public wxServer {
public:
	~CCAppServer();

	static wxString GetServerName();

	static CCAppServer* MakeServer(CalChartApp* app);

	void OpenFile(const wxString& filename);

	virtual wxConnectionBase *OnAcceptConnection(const wxString& topic);
private:
	CCAppServer(CalChartApp* app);
	

	void DisconnectAll();
	void DestroyConnection(CCAppServerConnection* connection);

	int GetConnectionIndex(CCAppServerConnection* connection);

	std::vector<CCAppServerConnection*> mActiveConnections;

	CalChartApp* mApp;

	friend class CCAppServerConnection;
};


class CCAppConnection : public wxConnection {
public:
	virtual void OpenFile(const wxString& filename) = 0;
};


class CCAppServerConnection : public CCAppConnection {
private:
	using super = CCAppConnection;
public:
	CCAppServerConnection(CCAppServer* server);

	virtual void OpenFile(const wxString& filename);

	virtual bool OnPoke(const wxString& topic, const wxString& item, const void *data, size_t size, wxIPCFormat format);

	virtual bool OnDisconnect();
private:
	CCAppServer* mServer;
};


class CCAppClientConnection : public CCAppConnection {
private:
	using super = CCAppConnection;
public:
	CCAppClientConnection(CCAppClient* client);

	virtual void OpenFile(const wxString& filename);

	virtual bool OnDisconnect();
private:
	CCAppClient* mClient;
};



class HostAppInterface {
public:
	virtual void OpenFile(const wxString& filename) = 0;
};

class ServerSideHostAppInterface : public HostAppInterface {
public:
	~ServerSideHostAppInterface();

	static ServerSideHostAppInterface* Make(CalChartApp* app);

	virtual void OpenFile(const wxString& filename);
private:
	ServerSideHostAppInterface(CCAppServer* server);
	

	CCAppServer* mServer;
};


class ClientSideHostAppInterface : public HostAppInterface {
public:
	~ClientSideHostAppInterface();

	static ClientSideHostAppInterface* Make();

	virtual void OpenFile(const wxString& filename);
private:
	ClientSideHostAppInterface(CCAppClient* client);
	

	CCAppClient* mClient;
};

class IndependentHostAppInterface : public HostAppInterface {
public:
	IndependentHostAppInterface(CalChartApp* app);

	virtual void OpenFile(const wxString& filename);
private:
	CalChartApp* mApp;
};

#endif