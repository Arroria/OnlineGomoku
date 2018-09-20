#include "stdafx.h"
#include "GomokuRoom.h"

#include "SceneManager.h"
#include "GomokuLobby.h"

GomokuRoom::GomokuRoom(AsyncConnector* serverConnector, int id, const std::string& name)
	: m_serverConnector(serverConnector)
	, m_id(id)
	, m_name(name)

	, m_playerReady{ false }
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

	if (g_inputDevice.IsKeyDown('2'))
	{
		arJSON oJSON;
		oJSON["Message"] = "Ready";
		oJSON["Ready"] = 1;
		__ar_send(*m_serverConnector, oJSON);
	}
	if (g_inputDevice.IsKeyDown('3'))
	{
		arJSON oJSON;
		oJSON["Message"] = "Ready";
		oJSON["Ready"] = 0;
		__ar_send(*m_serverConnector, oJSON);
	}


	auto GomokuAttack = [this](int x, int y)
	{
		arJSON oJSON;
		oJSON["Message"] = "Attack";
		arJSON attackJSON;
		{
			attackJSON["x"] = x;
			attackJSON["y"] = y;
		}
		oJSON["Attack"] = attackJSON;
		__ar_send(*m_serverConnector, oJSON);
	};
	if (g_inputDevice.IsKeyDown('4'))	GomokuAttack(7, 7);
	if (g_inputDevice.IsKeyDown('5'))	GomokuAttack(6, 7);
	if (g_inputDevice.IsKeyDown('6'))	GomokuAttack(8, 4);
	if (g_inputDevice.IsKeyDown('7'))	GomokuAttack(0, 0);
	if (g_inputDevice.IsKeyDown('8'))	GomokuAttack(14, 14);

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

		else if (iMessage == "Ready")		{ if (Ready(iJSON))			return true; }
		else if (iMessage == "GomokuStart")	{ if (GomokuStart(iJSON))	return true; }
		else if (iMessage == "Attacked")	{ if (Attacked(iJSON))		return true; }
		else if (iMessage == "GomokuEnd")	{ if (GomokuEnd(iJSON))		return true; }
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

bool GomokuRoom::Ready(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Ready") || !iJSON.IsIn("By"))
		return false;

	bool ready = iJSON["Ready"].Int();
	bool isBlack = iJSON["By"].Str() == "Black";

	m_playerReady[isBlack ? 0 : 1] = ready;
	locked_cout << (isBlack ? "Black" : "White") << " Player ready " << (ready ? "On" : "Off") << endl;
	return false;
}

bool GomokuRoom::GomokuStart(const arJSON & iJSON)
{
	locked_cout << "GameStarted\n";
	return false;
}


bool GomokuRoom::Attacked(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Attack"))
		return false;
	const arJSON& attackJSON = iJSON["Attack"].Sub();
	if (!attackJSON.IsIn("Attacker") ||
		!attackJSON.IsIn("x") ||
		!attackJSON.IsIn("y"))
		return false;

	bool isBlack = iJSON["Attacker"].Str() == "Black";
	int x = attackJSON["x"].Int();
	int y = attackJSON["y"].Int();

	m_gomokuBoard.At(x, y) = isBlack ? GomokuBoard::blackValue : GomokuBoard::whiteValue;
	{
		locked_cout << "Gomoku Board State\n";
		for (int y = 0; y < GomokuBoard::boardSizeY; y++)
		{
			for (int x = 0; x < GomokuBoard::boardSizeX; x++)
			{
				if		(GomokuBoard::IsBlack(x))	cout << "¡Ü";
				else if (GomokuBoard::IsWhite(y))	cout << "¡Û";
				else								cout << "¦«";
			}
			cout << endl;
		}
	}
	return false;
}

bool GomokuRoom::GomokuEnd(const arJSON & iJSON)
{
	if (!iJSON.IsIn("Winner"))
		return false;

	const std::string& winner = iJSON["Winner"].Str();
	locked_cout << "Winner Is " << winner << endl;
	return false;
}
