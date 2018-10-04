#include "stdafx.h"
#include "GomokuLobby.h"
#include "GomokuRoom.h"



GomokuLobby::GomokuLobby()
	: m_roomList(10, nullptr)
{
}
GomokuLobby::~GomokuLobby()
{
}



bool GomokuLobby::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		arJSON iJSON;
		Message msg = Message::Invalid;
		{
			recvData[recvResult] = NULL;
			if (JSON_To_arJSON(recvData.Buffer(), iJSON))
			{
				server_log_error("JSON Errored by received message from " << user.ToStr() << " in Lobby "<< endl);
				return true;
			}
		
			if ((msg = CheckMessage(iJSON["Message"].Str())) == Message::Invalid)
			{
				server_log_error("An undefined message(" << static_cast<int>(msg) << ") was received by Lobby from the "<< user.ToStr() << endl);
				return false;
			}
		}
		mutex_lock_guard actionLocker(m_mtxMsgProcessing);
		
		switch (msg)
		{
		case GomokuLobby::Message::LeaveLobby:	if (LeaveLobby(user, iJSON))	return true; break;
		case GomokuLobby::Message::CreateRoom:	if (CreateRoom(user, iJSON))	return true; break;
		case GomokuLobby::Message::EnterRoom:	if (EnterRoom(user, iJSON))		return true; break;
		default:
			server_log_error("An undefined message(" << static_cast<int>(msg) << ") was received by Lobby from the " << user.ToStr() << endl);
			break;
		}
	}
	else
	{
		mutex_lock_guard actionLocker(m_mtxMsgProcessing);

		DetachConnectorReturner(user);
		RemoveUserInUserList(user);
		server_log_note("User(" << user.ToStr() << ") disconnected." << endl);
		return true;
	}
	return false;
}

//딱 보면 앎
GomokuLobby::Message GomokuLobby::CheckMessage(const std::string & msg) const
{
	if (msg.empty())				return Message::Invalid;
	else if (msg == "LeaveLobby")	return Message::LeaveLobby;
	else if (msg == "CreateRoom")	return Message::CreateRoom;
	else if (msg == "EnterRoom")	return Message::EnterRoom;
	else							return Message::Invalid;
}

bool GomokuLobby::RemoveUserInUserList(AsyncConnector & user)
{
	auto iter = m_userList.find(&user);
	if (iter != m_userList.end())
	{
		m_userList.erase(iter);
		return true;
	}
	server_log_error("RemoveUserInUserList() was called for a user(" << user.ToStr() << ") who is not in the user list." << endl);
	return false;
}




bool GomokuLobby::CreateRoom(AsyncConnector & user, const arJSON & iJSON)
{
	std::string name, password;
	{
		const arJSON& roomJSON = iJSON["Room"].Sub();
		if (!roomJSON.IsIn("Name"))
		{
			server_log_error("The JSON parameter received from the user(" << user.ToStr() << ") in the Lobby CreateRoom() is abnormal." << endl);
			return false;
		}

		name = roomJSON["Name"].Str();
		if (roomJSON.IsIn("Password"))
			password = roomJSON["Password"].Str();
	}

	int id = GetEmptyRoomID();
	if (id < 0)
	{
		server_log_error("Room(" << id << ") already exists. Room creation was denied." << endl);
		return false;
	}

	return CreateRoom(user, id, name, password);
}

bool GomokuLobby::EnterRoom(AsyncConnector & user, const arJSON & iJSON)
{
	int id = NULL;
	std::string password;
	if (iJSON.IsIn("Room"))
	{
		const arJSON& roomJSON = iJSON["Room"].Sub();

		if (!roomJSON.IsIn("ID"))
		{
			server_log_error("The JSON parameter received from the user(" << user.ToStr() << ") in the Lobby EnterRoom() is abnormal." << endl);
			return false;
		}
		id = roomJSON["ID"].Int();
		if (roomJSON.IsIn("Password"))
			password = roomJSON["Password"].Str();
	}

	return EnterRoom(user, id, password);
}

bool GomokuLobby::LeaveLobby(AsyncConnector & user, const arJSON & iJSON)
{
	//Nothing
	return LeaveLobby(user);
}



//비활성화 된 방의 번호를 반환
int GomokuLobby::GetEmptyRoomID() const
{
	for (int i = 0; i < m_roomList.size(); i++)
	{
		if (!m_roomList[i])
			return i;
	}
	return -1;
}

bool GomokuLobby::EnterLobby(AsyncConnector& user)
{
	mutex_lock_guard actionLocker(m_mtxMsgProcessing);

	m_userList.insert(&user);
	AttachConnectorReturner(user);
	server_log_note("The user(" << user.ToStr() << ") has joined the lobby" << endl);

	BCEnterLobby(user);
	return true;
}

