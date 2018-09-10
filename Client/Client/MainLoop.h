#pragma once
class MainLoop
{
public:
	static bool Initialize();
	static void Update();
	static bool Render();
	static bool Release();
	static LRESULT CALLBACK MsgProc(HWND, UINT, WPARAM, LPARAM);
};

