#include "stdafx.h"
#include "GomokuTitle.h"

#include "SceneManager.h"
#include "GomokuLobby.h"


constexpr POINT c_backgroundPos = { 0, 0 };
constexpr POINT c_buttonPos = { 350, 350 };
constexpr POINT c_buttonSize = { 500, 100 };
constexpr size_t c_buttonInterval = 120;

constexpr POINT c_onlinePlayPos		= c_buttonPos;
constexpr POINT c_onlinePlaySize	= c_buttonSize;
constexpr POINT c_offlinePlayPos	= { c_onlinePlayPos.x, c_onlinePlayPos.y + c_buttonInterval };
constexpr POINT c_offlinePlaySize	= c_buttonSize;
constexpr POINT c_exitPos			= { c_offlinePlayPos.x, c_offlinePlayPos.y + c_buttonInterval };
constexpr POINT c_exitSize			= c_buttonSize;



GomokuTitle::GomokuTitle()
{
}

GomokuTitle::~GomokuTitle()
{
}



void GomokuTitle::Init()
{
	auto CreateTex = [](const std::wstring& path, LPDIRECT3DTEXTURE9& target) { D3DXCreateTextureFromFileExW(DEVICE, path.data(), D3DX_DEFAULT_NONPOW2, D3DX_DEFAULT_NONPOW2, NULL, NULL, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, NULL, nullptr, nullptr, &target); };
	
	CreateTex(L"./Resource/title/background.png", m_resource.background);
	CreateTex(L"./Resource/title/online.png", m_resource.onlinePlay);
	CreateTex(L"./Resource/title/offline.png", m_resource.offlinePlay);
	CreateTex(L"./Resource/title/quit.png", m_resource.quit);
}

void GomokuTitle::Update()
{
	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		const POINT mousePos = g_inputDevice.MousePos();
		auto IsMouseIn = [&mousePos](int left, int top, int right, int bottom)->bool
		{
			return
				left <= mousePos.x &&
				top <= mousePos.y &&
				mousePos.x < right &&
				mousePos.y < bottom;
		};

		//Online Play
		if (IsMouseIn(c_onlinePlayPos.x, c_onlinePlayPos.y, c_onlinePlayPos.x + c_onlinePlaySize.x, c_onlinePlayPos.y + c_onlinePlaySize.y))
		{
			sockaddr_in serverAddress;
			///__ar_make_sockaddrin(AF_INET, htonl(INADDR_LOOPBACK), htons(5656), &serverAddress);
			__ar_make_sockaddrin(AF_INET, inet_addr("222.108.204.82"), htons(5656), &serverAddress);
			{ locked_cout << "Server connecting..." << endl; }

			SOCKET mySocket;
			if (mySocket = ServerConnect(serverAddress))
			{
				locked_cout << "Server connect success" << endl;

				AsyncConnector* serverConnector = new AsyncConnector(mySocket, serverAddress);
				serverConnector->Run();
				SntInst(SceneManager).ChangeScene(new GomokuLobby(serverConnector));
				return;
			}
			else
				locked_cout << "Server connect failed" << endl;
		}
		//Offline Play
		else if (IsMouseIn(c_offlinePlayPos.x, c_offlinePlayPos.y, c_offlinePlayPos.x + c_offlinePlaySize.x, c_offlinePlayPos.y + c_offlinePlaySize.y))
		{
			locked_cout << "안만들었어!" << endl;
		}
		//Quit
		else if (IsMouseIn(c_exitPos.x, c_exitPos.y, c_exitPos.x + c_exitSize.x, c_exitPos.y + c_exitSize.y))
		{
			locked_cout << "종료할께!" << endl;
			PostQuitMessage(0);
		}
	}
}

void GomokuTitle::Render()
{
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };
	g_sprite->Begin(D3DXSPRITE_ALPHABLEND);
	
	Draw(m_resource.background, c_backgroundPos.x, c_backgroundPos.y);
	Draw(m_resource.onlinePlay, c_onlinePlayPos.x, c_onlinePlayPos.y);
	Draw(m_resource.offlinePlay, c_offlinePlayPos.x, c_offlinePlayPos.y);
	Draw(m_resource.quit, c_exitPos.x, c_exitPos.y);

	g_sprite->End();
}

void GomokuTitle::Release()
{
}



SOCKET GomokuTitle::ServerConnect(const sockaddr_in& address)
{
	SOCKET mySocket = NULL;
	SocketError socketError;
	
	if (__ar_socket(&socketError, AF_INET, SOCK_STREAM, NULL, &mySocket))
	{
		cout_region_lock;
		cout << "__ar_socket >> Error : " << socketError.errorName << endl;
		return NULL;
	}
	
	if (__ar_connect(&socketError, mySocket, address))
	{
		cout_region_lock;
		cout << "__ar_connect >> Error : "	<< socketError.errorName << endl;
		closesocket(mySocket);
		return NULL;
	}

	return mySocket;
}





GomokuTitle::Resource::Resource()
	: background(nullptr)
	, onlinePlay(nullptr)
	, offlinePlay(nullptr)
	, quit(nullptr)
{
}

GomokuTitle::Resource::~Resource()
{
	if (background)		background->Release();
	if (onlinePlay)		onlinePlay->Release();
	if (offlinePlay)	offlinePlay->Release();
	if (quit)			quit->Release();
}
