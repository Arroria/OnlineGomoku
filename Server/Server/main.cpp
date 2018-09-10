#include "stdafx.h"

#include "GomokuLobby.h"

int main()
{
	g_server = new GomokuLobby();

	SocketError socketError;
	WSADATA wsaData;
	if (__ar_WSAStartup(&socketError, 0x0202, &wsaData))					{ cout << "__ar_WSAStartup >> Error : "	<< socketError.errorName << endl;	system("pause");	exit(1); }

	SOCKET mySocket;
	sockaddr_in myAddress;
	if (__ar_socket(&socketError, AF_INET, SOCK_STREAM, NULL, &mySocket))	{ cout << "__ar_socket >> Error : "		<< socketError.errorName << endl;	system("pause");	exit(1); }
	__ar_make_sockaddrin(AF_INET, htonl(INADDR_ANY), htons(5656), &myAddress);
	if (__ar_bind(&socketError, mySocket, (sockaddr*)&myAddress))			{ cout << "__ar_bind >> Error : "		<< socketError.errorName << endl;	system("pause");	exit(1); }
	if (__ar_listen(&socketError, mySocket, 64))							{ cout << "__ar_listen >> Error : "		<< socketError.errorName << endl;	system("pause");	exit(1); }


	while (true)
	{
		SOCKET clientSocket;
		sockaddr_in clientAddress;
		__ar_accept(&socketError, mySocket, clientSocket, clientAddress);

		{
			cout_region_lock;
			cout << "Server connected >> Socket : " << clientSocket << " >> Address : " << inet_ntoa(clientAddress.sin_addr) << ':' << ntohs(clientAddress.sin_port) << endl;
		}

		AsyncConnector* client = new AsyncConnector(clientSocket, clientAddress);
		client->Run();
		{
			cout_region_lock;
			cout << "AsyncConnector Running >> " << clientSocket << ' ' << inet_ntoa(clientAddress.sin_addr) << ':' << ntohs(clientAddress.sin_port) << endl;
		}

		g_server->EnterLobby(client);
	}


	closesocket(mySocket);
	WSACleanup();
	return 0;
}
