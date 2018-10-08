#pragma once
#include "Scene.h"

#include "GomokuBoard.h"
class GomokuRoom final :
	public Scene
{
private:
	enum class Message
	{
		UserState,
		RoomEntered,
		IsReady,
		RoomLeaved,

		GomokuStarted,
		GomokuAttacked,
		GomokuEnd,

		Invalid = -1
	};

public:
	GomokuRoom(AsyncConnector* serverConnector, int id, const std::string& name);
	~GomokuRoom();

public:
	void Init() override;
	void Update() override;
	void Render() override;
	void Release() override;
	LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM) override { return NULL; }

private:
	inline void AttachConnectorReturner()
	{
		m_serverConnector->Returner([this](AsyncConnector & user, int recvResult, SocketBuffer & recvData)->bool { return MessageProcessing(user, recvResult, recvData); });
	}
	inline void DetachConnectorReturner() { m_serverConnector->Returner(nullptr); }
	bool MessageProcessing(AsyncConnector&, int, SocketBuffer&);
	Message CheckMessage(const std::string& msg) const;


	//JSON analysis
	bool UserState	(const arJSON& iJSON);
	bool RoomEntered(const arJSON& iJSON);
	bool IsReady	(const arJSON& iJSON);
	bool RoomLeaved	(const arJSON& iJSON);

	bool GomokuStarted	(const arJSON& iJSON);
	bool GomokuAttacked	(const arJSON& iJSON);
	bool GomokuEnd		(const arJSON& iJSON);

	//Action
	bool UserState	(bool isHost);
	bool RoomEntered(bool isHost);
	bool IsReady	(bool isHost, bool isReady);
	bool RoomLeaved	(bool isHost);
	
	bool GomokuStarted	();
	bool GomokuAttacked	(bool isBlack, int x, int y);
	bool GomokuEnd		(bool isBlackWin);

private:
	AsyncConnector * m_serverConnector;
	int m_id;
	std::string m_name;
	bool m_isLocked;

	bool m_isHost;
	bool m_playerJoined;
	bool m_hostReady;
	bool m_guestReady;

	mutable std::mutex m_mtxProcessing;

	GomokuBoard m_gomokuBoard;
	bool m_playerReady[2];

	struct Resource
	{
		LPDIRECT3DTEXTURE9 background;
		LPDIRECT3DTEXTURE9 board;

		LPDIRECT3DTEXTURE9 guest;
		LPDIRECT3DTEXTURE9 host;
		
		LPDIRECT3DTEXTURE9 ready;
		LPDIRECT3DTEXTURE9 readyMark;
		LPDIRECT3DTEXTURE9 quit;

		LPDIRECT3DTEXTURE9 stoneW;
		LPDIRECT3DTEXTURE9 stoneB;
		LPDIRECT3DTEXTURE9 stoneMarker;

		Resource();
		~Resource();
	} m_resource;
};

