#include "stdafx.h"

int main()
{
	SocketError socketError;
	WSADATA wsaData;
	if (__ar_WSAStartup(&socketError, 0x0202, &wsaData))					{ cout << "__ar_WSAStartup >> Error : "	<< socketError.errorName << endl;	system("pause");	exit(1); }

	SOCKET mySocket;
	if (__ar_socket(&socketError, AF_INET, SOCK_STREAM, NULL, &mySocket))	{ cout << "__ar_socket >> Error : "		<< socketError.errorName << endl;	system("pause");	exit(1); }
	
	sockaddr_in serverAddress;
	__ar_make_sockaddrin(AF_INET, htonl(INADDR_LOOPBACK), htons(5656), &serverAddress);
	//__ar_make_sockaddrin(AF_INET, inet_addr("222.110.147.51"), htons(5656), &serverAddress);
	if (__ar_connect(&socketError, mySocket, serverAddress))				{ cout << "__ar_connect >> Error : "	<< socketError.errorName << endl;	system("pause");	exit(1); }


	{
		cout << "Connect >> " << mySocket << ">>" << inet_ntoa(serverAddress.sin_addr) << ':' << ntohs(serverAddress.sin_port) << endl;
		system("pause");
	}


	closesocket(mySocket);
	WSACleanup();
	return 0;
}
