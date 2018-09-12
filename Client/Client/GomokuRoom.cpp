#include "stdafx.h"
#include "GomokuRoom.h"

#include "SceneManager.h"
#include "GomokuLobby.h"

GomokuRoom::GomokuRoom(AsyncConnector* serverConnector, int id, const std::string& name)
	: m_serverConnector(serverConnector)
	, m_id(id)
	, m_name(name)
{
}

GomokuRoom::~GomokuRoom()
{
}



void GomokuRoom::Init()
{
	AttachConnectorReturner();
}

void GomokuRoom::Update()
{
	if (g_inputDevice.IsKeyDown('1'))
	{
		arJSON oJSON;
		oJSON["Message"] = "LeaveRoom";
		__ar_send(*m_serverConnector, oJSON);
	}

	if (g_inputDevice.IsKeyDown('0'))
		std::terminate();
}

void GomokuRoom::Render()
{
}

void GomokuRoom::Release()
{
	DetachConnectorReturner();
}



bool GomokuRoom::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
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
		///	 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		///else if (iMessage == "JoinRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		///else 

			 if (iMessage == "RoomLeaved")	{ if (RoomLeaved(iJSON))	return true; }
		else
		{ cout_region_lock; cout << "GomokuLobby >> Note : UnknownMessage" << endl; }
	}
	else
	{
		
		return true;
	}
	return false;
}

bool GomokuRoom::RoomLeaved(const arJSON & iJSON)
{
	if (iJSON.IsIn("Result") && iJSON["Result"].Int())
	{
		cout_region_lock;
		cout << "Room Leaved " << endl;

		DetachConnectorReturner();
		SntInst(SceneManager).ChangeScene(new GomokuLobby(m_serverConnector));
	}
	return false;
}
