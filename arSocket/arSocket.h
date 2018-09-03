#pragma once
#include <winsock2.h>
#pragma comment (lib, "ws2_32.lib")
#pragma warning(disable : 4996)

#ifndef OUT
#define OUT
#endif

struct SocketError
{
	int errorCode;
	const char* errorName;
	SocketError() : errorCode(NULL), errorName(nullptr) {}
	SocketError(int _errorCode, const char* _errorName) : errorCode(_errorCode), errorName(_errorName) {}
};

bool __ar_WSAStartup(SocketError* errorReturn, WORD wVersionRequested, LPWSADATA lpWSAData);
bool __ar_socket(SocketError* errorReturn, int af, int type, int protocol, SOCKET* sock);
void __ar_make_sockaddrin(ADDRESS_FAMILY addressFamily, u_long ip, u_short port, sockaddr_in* socketAddress);
bool __ar_bind(SocketError* errorReturn, SOCKET& sock, const sockaddr * socketAddress);
bool __ar_listen(SocketError* errorReturn, SOCKET sock, int backlog);
bool __ar_accept(SocketError* errorReturn, SOCKET serverSocket, OUT SOCKET& clientSocket, OUT sockaddr& clientAddress);
bool __ar_connect(SocketError* errorReturn, SOCKET sock, const sockaddr& socketAddress);

inline bool __ar_accept(SocketError* errorReturn, SOCKET serverSocket, OUT SOCKET& clientSocket, OUT sockaddr_in& clientAddress)	{ return __ar_accept(errorReturn, serverSocket, clientSocket, (sockaddr&)clientAddress); }
inline bool __ar_connect(SocketError* errorReturn, SOCKET sock, const sockaddr_in& socketAddress)	{ return __ar_connect(errorReturn, sock, (sockaddr&)socketAddress); }



#include <stdint.h>
class SocketBuffer
{
public:
	using Length_t = int32_t;
	static constexpr size_t socketSize = 0xffff;
	static constexpr size_t lengthSize = sizeof(Length_t);
	static constexpr size_t bufferSize = socketSize - lengthSize;
	static constexpr auto LengthToN = htonl;
	static constexpr auto LengthToH = ntohl;

public:
	SocketBuffer();

	inline void DataLength(Length_t dataLength);
	inline char* Buffer();
	inline char* At(int index);
	inline char& operator[](int index);
	
	inline Length_t DataLength() const;
	inline const char* Buffer() const;
	inline const char* At(int index) const;
	inline const char& operator[](int index) const;
	
	friend int __ar_send(SOCKET socket, const SocketBuffer& data, int flags);

private:
	char m_buffer[socketSize];
};
#include "arSocket.inl"

int __ar_send(SOCKET socket, const SocketBuffer& socketBuffer, int flags = NULL);
int __ar_recv(SOCKET socket, SocketBuffer& socketBuffer, int flags = NULL);

#include <string>
void SetSocketBuffer(const std::string& data, OUT SocketBuffer& socketBuffer);
