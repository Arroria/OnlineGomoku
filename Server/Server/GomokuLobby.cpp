#include "stdafx.h"
#include "GomokuLobby.h"
#include "GomokuRoom.h"


GomokuLobby::GomokuLobby()
{
}
GomokuLobby::~GomokuLobby()
{
}



void GomokuLobby::PlayerConnect(AsyncConnector * user)
{
}

void GomokuLobby::EnterLobby(AsyncConnector * user)
{
	user->Returner([this](AsyncConnector& user, int recvResult, SocketBuffer& recvData)->bool { return GomokuLobby::MessageProcess(user, recvResult, recvData); });
	mutex_lock_guard locker(m_mtxUserList);
	m_userList.insert(user);

	std::thread([&user]() { cout_region_lock; cout << "Lobby >> Enter lobby : " << inet_ntoa(user->Address().sin_addr) << ':' << ntohs(user->Address().sin_port) << endl; }).detach();
}

void GomokuLobby::DestroyRoom(int id)
{
}



bool GomokuLobby::MessageProcess(AsyncConnector & user, int recvResult, SocketBuffer & recvData)
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
		std::thread([&user]() { cout_region_lock; cout << "Lobby >> Client disconnected : " << inet_ntoa(user.Address().sin_addr) << ':' << ntohs(user.Address().sin_port) << endl; }).detach();

		user.Returner(nullptr);
		if (!RegistedUserRemove(user))
		{
			cout_region_lock;
			cout << "Lobby >> LeaveLobby : Can not found user in user list" << endl;
		}
		return true;
	}
	return false;
}

bool GomokuLobby::CreateRoom(AsyncConnector & user, const arJSON & iJSON)
{
	return false;
}

bool GomokuLobby::EnterRoom(AsyncConnector & user, const arJSON & iJSON)
{
	return false;
}

bool GomokuLobby::LeaveLobby(AsyncConnector & user, const arJSON & iJSON)
{
	user.Returner(nullptr);
	if (!RegistedUserRemove(user))
	{
		cout_region_lock;
		cout << "Lobby >> LeaveLobby : Can not found user in user list" << endl;
	}
	
	arJSON oJSON;
	{
		oJSON["Message"] = "LeaveLobby";
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
		user.Returner(nullptr);
		m_userList.erase(iter);
		return true;
	}
	return false;
}
