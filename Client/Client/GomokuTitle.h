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
	SOCKET ServerConnect(const sockaddr_in& address);


private:
	struct Resource
	{
		LPDIRECT3DTEXTURE9 background;
		LPDIRECT3DTEXTURE9 onlinePlay;
		LPDIRECT3DTEXTURE9 offlinePlay;
		LPDIRECT3DTEXTURE9 quit;

		Resource();
		~Resource();
	} m_resource;
};

