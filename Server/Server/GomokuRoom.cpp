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
}

GomokuRoom::~GomokuRoom()
{
	mutex_lock_guard locker0(m_mtxHost);
	mutex_lock_guard locker1(m_mtxGuest);

	if (m_host || m_guest)
		std::thread([]() { cout_region_lock; cout << "Room >> Disconstructor : Unsafety called" << endl; }).detach();
}



bool GomokuRoom::EnterRoom(AsyncConnector * guest, const std::string & password)
{
	mutex_lock_guard locker(m_mtxGuest);
	if (m_guest || m_password != password)
		return false;

	m_guest = guest;
	return true;
}

bool GomokuRoom::MessageProcess(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
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
			 if (iMessage == "LeaveRoom")	{ if (LeaveRoom(user, iJSON))	return true; }
	}
	else
	{
		return true;
	}
	return false;
}

bool GomokuRoom::LeaveRoom(AsyncConnector & user, const arJSON & iJSON)
{
	{
		mutex_lock_guard locker(m_mtxHost);
		if (&user == m_guest)
		{
			m_lobby.EnterLobby(&user);
			m_guest = nullptr;
			return false;
		}
	}

	bool destroyRoom = false;
	{
		mutex_lock_guard locker(m_mtxGuest);
		if (&user == m_host)
		{
			m_lobby.EnterLobby(&user);
			destroyRoom = true;
		}
	}
	if (destroyRoom)
	{
		m_lobby.DestroyRoom(m_id);
		return true;
	}
	
	std::thread([]() { cout_region_lock; cout << "Room >> LeaveRoom : Unsafety called" << endl; }).detach();
	return false;
}
