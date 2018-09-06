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
	bool MessageProcess(AsyncConnector&, int, SocketBuffer&);

	bool CreateRoom(AsyncConnector& user, const arJSON& iJSON);
	bool EnterRoom(AsyncConnector& user, const arJSON& iJSON);
	bool LeaveLobby(AsyncConnector& user, const arJSON& iJSON);


private:
	std::set<AsyncConnector*> m_userList;
	std::vector<GomokuRoom*> m_roomList;
	std::mutex m_mtxUserList;
	std::mutex m_mtxRoomList;
};

