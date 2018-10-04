#pragma once
#include "Scene.h"


class GomokuRoomData
{
public:
	enum class State : int
	{
		Waiting = 0,
		Ready = 1,
		Playing = 2,
	};

public:
	GomokuRoomData() : id(-1), name(), isLocked(false), state(State::Waiting) {}

	int id;
	std::string name;
	bool isLocked;
	State state;
};

#include <map>
class GomokuLobby final :
	public Scene
{
private:
	enum class Message
	{
		RoomListRefresh,
		RoomCreated,
		RoomDestroyed,
		RoomEntered,
		RoomUpdate,
		LobbyLeaved,

		Invalid = -1
	};

public:
	GomokuLobby(AsyncConnector* serverConnector);
	~GomokuLobby();

public:
	void Init() override;
	void Update() override;
	void Render() override;
	void Release() override;

private:
	inline void AttachConnectorReturner()
	{
		m_serverConnector->Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline void DetachConnectorReturner() { m_serverConnector->Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);
	Message CheckMessage(const std::string& msg) const;

	//JSON analysis
	bool RoomListRefresh	(const arJSON& iJSON);
	bool RoomCreated		(const arJSON& iJSON);
	bool RoomUpdate			(const arJSON& iJSON);
	bool RoomEntered		(const arJSON& iJSON);
	bool RoomDestroyed		(const arJSON& iJSON);
	bool LobbyLeaved		(const arJSON& iJSON);

	//Action
	bool RoomCreated		(int id, const std::string& name, bool isLocked, GomokuRoomData::State state);
	bool RoomUpdate			(int id, GomokuRoomData::State state);
	bool RoomEntered		(int id, const std::string& name, bool isLocked);
	bool RoomDestroyed		(int id);
	bool LobbyLeaved		();

private:
	AsyncConnector * m_serverConnector;

	std::map<int, GomokuRoomData> m_roomList;
	mutable std::mutex m_mtxProcessing;


	struct Resource
	{
		LPDIRECT3DTEXTURE9 background;
		
		LPD3DXFONT font;
		LPDIRECT3DTEXTURE9 listBar;
		LPDIRECT3DTEXTURE9 create;
		LPDIRECT3DTEXTURE9 exit;

		Resource();
		~Resource();
	} m_resource;
};

