#include "stdafx.h"
#include "GomokuLobby.h"

#include "SceneManager.h"
#include "GomokuTitle.h"

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
}

void GomokuLobby::Update()
{
	if (g_inputDevice.IsKeyDown('1'))
	{
		arJSON oJSON;
		oJSON["Message"] = "LeaveLobby";
		__ar_send(*m_serverConnector, oJSON);
	}

	if (g_inputDevice.IsKeyDown('2'))
		std::terminate();
}

void GomokuLobby::Render()
{
	cout_region_lock;
	cout << "is Lobby" << endl;
}

void GomokuLobby::Release()
{
	DetachConnector();
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
		///	 if (iMessage == "CreateRoom")	{ if (CreateRoom(user, iJSON))	return true; }
		///else if (iMessage == "JoinRoom")	{ if (EnterRoom(user, iJSON))	return true; }
		///else 

		if (iMessage == "LeaveLobby")	{ if (LeaveLobby(iJSON))	return true; }
		else
		{ cout_region_lock; cout << "GomokuLobby >> Note : UnknownMessage" << endl; }
	}
	else
	{
		
		return true;
	}
	return false;
}

bool GomokuLobby::LeaveLobby(const arJSON & iJSON)
{
	if (iJSON.IsIn("Result") && iJSON["Result"].Int())
	{
		DetachConnector();
		SntInst(SceneManager).ChangeScene(new GomokuTitle());
		return true;
	}
	return false;
}
