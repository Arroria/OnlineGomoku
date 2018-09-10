#include "stdafx.h"
#include "GomokuTitle.h"

#include "SceneManager.h"
#include "GomokuLobby.h"

GomokuTitle::GomokuTitle()
{
}

GomokuTitle::~GomokuTitle()
{
}



void GomokuTitle::Init()
{
	ServerSystemBuild();
}

void GomokuTitle::Update()
{
	if (g_inputDevice.IsKeyDown('1'))
	{
		sockaddr_in serverAddress;
		__ar_make_sockaddrin(AF_INET, htonl(INADDR_LOOPBACK), htons(5656), &serverAddress);
		//__ar_make_sockaddrin(AF_INET, inet_addr("222.110.147.51"), htons(5656), &serverAddress);

		SOCKET mySocket;
		if (mySocket = ServerConnect(serverAddress))
		{
			AsyncConnector* serverConnector = new AsyncConnector(mySocket, serverAddress);
			serverConnector->Run();
			SntInst(SceneManager).ChangeScene(new GomokuLobby(serverConnector));
		}
	}

	if (g_inputDevice.IsKeyDown('2'))
		std::terminate();
}

void GomokuTitle::Render()
{
	cout << "is Title" << endl;
}

void GomokuTitle::Release()
{
}



bool GomokuTitle::ServerSystemBuild()
{
	SocketError socketError;
	WSADATA wsaData;	
	if (__ar_WSAStartup(&socketError, 0x0202, &wsaData))
	{
		cout << "__ar_WSAStartup >> Error : "	<< socketError.errorName << endl;	system("pause");
		exit(1);
	}

	//성공한다고 치자
	return true;
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