#pragma once
#include <set>
#include <vector>

class User;
class GomokuRoom;
class GomokuLobby
{
public:
	GomokuLobby();
	~GomokuLobby();

	void EnterLobby(AsyncConnector* user);
	void DestroyRoom(int id);

private:
	inline void AttachConnectorReturner(AsyncConnector& connector)
	{
		connector.Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline void DetachConnectorReturner(AsyncConnector& connector) { connector.Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);

	bool CreateRoom(AsyncConnector& user, const arJSON& iJSON);
	bool CreateRoom(AsyncConnector& user, int id, const std::string& name, const std::string& password, bool noMutex = false);
	bool EnterRoom(AsyncConnector& user, const arJSON& iJSON);
	bool LeaveLobby(AsyncConnector& user, const arJSON& iJSON);


	bool RegistedUserRemove(AsyncConnector& user);

	arJSON RoomToJSON(GomokuRoom& roomData);

private:
	std::set<AsyncConnector*> m_userList;
	std::vector<GomokuRoom*> m_roomList;
	std::mutex m_mtxUserList;
	std::mutex m_mtxRoomList;
};

