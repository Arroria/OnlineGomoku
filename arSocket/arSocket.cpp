#include "arSocket.h"

bool __ar_WSAStartup(SocketError* errorReturn, WORD wVersionRequested, LPWSADATA lpWSAData)
{
	int result;
	if (result = WSAStartup(0x0202, lpWSAData))
	{
		const char* errorStr = nullptr;
		switch (result)
		{
		case WSASYSNOTREADY:	errorStr = "WSASYSNOTREADY";		break;
		case WSAVERNOTSUPPORTED:errorStr = "WSAVERNOTSUPPORTED";	break;
		case WSAEINPROGRESS:	errorStr = "WSAEINPROGRESS";		break;
		case WSAEPROCLIM:		errorStr = "WSAEPROCLIM";			break;
		case WSAEFAULT:			errorStr = "WSAEFAULT";				break;
		default:				errorStr = "Undefined errorCode.";	break;
		}

		if (errorReturn)
			*errorReturn = SocketError(result, errorStr);
		return true;
	}
	return false;
}

bool __ar_socket(SocketError* errorReturn, int af, int type, int protocol, SOCKET* sock)
{
	SOCKET _sock;
	if ((_sock = socket(af, type, protocol)) == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		const char* errorStr = nullptr;
		switch (error)
		{
		case WSANOTINITIALISED:		errorStr = "WSANOTINITIALISED";			break;
		case WSAENETDOWN:			errorStr = "WSAENETDOWN";				break;
		case WSAEAFNOSUPPORT:		errorStr = "WSAEAFNOSUPPORT";			break;
		case WSAEINPROGRESS:		errorStr = "WSAEINPROGRESS";			break;
		case WSAEMFILE:				errorStr = "WSAEMFILE";					break;
		case WSAEINVAL:				errorStr = "WSAEINVAL";					break;
		case WSAEINVALIDPROVIDER:	errorStr = "WSAEINVALIDPROVIDER";		break;
		case WSAEINVALIDPROCTABLE:	errorStr = "WSAEINVALIDPROCTABLE";		break;
		case WSAENOBUFS:			errorStr = "WSAENOBUFS";				break;
		case WSAEPROTONOSUPPORT:	errorStr = "WSAEPROTONOSUPPORT";		break;
		case WSAEPROTOTYPE:			errorStr = "WSAEPROTOTYPE";				break;
		case WSAEPROVIDERFAILEDINIT:errorStr = "WSAEPROVIDERFAILEDINIT";	break;
		case WSAESOCKTNOSUPPORT:	errorStr = "WSAESOCKTNOSUPPORT";		break;
		default:					errorStr = "Undefined errorCode.";		break;
		}

		if (errorReturn)
			*errorReturn = SocketError(error, errorStr);
		return true;
	}
	else
		*sock = _sock;
	return false;
}

void __ar_make_sockaddrin(ADDRESS_FAMILY addressFamily, u_long ip, u_short port, sockaddr_in* socketAddress)
{
	ZeroMemory(socketAddress, sizeof(sockaddr_in));
	socketAddress->sin_family = addressFamily;
	socketAddress->sin_addr.s_addr = (ip);
	socketAddress->sin_port = (port);
}

