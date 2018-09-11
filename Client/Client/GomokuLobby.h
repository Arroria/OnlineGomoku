#pragma once
#include "Scene.h"
class GomokuLobby final :
	public Scene
{
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

	bool RoomCreated(const arJSON& iJSON);
	bool LobbyLeaved(const arJSON& iJSON);

private:
	AsyncConnector * m_serverConnector;
};

