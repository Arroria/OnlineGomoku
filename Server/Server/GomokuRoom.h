#pragma once
#include <string>


class GomokuLobby;
enum class GomokuRoomState : int
{
	Waiting = 0,
	Ready = 1,
	Playing = 2,
};

class Gomoku;
class GomokuRoom
{
private:
	//using State = GomokuRoomState;
	enum class Message
	{
		ReadyToInit,

		Ready,
		LeaveRoom,

		GomokuAttack,

		Invalid = -1
	};

public:
	GomokuRoom(GomokuLobby& lobby, AsyncConnector* host, int id, const std::string& name, const std::string& password);
	~GomokuRoom();

	inline int ID()					const { return m_id; }
	inline std::string Name()		const { return m_name; }
	inline bool IsLocked()			const { return m_password.size(); }
	inline GomokuRoomState State()	const { return m_state; }

	//Action
	bool EnterRoom(AsyncConnector& guest, const std::string& password);

private:
	inline void AttachConnectorReturner(AsyncConnector& connector)
	{
		connector.Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline static void DetachConnectorReturner(AsyncConnector& connector) { connector.Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);
	Message CheckMessage(const std::string& msg) const;

	//JSON analysis
	bool Ready(AsyncConnector& user, const arJSON& iJSON);
	bool LeaveRoom(AsyncConnector& user, const arJSON& iJSON);
	bool GomokuAttack(AsyncConnector& user, const arJSON& iJSON);

	//Action
	bool Ready(AsyncConnector& user, bool isReady);
	bool LeaveRoom(AsyncConnector& user);

	bool GomokuStart();
	bool GomokuAttack(bool isBlack, int x, int y);
	void CALLBACK GomokuEnd(bool blackWin);
	
	//Broadcast
	void BCUserClientInit(AsyncConnector& user) const;

	void BCUserState(AsyncConnector& user, bool isHost) const;
	void BCEnterRoom(bool isHost) const;
	void BCReady(bool isHost, bool isReady) const;
	void BCLeaveRoom(bool isHost) const;

	void BCGomokuStart() const;
	void BCGomokuAttack(bool isBlack, int x, int y) const;
	void BCGomokuEnd(bool blackWin);

private:
	GomokuLobby& m_lobby;

	const int m_id;
	const std::string m_name;
	const std::string m_password;
	GomokuRoomState m_state;

	AsyncConnector* m_host;
	AsyncConnector* m_guest;
	Gomoku* m_gomoku;
	bool m_hostReady;
	bool m_guestReady;

	mutable std::mutex m_mtxProcessing;
};

