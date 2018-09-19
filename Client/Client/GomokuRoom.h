#pragma once
#include "Scene.h"

#include "GomokuBoard.h"
class GomokuRoom final :
	public Scene
{
public:
	GomokuRoom(AsyncConnector* serverConnector, int id, const std::string& name);
	~GomokuRoom();

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

	bool RoomLeaved(const arJSON& iJSON);

	bool Ready(const arJSON& iJSON);
	bool GomokuStart(const arJSON& iJSON);
	bool Attacked(const arJSON& iJSON);
	bool GomokuEnd(const arJSON& iJSON);

private:
	AsyncConnector * m_serverConnector;
	int m_id;
	std::string m_name;

	GomokuBoard m_gomokuBoard;
	bool m_playerReady[2];
};

