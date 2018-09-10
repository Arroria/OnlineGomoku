#pragma once

#include "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include/d3dx9.h"
#pragma comment (lib, "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/d3d9.lib")
#pragma comment (lib, "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Lib/x86/d3dx9.lib")

#include <chrono>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if (p) { delete (p); (p) = nullptr; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = nullptr; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p); (p) = nullptr; } }
#endif

struct WndInfo
{
	HWND hWnd;
	DWORD wndStyle;
	std::wstring name;

	//int xPos;
	//int yPos;
	unsigned int xSize;
	unsigned int ySize;

	WndInfo();
};

class ProcessManager
{
	using FrameTimer = std::chrono::steady_clock;



private:
	HINSTANCE m_hInstance;
	WndInfo* m_wndInfo;

public:
	HRESULT CreateWnd(const std::wstring& wndName, const DWORD& wndStyle, LRESULT(CALLBACK *wndProc)(HWND, UINT, WPARAM, LPARAM));
	void WndMove(const int& xPos, const int& yPos);
	void WndResize(const unsigned int& xSize, const unsigned int& ySize);

	WndInfo* GetWndInfo() { return m_wndInfo; }



private:
	LPDIRECT3DDEVICE9 m_d3dxDevice;

public:
	HRESULT CreateDevice(const bool& isWindow);

	LPDIRECT3DDEVICE9 GetDevice() { return m_d3dxDevice; }


private:
	bool(*m_initialize)();
	void(*m_update)();
	bool(*m_render)();
	bool(*m_release)();

	size_t m_updateFPSLimite;
	size_t m_renderFPSLimite;
	FrameTimer::duration m_updateDeltaTime;
	
public:
	WPARAM Loop();

	void SetFunction(bool(*initialize)(), void(*update)(), bool(*render)(), bool(*release)()) { m_initialize = initialize; m_update = update; m_render = render; m_release = release; }
	void SetFPSLimite(size_t updateFPS, size_t renderFPS) { m_updateFPSLimite = updateFPS; m_renderFPSLimite = renderFPS; }
	FrameTimer::duration GetDeltaTime() { return m_updateDeltaTime; }


public:
	ProcessManager(const HINSTANCE& hInstance);
	~ProcessManager();
};


