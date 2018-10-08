#pragma once
#include "Scene.h"
class StaticButton;

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
	LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM) override { return NULL; }

private:
	SOCKET ServerConnect(const sockaddr_in& address);

	void AccessOnline();
	void AccessOffline();
	void QuitProgram();

private:
	std::unique_ptr<StaticButton> m_online;
	std::unique_ptr<StaticButton> m_offline;
	std::unique_ptr<StaticButton> m_quit;

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

