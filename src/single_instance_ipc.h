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

	static std::unique_ptr<CCAppClient> MakeClient();

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

	static std::unique_ptr<CCAppServer> MakeServer(CalChartApp* app);

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


using StartStopFunc_t = std::pair<std::function<void()>, std::function<void()>>;
class HostAppInterface {
public:
	static std::unique_ptr<HostAppInterface> Make(CalChartApp* app,
												  StartStopFunc_t serverStartStop,
												  StartStopFunc_t clientStartStop);
protected:
	HostAppInterface(StartStopFunc_t serverStartStop,
					 StartStopFunc_t clientStartStop);
	StartStopFunc_t m_serverStartStop;
	StartStopFunc_t m_clientStartStop;
public:
	virtual ~HostAppInterface(); // calls the exit function

	virtual bool OnInit() = 0;
	virtual void OpenFile(const wxString& filename) = 0;
};

class ServerSideHostAppInterface : public HostAppInterface {
public:
	virtual ~ServerSideHostAppInterface();

	static std::unique_ptr<HostAppInterface> Make(CalChartApp* app,
												  StartStopFunc_t serverStartStop,
												  StartStopFunc_t clientStartStop);

	virtual bool OnInit();
	virtual void OpenFile(const wxString& filename);
private:
	ServerSideHostAppInterface(std::unique_ptr<CCAppServer> server,
							   StartStopFunc_t serverStartStop,
							   StartStopFunc_t clientStartStop);
	

	std::unique_ptr<CCAppServer> mServer;
};


class ClientSideHostAppInterface : public HostAppInterface {
public:
	virtual ~ClientSideHostAppInterface();

	static std::unique_ptr<HostAppInterface> Make(StartStopFunc_t serverStartStop,
												  StartStopFunc_t clientStartStop);

	virtual bool OnInit();
	virtual void OpenFile(const wxString& filename);
private:
	ClientSideHostAppInterface(std::unique_ptr<CCAppClient> client,
							   StartStopFunc_t serverStartStop,
							   StartStopFunc_t clientStartStop);
	

	std::unique_ptr<CCAppClient> mClient;
};

class IndependentHostAppInterface : public HostAppInterface {
public:
	virtual ~IndependentHostAppInterface();

	static std::unique_ptr<HostAppInterface> Make(CalChartApp* app,
												  StartStopFunc_t serverStartStop,
												  StartStopFunc_t clientStartStop);
	
	virtual bool OnInit();
	virtual void OpenFile(const wxString& filename);
private:
	IndependentHostAppInterface(CalChartApp* app,
								StartStopFunc_t serverStartStop,
								StartStopFunc_t clientStartStop);

	CalChartApp* mApp;
};

#endif