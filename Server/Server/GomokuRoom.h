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
	inline void AttachConnectorReturner(AsyncConnector& connector)
	{
		connector.Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline void DetachConnectorReturner(AsyncConnector& connector) { connector.Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);

	bool LeaveRoom(AsyncConnector& user);

private:
	int m_id;
	std::string m_name;
	std::string m_password;

	GomokuLobby& m_lobby;
	AsyncConnector* m_host;
	AsyncConnector* m_guest;

	std::mutex m_mtxEnterLeave;
	std::mutex m_mtxHost;
	std::mutex m_mtxGuest;
};

