#pragma once
#include "Scene.h"
class GomokuTitle final :
	public Scene
{
public:
	GomokuTitle();
	~GomokuTitle();

public:
	void Init() override;
	void Update() override;
	void Render() override;
	void Release() override;

private:
	bool ServerSystemBuild();
	SOCKET ServerConnect(const sockaddr_in& address);
};

