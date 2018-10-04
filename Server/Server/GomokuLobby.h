#pragma once
#include <set>
#include <vector>

class User;
class GomokuRoom;
enum class GomokuRoomState;

class GomokuLobby
{
private:
	enum class Message
	{
		LeaveLobby,
		CreateRoom,
		EnterRoom,

		Invalid = -1
	};

public:
	GomokuLobby();
	~GomokuLobby();

	//Action
	bool EnterLobby(AsyncConnector& user);
	void DestroyRoom(int id);

	//Broadcast
	void BCRoomUpdate(int id, GomokuRoomState state) const;

private:
	inline void AttachConnectorReturner(AsyncConnector& connector)
	{
		connector.Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline void DetachConnectorReturner(AsyncConnector& connector) const { connector.Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);
	Message CheckMessage(const std::string& msg) const;
	bool RemoveUserInUserList(AsyncConnector& user);

	//JSON analysis
	bool CreateRoom(AsyncConnector& user, const arJSON& iJSON);
	bool EnterRoom(AsyncConnector& user, const arJSON& iJSON);
	bool LeaveLobby(AsyncConnector& user, const arJSON& iJSON);

	//Action
	int GetEmptyRoomID() const;
	bool CreateRoom(AsyncConnector& user, int id, const std::string& name, const std::string& password);
	bool EnterRoom(AsyncConnector& user, int id, const std::string& password);
	bool LeaveLobby(AsyncConnector& user);

	//Broadcast
	void BCEnterLobby(AsyncConnector& user) const;
	void BCDestroyRoom(int id) const;

	void BCCreateRoom(AsyncConnector& user, int id, const std::string& name, bool isLocked) const;
	void BCEnterRoom(AsyncConnector& user, int id, const std::string& name, bool isLocked) const;
	void BCEnterRoom(AsyncConnector& user, const arJSON& roomJSON) const;
	void BCLeaveLobby(AsyncConnector& user) const;


	arJSON RoomToJSON(GomokuRoom& roomData);

private:
	std::set<AsyncConnector*> m_userList;
	std::vector<GomokuRoom*> m_roomList;
	mutable std::mutex m_mtxMsgProcessing;
};

