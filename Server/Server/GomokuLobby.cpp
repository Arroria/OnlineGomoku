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
	mutex_lock_guard locker(m_mtxUserList);
	m_userList.insert(user);

	AttachConnectorReturner(*user);
	std::thread([&user]() { cout_region_lock; cout << "Lobby >> Enter lobby : " << inet_ntoa(user->Address().sin_addr) << ':' << ntohs(user->Address().sin_port) << endl; }).detach();
}

void GomokuLobby::DestroyRoom(int id)
{
}



bool GomokuLobby::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		recvData[recvResult] = NULL;
		arJSON iJSON;
		if (JSON_To_arJSON(recvData.Buffer(), iJSON))
		{
			cout_region_lock;
			cout << "JSON Errored by " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl;
			return true;
		}

		const std::string& iMessage = iJSON["Message"].Str();
			 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		else if (iMessage == "JoinRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		else if (iMessage == "LeaveLobby")	{ if (LeaveLobby(user, iJSON))	return true; }
	}
	else
	{
		DetachConnectorReturner(user);
		if (!RegistedUserRemove(user))
		{
			cout_region_lock;
			cout << "Lobby >> LeaveLobby : Can not found user in user list" << endl;
		}
		std::thread([&user]() { cout_region_lock; cout << "Lobby >> Client disconnected : " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl; }).detach();
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
			server_srdlog_error("Lobby >> CreateRoom : iJSON Parameter error" << endl);
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
		server_srdlog_error("Lobby >> Create room >> Create ID : Can not found empty slot" << endl);
		return false;
	}

	CreateRoom(user, id, name, password, true);
	


	//Broadcast
	arJSON oJSON;
	{
		oJSON["Message"] = "RoomCreated";
		arJSON roomJSON;
		{
			roomJSON["ID"] = id;
			roomJSON["Name"] = name;
		}
		oJSON["Room"] = roomJSON;
	}
	{
		std::lock_guard<std::mutex> locker(m_mtxUserList);
		for (auto iter : m_userList)
			__ar_send(user, oJSON);
	}
	return false;
}

bool GomokuLobby::CreateRoom(AsyncConnector & user, int id, const std::string & name, const std::string & password, bool noMutex)
{
	std::shared_ptr<std::lock_guard<std::mutex>> mtx(noMutex ? nullptr : new std::lock_guard<std::mutex>(m_mtxRoomList));
	
	m_roomList[id] = new GomokuRoom(*this, &user, id, name, password);
	server_log_note("Lobby >> CreateRoom : Room created for [ " << id << " : " << name << " ] by " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl);
	return true;
}

bool GomokuLobby::EnterRoom(AsyncConnector & user, const arJSON & iJSON)
{
	return false;
}

bool GomokuLobby::LeaveLobby(AsyncConnector & user, const arJSON & iJSON)
{
	DetachConnectorReturner(user);
	if (!RegistedUserRemove(user))
	{
		cout_region_lock;
		cout << "Lobby >> LeaveLobby : Can not found user in user list" << endl;
	}
	
	arJSON oJSON;
	{
		oJSON["Message"] = "LobbyLeaved";
		oJSON["Result"] = 1;
	}
	__ar_send(user, oJSON);

	std::thread([&user]() { cout_region_lock; cout << "Lobby >> Leave lobby : " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl; }).detach();
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
	return false;
}