void GomokuLobby::DestroyRoom(int id)
{
	mutex_lock_guard actionLocker(m_mtxMsgProcessing);
	GomokuRoom*& room = m_roomList[id];
	if (room)
	{
		delete room;
		room = nullptr;

		BCDestroyRoom(id);
		server_log_note("Room(" << id << ") deleted" << endl);
	}
	else
		server_log_error("Attempted to delete a nonexistent room(" << id << ')' << endl);
}

bool GomokuLobby::CreateRoom(AsyncConnector & user, int id, const std::string & name, const std::string & password)
{
	if (id < 0 || m_roomList.size() <= id)
	{
		server_log_error("Room(" << id << ") can not be created." << endl);
		return false;
	}

	GomokuRoom*& room = m_roomList[id];
	if (room != nullptr)
	{
		server_log_error("Room(" << id << ") already exists. Room creation was denied." << endl);
		return false;
	}


	RemoveUserInUserList(user);
	DetachConnectorReturner(user);
	room = new GomokuRoom(*this, &user, id, name, password);

	BCCreateRoom(user, id, name, password.size());
	server_log_note("Room(" << id << ") created" << endl);
	return false;
}

bool GomokuLobby::EnterRoom(AsyncConnector & user, int id, const std::string & password)
{
	if (id >= m_roomList.size())
	{
		server_log_error("Room ID out of range from EnterRoom(). ID:" << id << endl);
		return false;
	}
	GomokuRoom*& room = m_roomList[id];
	if (!room)
	{
		server_log_error("There is no room for that number. ID:" << id << endl);
		return false;
	}


	DetachConnectorReturner(user);
	if (!room->EnterRoom(&user, password))
	{
		AttachConnectorReturner(user);
		return false;
	}

	BCEnterRoom(user, room->ID(), room->Name(), room->IsLocked());
	server_log_note("EnterRoom success. User:" << user.ToStr() << endl);
	return false;
}

bool GomokuLobby::LeaveLobby(AsyncConnector & user)
{
	DetachConnectorReturner(user);
	if (!RemoveUserInUserList(user))
		return true;

	BCLeaveLobby(user);
	server_log_note("The user(" << user.ToStr() << ") has left the lobby." << endl);
	return true;
}




void GomokuLobby::BCEnterLobby(AsyncConnector & user) const
{
	arJSON oJSON;
	oJSON["Message"] = "RoomListRefresh";

	for (auto iter : m_roomList)
	{
		if (!iter)
			continue;

		arJSON roomJSON;
		{
			roomJSON["ID"] = iter->ID();
			roomJSON["Name"] = iter->Name();
			roomJSON["Locked"] = (iter->IsLocked() ? 1 : 0);
			roomJSON["State"] = static_cast<int>(iter->State());
		}
		oJSON["RoomList"].Append(roomJSON);
	}
	__ar_send(user, oJSON);
}

void GomokuLobby::BCDestroyRoom(int id) const
{
	arJSON oJSON;
	{
		oJSON["Message"] = "RoomDestroyed";
		oJSON["RoomDestroyed"] = id;
	}
	for (auto iter : m_userList)
		__ar_send(*iter, oJSON);
}

void GomokuLobby::BCCreateRoom(AsyncConnector & user, int id, const std::string & name, bool isLocked) const
{
	arJSON roomJSON;
	{
		roomJSON["ID"] = id;
		roomJSON["Name"] = name;
		roomJSON["Locked"] = (isLocked ? 1 : 0);
	}

	//Host
	BCEnterRoom(user, roomJSON);

	//Lobby user
	arJSON oJSON;
	{
		oJSON["Message"] = "RoomCreated";
		oJSON["Room"] = roomJSON;

		for (auto iter : m_userList)
			__ar_send(*iter, oJSON);
	}
}

void GomokuLobby::BCEnterRoom(AsyncConnector & user, const arJSON & roomJSON) const
{
	arJSON oJSON;
	oJSON["Message"] = "RoomEntered";
	oJSON["Room"] = roomJSON;
	__ar_send(user, oJSON);
}

void GomokuLobby::BCEnterRoom(AsyncConnector & user, int id, const std::string & name, bool isLocked) const
{
	arJSON roomJSON;
	{
		roomJSON["ID"] = id;
		roomJSON["Name"] = name;
		roomJSON["Locked"] = (isLocked ? 1 : 0);
	}
	return BCEnterRoom(user, roomJSON);
}

void GomokuLobby::BCLeaveLobby(AsyncConnector & user) const
{
	arJSON oJSON;
	{
		oJSON["Message"] = "LobbyLeaved";
		oJSON["Result"] = 1;
	}
	__ar_send(user, oJSON);
}

void GomokuLobby::BCRoomUpdate(int id, GomokuRoomState state) const
{
	arJSON oJSON;
	{
		oJSON["Message"] = "RoomUpdate";
		arJSON roomJSON;
		{
			roomJSON["ID"] = id;
			roomJSON["State"] = static_cast<int>(state);
		}
		oJSON["Room"] = roomJSON;


		for (auto iter : m_userList)
			__ar_send(*iter, oJSON);
	}
}
