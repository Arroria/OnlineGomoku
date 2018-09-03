#include "JSON_Based_Socket.h"


int __ar_send(SOCKET socket, const arJSON & jsonRoot, int flags)
{
	SocketBuffer socBuf;
	SetSocketBuffer(jsonRoot.ToJSON(), socBuf);
	return __ar_send(socket, socBuf, flags);
}

int __ar_recv(SOCKET socket, arJSON& jsonRoot, int flags)
{
	SocketBuffer socBuf;
	int result = __ar_recv(socket, socBuf, flags);
	if (result > 0)
	{
		socBuf[result] = NULL;
		JSON_To_arJSON(socBuf.Buffer(), jsonRoot);
	}
	return result;
}
