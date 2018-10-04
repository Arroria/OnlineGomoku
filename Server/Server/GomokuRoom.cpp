#include "stdafx.h"
#include "GomokuRoom.h"
#include "GomokuLobby.h"

#include "Gomoku.h"

GomokuRoom::GomokuRoom(GomokuLobby& lobby, AsyncConnector * host, int id, const std::string & name, const std::string & password)
	: m_lobby(lobby)

	, m_id(id)
	, m_name(name)
	, m_password(password)
	, m_state(GomokuRoomState::Waiting)

	, m_host(host)
	, m_guest(nullptr)

	, m_gomoku(nullptr)
	, m_hostReady(false)
	, m_guestReady(false)
{
	if (host)
		AttachConnectorReturner(*host);
	else
	{
		server_log_error("Room >> Constructure >> Host user is NULL" << endl);
	}
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



bool GomokuRoom::MessageProcessing(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
{
	if (recvResult > 0)
	{
		arJSON iJSON;
		Message msg = Message::Invalid;
		{
			recvData[recvResult] = NULL;
			if (JSON_To_arJSON(recvData.Buffer(), iJSON))
			{
				server_log_error("JSON Errored by received message from " << user.ToStr() << " in Room" << endl);
				return true;
			}

			if ((msg = CheckMessage(iJSON["Message"].Str())) == Message::Invalid)
			{
				server_log_error("An undefined message(" << static_cast<int>(msg) << ") was received by Room from the " << user.ToStr() << endl);
				return false;
			}
		}
		mutex_lock_guard actionLocker(m_mtxProcessing);

		switch (msg)
		{
		case GomokuRoom::Message::ReadyToInit:		BCUserClientInit(user);	break;
		case GomokuRoom::Message::Ready:			Ready			(user, iJSON);	break;
		case GomokuRoom::Message::LeaveRoom:		LeaveRoom		(user, iJSON);	break;
		case GomokuRoom::Message::GomokuAttack:		GomokuAttack	(user, iJSON);	break;
		default:
			server_log_error("An undefined message(" << static_cast<int>(msg) << ") was received by Room from the " << user.ToStr() << endl);
			break;
		}
	}
	else
	{
		{
		mutex_lock_guard actionLocker(m_mtxProcessing);

		DetachConnectorReturner(user);
		LeaveRoom(user);
		server_log_note("User(" << user.ToStr() << ") disconnected." << endl);
		}

		//임시
		if (!m_host)
			m_lobby.DestroyRoom(m_id);

		return true;
	}

	//임시
	if (!m_host)
		m_lobby.DestroyRoom(m_id);

	return false;
}

GomokuRoom::Message GomokuRoom::CheckMessage(const std::string & msg) const
{
	if (msg.empty())				return Message::Invalid;
	else if (msg == "ReadyToInit")	return Message::ReadyToInit;
	else if (msg == "Ready")		return Message::Ready;
	else if (msg == "LeaveRoom")	return Message::LeaveRoom;
	else if (msg == "Attack")		return Message::GomokuAttack;
	else							return Message::Invalid;
}




bool GomokuRoom::Ready(AsyncConnector & user, const arJSON& iJSON)
{
	if (m_gomoku ||
		!iJSON.IsIn("Ready"))
	{
		server_log_error("ReadyFailed" << endl);
		return false;
	}
	bool isReady = iJSON["Ready"].Int();

	return Ready(user, isReady);
}

bool GomokuRoom::LeaveRoom(AsyncConnector & user, const arJSON & iJSON)
{
	//Nothing
	return LeaveRoom(user);
}

bool GomokuRoom::GomokuAttack(AsyncConnector & user, const arJSON & iJSON)
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

	return GomokuAttack(isBlack, x, y);
}




bool GomokuRoom::EnterRoom(AsyncConnector & guest, const std::string & password)
{
	mutex_lock_guard locker(m_mtxProcessing);
	if (m_guest)
	{
		server_log_error("The room is full." << endl);
		return false;
	}
	if (m_password != password)
	{
		server_log_error("Password incorrect." << endl);
		return false;
	}

	m_guest = &guest;
	AttachConnectorReturner(*m_guest);
	server_log_note("User(" << guest.ToStr() << ") entered the room(" << m_id << ")." << endl);

	m_lobby.BCRoomUpdate(m_id, GomokuRoomState::Ready);
	return true;
}

bool GomokuRoom::Ready(AsyncConnector & user, bool isReady)
{
	bool isHost;
	if		(&user == m_host) isHost = true;
	else if (&user == m_guest) isHost = false;
	else	return false;

	if (isHost)	m_hostReady = isReady;
	else		m_guestReady = isReady;
	BCReady(isHost, isReady);


	if (m_hostReady && m_guestReady)
		return GomokuStart();
	return false;
}

bool GomokuRoom::LeaveRoom(AsyncConnector & user)
{
	bool isHost;
	if		(&user == m_host) isHost = true;
	else if (&user == m_guest) isHost = false;
	else	return false;

	BCLeaveRoom(isHost);
	DetachConnectorReturner(user);

	bool roomDestroy = false;
	if (isHost)
	{
		m_host = nullptr;
		m_lobby.EnterLobby(user);

		if (m_guest)
		{
			m_host = m_guest;
			m_guest = nullptr;
			BCUserState(*m_host, true);

			m_hostReady = m_guestReady;
			m_guestReady = false;
			BCReady(true, m_hostReady);
			BCReady(false, false);
		}
		else
			roomDestroy = true;
	}
	else
	{
		m_guest = nullptr;
		m_lobby.EnterLobby(user);

		m_guestReady = false;
		BCReady(false, false);
	}

	if (m_gomoku)
	{
		delete m_gomoku;
		m_gomoku = nullptr;
	}

	if (roomDestroy)
		;//터진다;; m_lobby.DestroyRoom(m_id);
	else
		m_lobby.BCRoomUpdate(m_id, GomokuRoomState::Waiting);
	return false;
}

bool GomokuRoom::GomokuStart()
{
	m_hostReady = m_guestReady = false;

	m_gomoku = new Gomoku([this](bool blackWin) { this->GomokuEnd(blackWin); });
	server_log_note("Room(" << m_id << ") Gomoku created" << endl);

	BCGomokuStart();
	return false;
}

bool GomokuRoom::GomokuAttack(bool isBlack, int x, int y)
{
	if (m_gomoku->Attack(x, y, isBlack))
		BCGomokuAttack(isBlack, x, y);
	return false;
}

void GomokuRoom::GomokuEnd(bool blackWin)
{
	if (m_gomoku)
	{
		delete m_gomoku;
		m_gomoku = nullptr;
	}

	BCGomokuEnd(blackWin);
}




void GomokuRoom::BCUserClientInit(AsyncConnector & user) const
{
	bool isHost = &user == m_host;
	
	BCUserState(user, isHost);
	if (!isHost)
		BCReady(true, m_hostReady);
}

void GomokuRoom::BCUserState(AsyncConnector & user, bool isHost) const
{
	arJSON oJSON;
	oJSON["Message"] = "UserState";
	oJSON["UserState"] = isHost ? "Host" : "Guest";

	__ar_send(user, oJSON);
}

void GomokuRoom::BCEnterRoom(bool isHost) const
{
	arJSON oJSON;
	oJSON["Message"] = "RoomEntered";
	oJSON["Player"] = isHost ? "Host" : "Guest";

	if (m_host)		__ar_send(*m_host, oJSON);
	if (m_guest)	__ar_send(*m_guest, oJSON);
}

void GomokuRoom::BCReady(bool isHost, bool isReady) const
{
	arJSON oJSON;
	oJSON["Message"] = "IsReady";
	oJSON["IsReady"] = isReady ? 1 : 0;
	oJSON["Player"] = isHost ? "Host" : "Guest";

	if (m_host)		__ar_send(*m_host, oJSON);
	if (m_guest)	__ar_send(*m_guest, oJSON);
}

void GomokuRoom::BCLeaveRoom(bool isHost) const
{
	arJSON oJSON;
	oJSON["Message"] = "RoomLeaved";
	oJSON["Player"] = isHost ? "Host" : "Guest";

	if (m_guest)	__ar_send(*m_guest, oJSON);
	if (m_host)		__ar_send(*m_host, oJSON);
}

void GomokuRoom::BCGomokuStart() const
{
	arJSON oJSON;
	oJSON["Message"] = "GomokuStarted";
	if (m_host)		__ar_send(*m_host, oJSON);
	if (m_guest)	__ar_send(*m_guest, oJSON);

	m_lobby.BCRoomUpdate(m_id, GomokuRoomState::Playing);
}

void GomokuRoom::BCGomokuAttack(bool isBlack, int x, int y) const
{
	arJSON oJSON;
	oJSON["Message"] = "GomokuAttacked";
	arJSON attackJSON;
	{
		attackJSON["Attacker"] = isBlack ? "Black" : "White";
		attackJSON["x"] = x;
		attackJSON["y"] = y;
	}
	oJSON["Attack"] = attackJSON;

	if (m_host)		__ar_send(*m_host, oJSON);
	if (m_guest)	__ar_send(*m_guest, oJSON);
}

void GomokuRoom::BCGomokuEnd(bool blackWin)
{
	arJSON oJSON;
	oJSON["Message"] = "GomokuEnd";
	oJSON["Winner"] = blackWin ? "Black" : "White";

	if (m_host)		__ar_send(*m_host, oJSON);
	if (m_guest)	__ar_send(*m_guest, oJSON);

	m_lobby.BCRoomUpdate(m_id, GomokuRoomState::Playing);
}
