#include "stdafx.h"
#include "GomokuLobby.h"

#include "SceneManager.h"
#include "GomokuTitle.h"
#include "GomokuRoom.h"

GomokuLobby::GomokuLobby(AsyncConnector* serverConnector)
	: m_serverConnector(serverConnector)
{
}

GomokuLobby::~GomokuLobby()
{
}



void GomokuLobby::Init()
{
	AttachConnectorReturner();
	locked_cout << "Lobby >> Enter" << endl;
}

void GomokuLobby::Update()
{
	if (g_inputDevice.IsKeyDown('1'))
	{
		arJSON oJSON;
		oJSON["Message"] = "LeaveLobby";
		__ar_send(*m_serverConnector, oJSON);

		locked_cout << "Lobby >> Leave Lobby..." << endl;
	}

	if (g_inputDevice.IsKeyDown('2'))
	{
		arJSON oJSON;
		oJSON["Message"] = "CreateRoom";
		arJSON roomJSON;
		{
			roomJSON["Name"] = "Hello Gomoku";
		}
		oJSON["Room"] = roomJSON;
		__ar_send(*m_serverConnector, oJSON);


		locked_cout << "Lobby >> Create Room..." << endl;
	}

	if (g_inputDevice.IsKeyDown('3'))
	{
		mutex_lock_guard lockerR(m_mtxRoomList);
		cout_region_lock;
		cout << "[ Room List ]" << endl;
		for (auto& iter : m_roomList)
			cout << iter.first << " : " << iter.second << endl;
	}


	auto EnterRoomPlease = [this](int id)
	{
		arJSON oJSON;
		oJSON["Message"] = "EnterRoom";
		arJSON roomJSON;
		{
			roomJSON["ID"] = id;
		}
		oJSON["Room"] = roomJSON;
		__ar_send(*m_serverConnector, oJSON);

		locked_cout << "Lobby >> Room Enter..." << endl;
	};
	if (g_inputDevice.IsKeyDown('6'))	EnterRoomPlease(0);
	if (g_inputDevice.IsKeyDown('7'))	EnterRoomPlease(1);
	if (g_inputDevice.IsKeyDown('8'))	EnterRoomPlease(2);


	if (g_inputDevice.IsKeyDown('0'))
		std::terminate();
}

void GomokuLobby::Render()
{
}

void GomokuLobby::Release()
{
	DetachConnectorReturner();
}



bool GomokuLobby::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		recvData[recvResult] = NULL;
		arJSON iJSON;
		if (JSON_To_arJSON(recvData.Buffer(), iJSON))
		{
			locked_cout << "Lobby >> Server returned JSON errored" << endl;
			return true;
		}

		const std::string& iMessage = iJSON["Message"].Str();
		///	 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		///else if (iMessage == "JoinRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		///else 

			 if (iMessage == "RoomCreated")		{ if (RoomCreated(iJSON))	return true; }
		else if (iMessage == "RoomEntered")		{ if (RoomEntered(iJSON))	return true; }
		else if (iMessage == "LobbyLeaved")		{ if (LobbyLeaved(iJSON))	return true; }
		else if (iMessage == "RoomList")		{ if (RoomList(iJSON))		return true; }
		else if (iMessage == "RoomDestroyed")	{ if (RoomDestroyed(iJSON))	return true; }
		else
		{ locked_cout << "Lobby >> UnknownMessage recived : " << iMessage << endl; }
	}
	else
	{
		locked_cout << "Lobby >> Server Disconnected" << endl;
		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuTitle());
		return true;
	}
	return false;
}



bool GomokuLobby::RoomCreated(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;
	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID"))
		return false;

	int id = roomJSON["ID"].Int();
	std::string name;
	if (roomJSON.IsIn("Name"))
		name = roomJSON["Name"].Str();

	mutex_lock_guard lockerR(m_mtxRoomList);
	m_roomList.insert(std::make_pair(id, name));
	locked_cout << "Lobby >> Room list updated : created" << endl;
	return false;
}

bool GomokuLobby::RoomEntered(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Room"))
		return false;

	const arJSON& roomJSON = iJSON["Room"].Sub();
	if (!roomJSON.IsIn("ID") || !roomJSON.IsIn("Name"))
		return false;

	int id = roomJSON["ID"].Int();
	std::string name = roomJSON["Name"].Str();

	DetachConnectorReturner();
	SntInst(SceneManager).ChangeScene(new GomokuRoom(m_serverConnector, id, name));

	cout_region_lock;
	cout << "Lobby >> RoomEntered : " << id << ":" << name << endl;
	return false;
}

bool GomokuLobby::LobbyLeaved(const arJSON & iJSON)
{
	if (iJSON.IsIn("Result") && iJSON["Result"].Int())
	{
		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuTitle());
		locked_cout << "Lobby >> Leaved lobby" << endl;
		return true;
	}
	return false;
}

bool GomokuLobby::RoomList(const arJSON & iJSON)
{
	if (iJSON.IsIn("RoomList"))
	{
		mutex_lock_guard locker2(m_mtxRoomList);
		for (auto& iter : iJSON["RoomList"])
		{
			if (!iter.IsIn("ID"))
				continue;

			int id = iter["ID"].Int();
			std::string name;

			if (iter.IsIn("Name"))
				name = iter["Name"].Str();

			m_roomList.insert(std::make_pair(id, name));
		}
		locked_cout << "Lobby >> Room list updated : resetted" << endl;
	}
	return false;
}

bool GomokuLobby::RoomDestroyed(const arJSON & iJSON)
{
	if (iJSON.IsIn("RoomDestroyed"))
	{
		mutex_lock_guard locker2(m_mtxRoomList);
		auto iter = m_roomList.find(iJSON["RoomDestroyed"].Int());
		if (iter != m_roomList.end())
			m_roomList.erase(iter);

		locked_cout << "Lobby >> Room list updated : destroyed" << endl;
	}
	return false;
}
