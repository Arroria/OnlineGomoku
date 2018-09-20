#include "stdafx.h"
#include "GomokuRoom.h"
#include "GomokuLobby.h"


GomokuRoom::GomokuRoom(GomokuLobby& lobby, AsyncConnector * host, int id, const std::string & name, const std::string & password)
	: m_id(id)
	, m_name(name)
	, m_password(password)

	, m_lobby(lobby)
	, m_host(host)
	, m_guest(nullptr)

	, m_gomoku(nullptr)
	, m_ready{ false }
{
	if (host)
		AttachConnectorReturner(*host);
	else
		server_log_error("Room >> Constructure >> Host user is NULL" << endl);
}

GomokuRoom::~GomokuRoom()
{
	if (m_host || m_guest)
		server_log_error("Room >> Disconstructor >> Has a user" << endl);
	if (m_gomoku)
	{
		delete m_gomoku;
		m_gomoku = nullptr;
	}
}



bool GomokuRoom::EnterRoom(AsyncConnector * guest, const std::string & password)
{
	mutex_lock_guard locker(m_mtxEnterLeave);
	if (!m_host || m_guest)
	{
		server_log_error("Room >> Guest enter failed by unknown error" << endl);
		return false;
	}
	if (m_password != password)
	{
		server_log_error("Room >> Guest enter failed by inconsistency password" << endl);
		return false;
	}

	m_guest = guest;
	AttachConnectorReturner(*m_guest);
	server_log_note("Room >> Guest enter success" << endl);
	return true;
}

bool GomokuRoom::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		recvData[recvResult] = NULL;
		arJSON iJSON;
		if (JSON_To_arJSON(recvData.Buffer(), iJSON))
		{
			server_log_error("JSON Errored by " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl);
			return true;
		}
		const std::string& iMessage = iJSON["Message"].Str();


			 if (iMessage == "LeaveRoom")	{ if (LeaveRoom(user))		return true; }
		else if (iMessage == "Ready")		{ if (Ready(user, iJSON))	return true; }
		else if (iMessage == "Attack")		{ if (Attack(user, iJSON))	return true; }
		else
			server_log_error("Lobby >> Unknown message recived" << endl);
	}
	else
	{
		DetachConnectorReturner(user);
		if (!LeaveRoom(user))
			server_log_error("Room >> LeaveLobby : Can not found user in user list" << endl);
		server_log_note("Room >> Client disconnected : " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl);
		return true;
	}
	return false;
}

bool GomokuRoom::LeaveRoom(AsyncConnector & user)
{
	bool destroyRoom = false;
	{
		mutex_lock_guard locker(m_mtxEnterLeave);
		DetachConnectorReturner(user);

		auto SendLeaved = [](AsyncConnector & leavedUser)
		{
			arJSON oJSON;
			{
				oJSON["Message"] = "RoomLeaved";
				oJSON["Result"] = 1;
			}
			__ar_send(leavedUser, oJSON);
		};
		auto SendRivalLeaved = [](AsyncConnector & stayUser)
		{
			arJSON oJSON;
			oJSON["Message"] = "UserDisconnected";
			
			__ar_send(stayUser, oJSON);
		};
		auto SendReady = [this](bool isBlack, bool isReady)
		{
			arJSON oJSON;
			oJSON["Message"] = "Ready";
			oJSON["Ready"] = isReady ? 1 : 0;
			oJSON["By"] = isBlack ? "Black" : "White";

			if (m_host)		__ar_send(*m_host, oJSON);
			if (m_guest)	__ar_send(*m_guest, oJSON);
		};

		if (&user == m_guest)
		{
			SendLeaved(*m_guest);
			SendRivalLeaved(*m_host);
			m_guest = nullptr;
			m_ready[1] = false;
			SendReady(false, false);
			
			m_lobby.EnterLobby(&user);
			server_log_note("Room >> Guest leaved the room" << endl);
		}
		else if (&user == m_host)
		{
			SendLeaved(*m_host);
			m_host = nullptr;

			if (m_guest)
			{
				SendRivalLeaved(*m_guest);
				m_host = m_guest;
				m_guest = nullptr;

				m_ready[0] = m_ready[1];
				m_ready[1] = false;
				SendReady(false, false);
				SendReady(true, m_ready[0]);

				m_lobby.EnterLobby(&user);
				server_log_note("Room >> Host leaved the room, Guest get host" << endl);
			}
			else
			{
				m_lobby.EnterLobby(&user);
				destroyRoom = true;
			}
		}
		else
			server_log_error("Room >> Leave user unknown" << endl);
		GomokuDestroy();
	}

	//밖으로 안빼면 스레드 터짐
	if (destroyRoom)
	{
		m_lobby.DestroyRoom(m_id);
		server_log_note("Room >> Host leaved the room, Room destroyed" << endl);
	}
	return false;
}

