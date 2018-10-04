#include "stdafx.h"
#include "MainLoop.h"

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	g_processManager = new ProcessManager(hInstance);
	int lastWParam = -1;

	if (SUCCEEDED(g_processManager->CreateWnd(L"Gomoku", NULL, MainLoop::MsgProc)))
	//if (SUCCEEDED(g_processManager->CreateWnd(L"Gomoku", WS_POPUP, MainLoop::MsgProc)))
	{
		g_processManager->WndResize(1200, 800);
		g_processManager->WndMove((1920 - 1200) / 2, (1080 - 800) / 2);
		g_processManager->SetFPSLimite(30, 30);
		g_processManager->SetFunction(MainLoop::Initialize, MainLoop::Update, MainLoop::Render, MainLoop::Release);
		if (SUCCEEDED(g_processManager->CreateDevice(true)))
		{
			SocketError socketError;
			WSADATA wsaData;
			if (__ar_WSAStartup(&socketError, 0x0202, &wsaData))
			{
				cout << "__ar_WSAStartup >> Error : " << socketError.errorName << endl;
				system("pause");
				exit(1);
			}



			lastWParam = g_processManager->Loop();
			if (DEVICE)
				DEVICE->Release();
		}
	}

	return lastWParam;
}
