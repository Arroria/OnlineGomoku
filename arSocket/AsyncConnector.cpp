#include "AsyncConnector.h"



AsyncConnector::AsyncConnector(const SOCKET & socket, const sockaddr_in & address)
	: m_socket(socket)
	, m_address(address)
{
}
AsyncConnector::~AsyncConnector()
{
	if (Joinable())
		std::terminate();
}


void AsyncConnector::Run()
{
	if (Joinable())
		std::terminate();

	using StaticRecvLoop = void(*)(AsyncConnector&);
	m_reciveThread = std::thread(static_cast<StaticRecvLoop>(&AsyncConnector::RecvLoop), std::ref(*this));
}





void AsyncConnector::RecvLoop()
{
	SocketBuffer socketBuffer;
	bool run = true;
	while (run)
	{
		int result = __ar_recv(m_socket, socketBuffer);
		while (true)
		{
			Returner_t returner = Returner();
			if (returner)
			{
				if (returner(*this, result, socketBuffer))
					run = false;
				break;
			}
		}
	}
}