bool GomokuRoom::Ready(AsyncConnector & user, const arJSON& iJSON)
{
	if (!m_gomoku &&
		iJSON.IsIn("Ready"))
	{
		auto SendReady = [this](bool isBlack, bool isReady)
		{
			arJSON oJSON;
			oJSON["Message"] = "Ready";
			oJSON["Ready"] = isReady ? 1 : 0;
			oJSON["By"] = isBlack ? "Black" : "White";

			if (m_host)		__ar_send(*m_host, oJSON);
			if (m_guest)	__ar_send(*m_guest, oJSON);
		};

		bool ready = iJSON["Ready"].Int();
		if (&user == m_host)
		{
			server_log_note("Room >> Host Ready " << (ready ? "ON" : "OFF") << endl);
			m_ready[0] = ready;
			SendReady(true, ready);
		}
		else if (&user == m_guest)
		{
			server_log_note("Room >> Guest Ready " << (ready ? "ON" : "OFF") << endl);
			m_ready[1] = ready;
			SendReady(false, ready);
		}

		if (m_ready[0] && m_ready[1])
		{
			std::lock_guard<std::mutex> locker(m_mtxGomoku);
			m_gomoku = new Gomoku([this](bool blackWin) { this->GomokuMessageProcessing(blackWin); });
			server_log_note("Room >> Game created" << endl);


			arJSON oJSON;
			oJSON["Message"] = "GomokuStart";
			if (m_host)		__ar_send(*m_host, oJSON);
			if (m_guest)	__ar_send(*m_guest, oJSON);
		}
	}
	return false;
}

bool GomokuRoom::Attack(AsyncConnector & user, const arJSON & iJSON)
{
	if (!m_gomoku)
		return false;

	if (!iJSON.IsIn("Attack"))
		return false;
	
	const arJSON& attackJSON = iJSON["Attack"].Sub();
	if (!attackJSON.IsIn("x") || !attackJSON.IsIn("y"))
		return false;



	int x = attackJSON["x"].Int();
	int y = attackJSON["y"].Int();
	bool isBlack = &user == m_host;

	std::lock_guard<std::mutex> locker(m_mtxGomoku);
	if (m_gomoku->Attack(x, y, isBlack))
	{
		arJSON oJSON;
		oJSON["Message"] = "Attacked";
		arJSON attackJSON;
		{
			attackJSON["Attacker"] = isBlack ? "Black" : "White";
			attackJSON["x"] = x;
			attackJSON["y"] = y;
		}
		oJSON["Attack"] = attackJSON;

		if (m_host)		__ar_send(*m_host, oJSON);
		if (m_guest)	__ar_send(*m_guest, oJSON);
		server_log_note("Room >> " << (isBlack ? "Black" : "White") << " attacked" << endl);
	}
	return false;
}



void GomokuRoom::GomokuMessageProcessing(bool blackWin)
{
	arJSON oJSON;
	oJSON["Message"] = "GomokuEnd";
	oJSON["Winner"] = blackWin ? "Black" : "White";
	
	if (m_host)		__ar_send(*m_host, oJSON);
	if (m_guest)	__ar_send(*m_guest, oJSON);
	GomokuDestroy();
}



void GomokuRoom::GomokuDestroy()
{
	std::lock_guard<std::mutex> locker(m_mtxGomoku);
	if (m_gomoku)
	{
		delete m_gomoku;
		m_gomoku = nullptr;
	}
}
