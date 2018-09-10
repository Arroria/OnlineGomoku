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
	m_reciveThread = std::thread([this]() { RecvLoop(); });
}





void AsyncConnector::RecvLoop()
{
	SocketBuffer socketBuffer;
	bool run = true;
	while (run)
	{
		int result = __ar_recv(m_socket, socketBuffer);
		if (result < 0)
		{
			int a = WSAGetLastError();
			int b = 1;
		}

		while (true)
		{
			std::lock_guard<std::mutex> locker(ReturnerAccessMutex());
			Returner_t returner = Returner();
			if (returner)
			{
				if (returner(*this, result, socketBuffer))
					int a= 6;// run = false;
				break;
			}
		}
	}
}
