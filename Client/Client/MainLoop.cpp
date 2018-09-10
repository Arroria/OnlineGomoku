#include "stdafx.h"
#include "MainLoop.h"
#define CREATE_CONSOLE

#include "SceneManager.h"
#include "GomokuTitle.h"

bool MainLoop::Initialize()
{
#ifdef CREATE_CONSOLE
	if (AllocConsole())
	{
		freopen("CONIN$", "rb", stdin);
		freopen("CONOUT$", "wb", stdout);
		freopen("CONOUT$", "wb", stderr);
	}
#endif

				

	cout.imbue(std::locale("kor"));

	DEVICE->SetRenderState(D3DRS_LIGHTING, false);
	DEVICE->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	DEVICE->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	DEVICE->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	DEVICE->SetRenderState(D3DRS_ZENABLE, false);
	DEVICE->SetRenderState(D3DRS_ZWRITEENABLE, false);
	
	D3DXCreateSprite(DEVICE, &g_sprite);



	SntInst(SceneManager).ChangeScene(new GomokuTitle());


	return true;
}

void MainLoop::Update()
{
	g_inputDevice.BeginFrame(g_processManager->GetWndInfo()->hWnd);
	SntInst(SceneManager).Update();
	g_inputDevice.EndFrame();
}

bool MainLoop::Render()
{
	SntInst(SceneManager).Render();
	return true;
}

bool MainLoop::Release()
{
	SntRelease(SceneManager);


#ifdef CREATE_CONSOLE
	FreeConsole();
#endif
	return true;
}


LRESULT MainLoop::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	g_inputDevice.MsgProc(msg, wParam, lParam);
	
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}

	return 0;
}
