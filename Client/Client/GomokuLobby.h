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
	bool ProtocolProcessing(AsyncConnector&, int, SocketBuffer&);

	bool LeaveLobby(const arJSON& iJSON);

private:
	AsyncConnector * m_serverConnector;
};