bool __ar_bind(SocketError* errorReturn, SOCKET& sock, const sockaddr * socketAddress)
{
	if (bind(sock, socketAddress, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		const char* errorStr = nullptr;
		switch (error)
		{
		case WSANOTINITIALISED:	errorStr = "WSANOTINITIALISED";		break;
		case WSAENETDOWN:		errorStr = "WSAENETDOWN";			break;
		case WSAEACCES:			errorStr = "WSAEACCES";				break;
		case WSAEADDRINUSE:		errorStr = "WSAEADDRINUSE";			break;
		case WSAEADDRNOTAVAIL:	errorStr = "WSAEADDRNOTAVAIL";		break;
		case WSAEFAULT:			errorStr = "WSAEFAULT";				break;
		case WSAEINPROGRESS:	errorStr = "WSAEINPROGRESS";		break;
		case WSAEINVAL:			errorStr = "WSAEINVAL";				break;
		case WSAENOBUFS:		errorStr = "WSAENOBUFS";			break;
		case WSAENOTSOCK:		errorStr = "WSAENOTSOCK";			break;
		default:				errorStr = "Undefined errorCode.";	break;
		}

		if (errorReturn)
			*errorReturn = SocketError(error, errorStr);
		return true;
	}
	return false;
}

bool __ar_listen(SocketError* errorReturn, SOCKET sock, int backlog)
{
	if (listen(sock, backlog) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		const char* errorStr = nullptr;
		switch (error)
		{
		case WSANOTINITIALISED:	errorStr = "WSANOTINITIALISED";		break;
		case WSAENETDOWN:		errorStr = "WSAENETDOWN";			break;
		case WSAEADDRINUSE:		errorStr = "WSAEADDRINUSE";			break;
		case WSAEINPROGRESS:	errorStr = "WSAEINPROGRESS";		break;
		case WSAEINVAL:			errorStr = "WSAEINVAL";				break;
		case WSAEISCONN:		errorStr = "WSAEISCONN";			break;
		case WSAEMFILE:			errorStr = "WSAEMFILE";				break;
		case WSAENOBUFS:		errorStr = "WSAENOBUFS";			break;
		case WSAENOTSOCK:		errorStr = "WSAENOTSOCK";			break;
		case WSAEOPNOTSUPP:		errorStr = "WSAEOPNOTSUPP";			break;
		default:				errorStr = "Undefined errorCode.";	break;
		}

		if (errorReturn)
			*errorReturn = SocketError(error, errorStr);
		return true;
	}
	return false;
}

bool __ar_accept(SocketError* errorReturn, SOCKET serverSocket, OUT SOCKET& clientSocket, OUT sockaddr& clientAddress)
{
	SOCKET _clientSocket;
	int clientAddressSize = sizeof(sockaddr);
	if ((_clientSocket = accept(serverSocket, &clientAddress, &clientAddressSize)) == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		const char* errorStr = nullptr;
		switch (error)
		{
		case WSANOTINITIALISED:	errorStr = "WSANOTINITIALISED";		break;
		case WSAECONNRESET:		errorStr = "WSAECONNRESET";			break;
		case WSAEFAULT:			errorStr = "WSAEFAULT";				break;
		case WSAEINTR:			errorStr = "WSAEINTR";				break;
		case WSAEINVAL:			errorStr = "WSAEINVAL";				break;
		case WSAEINPROGRESS:	errorStr = "WSAEINPROGRESS";		break;
		case WSAEMFILE:			errorStr = "WSAEMFILE";				break;
		case WSAENETDOWN:		errorStr = "WSAENETDOWN";			break;
		case WSAENOBUFS:		errorStr = "WSAENOBUFS";			break;
		case WSAENOTSOCK:		errorStr = "WSAENOTSOCK";			break;
		case WSAEOPNOTSUPP:		errorStr = "WSAEOPNOTSUPP";			break;
		case WSAEWOULDBLOCK:	errorStr = "WSAEWOULDBLOCK";		break;
		default:				errorStr = "Undefined errorCode.";	break;
		}

		if (errorReturn)
			*errorReturn = SocketError(error, errorStr);
		return true;
	}
	else
		clientSocket = _clientSocket;
	return false;
}

bool __ar_connect(SocketError* errorReturn, SOCKET sock, const sockaddr& socketAddress)
{
	if (connect(sock, &socketAddress, sizeof(sockaddr)) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		const char* errorStr = nullptr;
		switch (error)
		{
		case WSANOTINITIALISED:	errorStr = "WSANOTINITIALISED";		break;
		case WSAENETDOWN:		errorStr = "WSAENETDOWN";			break;
		case WSAEADDRINUSE:		errorStr = "WSAEADDRINUSE";			break;
		case WSAEINTR:			errorStr = "WSAEINTR";				break;
		case WSAEINPROGRESS:	errorStr = "WSAEINPROGRESS";		break;
		case WSAEALREADY:		errorStr = "WSAEALREADY";			break;
		case WSAEADDRNOTAVAIL:	errorStr = "WSAEADDRNOTAVAIL";		break;
		case WSAEAFNOSUPPORT:	errorStr = "WSAEAFNOSUPPORT";		break;
		case WSAECONNREFUSED:	errorStr = "WSAECONNREFUSED";		break;
		case WSAEFAULT:			errorStr = "WSAEFAULT";				break;
		case WSAEINVAL:			errorStr = "WSAEINVAL";				break;
		case WSAEISCONN:		errorStr = "WSAEISCONN";			break;
		case WSAENETUNREACH:	errorStr = "WSAENETUNREACH";		break;
		case WSAEHOSTUNREACH:	errorStr = "WSAEHOSTUNREACH";		break;
		case WSAENOBUFS:		errorStr = "WSAENOBUFS";			break;
		case WSAENOTSOCK:		errorStr = "WSAENOTSOCK";			break;
		case WSAETIMEDOUT:		errorStr = "WSAETIMEDOUT";			break;
		case WSAEWOULDBLOCK:	errorStr = "WSAEWOULDBLOCK";		break;
		case WSAEACCES:			errorStr = "WSAEACCES";				break;
		default:				errorStr = "Undefined errorCode.";	break;
		}

		if (errorReturn)
			*errorReturn = SocketError(error, errorStr);
		return true;
	}
	return false;
}




SocketBuffer::SocketBuffer() : m_buffer{ NULL } {}




int __ar_send(SOCKET socket, const SocketBuffer& socketBuffer, int flags)
{
	return send(socket, (char*)&socketBuffer, socketBuffer.DataLength() + SocketBuffer::lengthSize, flags);
}

int __ar_recv(SOCKET socket, SocketBuffer& socketBuffer, int flags)
{
	int result = NULL;
	
	if ((result = recv(socket, (char*)&socketBuffer, SocketBuffer::lengthSize, flags)) <= 0)
		return result;
	
	const SocketBuffer::Length_t dataSize = socketBuffer.DataLength();
	SocketBuffer::Length_t recvedSize = 0;
	while (dataSize > recvedSize)
	{
		if ((result = recv(socket, &socketBuffer[recvedSize], dataSize - recvedSize, flags)) > 0)
			recvedSize += result;
		else
			return result;
	}
	return recvedSize;
}




void SetSocketBuffer(const std::string & data, OUT SocketBuffer& socketBuffer)
{
	strcpy(socketBuffer.Buffer(), data.data());
	socketBuffer[data.size()] = NULL;
	socketBuffer.DataLength(data.size());
}
