#pragma once
#include <string>

class GomokuLobby;
class GomokuRoom
{
public:
	GomokuRoom(GomokuLobby& lobby, AsyncConnector* host, int id, const std::string& name, const std::string& password);
	~GomokuRoom();

	bool EnterRoom(AsyncConnector* guest, const std::string& password);

private:
	bool MessageProcess(AsyncConnector&, int, SocketBuffer&);

	bool LeaveRoom(AsyncConnector& user, const arJSON& iJSON);

private:
	int m_id;
	std::string m_name;
	std::string m_password;

	GomokuLobby& m_lobby;
	AsyncConnector* m_host;
	AsyncConnector* m_guest;

	std::mutex m_mtxHost;
	std::mutex m_mtxGuest;
};

