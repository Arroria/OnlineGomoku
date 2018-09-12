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
{
	if (host)
		AttachConnectorReturner(*host);
}

GomokuRoom::~GomokuRoom()
{
	if (m_host || m_guest)
		server_log_error("Room >> Disconstructor : Unsafety called" << endl);
}



bool GomokuRoom::EnterRoom(AsyncConnector * guest, const std::string & password)
{
	mutex_lock_guard locker(m_mtxEnterLeave);
	if (!m_host || m_guest || m_password != password)
	{
		server_log_error("Room >> Enter Error" << endl);
		return false;
	}

	m_guest = guest;
	AttachConnectorReturner(*m_guest);
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
			 if (iMessage == "LeaveRoom")	{ if (!LeaveRoom(user))	return true; }
	}
	else
	{
		DetachConnectorReturner(user);
		if (!LeaveRoom(user))
			server_log_error("Lobby >> LeaveLobby : Can not found user in user list" << endl);
		server_log_note("Lobby >> Client disconnected : " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl);
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

		if (&user == m_guest)
		{
			m_lobby.EnterLobby(&user);
			SendLeaved(*m_guest);
			SendRivalLeaved(*m_host);
			
			m_guest = nullptr;
			return true;
		}

		if (&user == m_host)
		{
			m_lobby.EnterLobby(&user);
			SendLeaved(*m_host);
			if (m_guest)
				SendRivalLeaved(*m_guest);
			
			m_host = nullptr;
			destroyRoom = true;
		}
	}
	if (destroyRoom)
	{
		m_lobby.DestroyRoom(m_id);
		return true;
	}
	return false;
}
