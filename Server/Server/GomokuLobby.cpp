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



void GomokuLobby::EnterLobby(AsyncConnector * user)
{
	//Regist user
	if (!user)
	{
		server_log_error("Lobby >> Lobby enter failed by nullptr" << endl);
		return;
	}

	DetachConnectorReturner(*user);
	{
		mutex_lock_guard locker(m_mtxUserList);
		m_userList.insert(user);
	}
	server_log_error("Lobby >> Lobby enter success " << user->ToStr() << endl);


	//send room list for registed user
	{
		arJSON oJSON;
		oJSON["Message"] = "RoomList";

		mutex_lock_guard locker(m_mtxRoomList);
		for (auto iter : m_roomList)
		{
			if (!iter)
				continue;

			arJSON roomJSON;
			{
				roomJSON["ID"] = iter->ID();
				roomJSON["Name"] = iter->Name();
				roomJSON["Locked"] = (iter->IsLocked() ? 1 : 0);
			}
			oJSON["RoomList"].Append(roomJSON);
		}
		__ar_send(*user, oJSON);
	}
	AttachConnectorReturner(*user);
}

void GomokuLobby::DestroyRoom(int id)
{
	mutex_lock_guard locker(m_mtxRoomList);
	GomokuRoom*& room = m_roomList[id];
	if (room)
	{
		delete room;
		room = nullptr;


		//Broadcast
		arJSON oJSON;
		{
			oJSON["Message"] = "RoomDestroyed";
			oJSON["RoomDestroyed"] = id;
		}
		{
			std::lock_guard<std::mutex> locker(m_mtxUserList);
			for (auto iter : m_userList)
				__ar_send(*iter, oJSON);
		}

		server_log_note("Lobby << Destroy Room : Room has destroyed id:" << id << endl);
	}
	else
		server_log_error("Lobby << Destroy Room : Can not found room id:" << id << endl);
}



bool GomokuLobby::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		recvData[recvResult] = NULL;
		arJSON iJSON;
		if (JSON_To_arJSON(recvData.Buffer(), iJSON))
		{
			server_log_error("JSON Errored by " << user.ToStr() << endl);
			return true;
		}
		const std::string& iMessage = iJSON["Message"].Str();


			 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		else if (iMessage == "EnterRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		else if (iMessage == "LeaveLobby")	{ if (LeaveLobby(user, iJSON))	return true; }
		else
			server_log_error("Lobby >> Unknown message recived by " << user.ToStr() <<  " said : " << iMessage << endl);
	}
	else
	{
		DetachConnectorReturner(user);
		RegistedUserRemove(user);
		server_log_note("Lobby >> Client disconnected server : " << user.ToStr() << endl);
		return true;
	}
	return false;
}

bool GomokuLobby::CreateRoom(AsyncConnector & user, const arJSON & iJSON)
{
	int id;
	std::string name, password;
	{
		const arJSON& roomJSON = iJSON["Room"].Sub();
		if (!roomJSON.IsIn("Name"))
		{
			server_log_error("Lobby >> CreateRoom : iJSON Parameter error" << endl);
			return false;
		}

		name = roomJSON["Name"].Str();
		if (roomJSON.IsIn("Password"))
			password = roomJSON["Password"].Str();
	}


	std::lock_guard<std::mutex> locker(m_mtxRoomList);
	id = [this]()->int
	{
		for (int i = 0; i < m_roomList.size(); i++)
		{
			if (!m_roomList[i])
				return i;
		}
		return -1;
	}();
	if (id < 0)
	{
		server_log_error("Lobby >> Create room >> Create ID : Can not found empty slot" << endl);
		return false;
	}

	return CreateRoom(user, id, name, password, true);
}

bool GomokuLobby::CreateRoom(AsyncConnector & user, int id, const std::string & name, const std::string & password, bool noMutex)
{
	std::shared_ptr<std::lock_guard<std::mutex>> mtx(noMutex ? nullptr : new std::lock_guard<std::mutex>(m_mtxRoomList));

	DetachConnectorReturner(user);
	if (!RegistedUserRemove(user))
		return true;

	m_roomList[id] = new GomokuRoom(*this, &user, id, name, password);
	{
		arJSON oJSON;
		oJSON["Message"] = "RoomEntered";
		arJSON roomJSON;
		{
			roomJSON["ID"] = id;
			roomJSON["Name"] = name;
			roomJSON["Locked"] = (password.size() ? 1 : 0);
		}
		oJSON["Room"] = roomJSON;
		__ar_send(user, oJSON);
	}
	server_log_note("Lobby >> CreateRoom : Room that [ " << id << " : " << name << " ] created by " << user.ToStr() << endl);


	//Broadcast
	arJSON oJSON;
	{
		oJSON["Message"] = "RoomCreated";
		arJSON roomJSON;
		{
			roomJSON["ID"] = id;
			roomJSON["Name"] = name;
			roomJSON["Locked"] = (password.size() ? 1 : 0);
		}
		oJSON["Room"] = roomJSON;
	}
	{
		std::lock_guard<std::mutex> locker(m_mtxUserList);
		for (auto iter : m_userList)
			__ar_send(*iter, oJSON);
	}
	return false;
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
			server_log_error("Lobby >> EnterRoom : JSON parameter error" << endl);
			return false;
		}
		id = roomJSON["ID"].Int();
		if (roomJSON.IsIn("Password"))
			password = roomJSON["Password"].Str();
	}

	mutex_lock_guard locker(m_mtxRoomList);
	if (id >= m_roomList.size())
	{
		server_log_error("Lobby >> EnterRoom : Room ID out of range" << endl);
		return false;
	}
	GomokuRoom* room = m_roomList[id];
	if (!room)
	{
		server_log_error("Lobby >> EnterRoom : Can not found room ID:" << id << endl);
		return false;
	}

	DetachConnectorReturner(user);
	if (!room->EnterRoom(&user, password))
	{
		AttachConnectorReturner(user);
		return false;
	}
	
	arJSON oJSON;
	{
		oJSON["Message"] = "RoomEntered";
		arJSON roomJSON;
		{
			roomJSON["ID"] = room->ID();
			roomJSON["Name"] = room->Name();
		}
		oJSON["Room"] = roomJSON;
	}
	__ar_send(user, oJSON);

	if (!RegistedUserRemove(user))
		return true;
	return false;
}

bool GomokuLobby::LeaveLobby(AsyncConnector & user, const arJSON & iJSON)
{
	DetachConnectorReturner(user);
	if (!RegistedUserRemove(user))
		return true;
	
	arJSON oJSON;
	{
		oJSON["Message"] = "LobbyLeaved";
		oJSON["Result"] = 1;
	}
	__ar_send(user, oJSON);

	server_log_note("Lobby >> Lobby leaved : " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl);
	return true;
}



bool GomokuLobby::RegistedUserRemove(AsyncConnector & user)
{
	mutex_lock_guard locker(m_mtxUserList);
	auto iter = m_userList.find(&user);
	if (iter != m_userList.end())
	{
		m_userList.erase(iter);
		return true;
	}
	else
		server_log_error("Lobby >> User remove : Can not found user in user list" << endl);
	return false;
}


