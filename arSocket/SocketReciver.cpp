#include "SocketReciver.h"


SocketReciver::SocketReciver(const SOCKET& socket, const sockaddr_in& address)
	: m_socket(socket)
	, m_address(address)

	, m_isRecived(false)
	, m_recvResult(NULL)
	, m_socketBuffer()
	
	, m_reciveThread()
	, m_mutex()
{
}
SocketReciver::SocketReciver(SocketReciver&& _other)
{
	*this = std::forward<SocketReciver>(_other);
}
SocketReciver::~SocketReciver()
{
	if (m_reciveThread.joinable())
		std::terminate();
}



SocketReciver& SocketReciver::operator=(SocketReciver&& other)
{
	m_mutex.lock();
	other.m_mutex.lock();
	
	
	m_socket		= other.m_socket;	other.m_socket = NULL;
	m_address		= other.m_address;	other.m_address = { NULL };

	m_isRecived		= other.m_isRecived;	other.m_isRecived = false;
	m_recvResult	= other.m_recvResult;	other.m_recvResult = NULL;
	m_socketBuffer	= other.m_socketBuffer;	//other.m_socketBuffer = { NULL };

	m_reciveThread	= std::move(m_reciveThread);
	

	other.m_mutex.unlock();
	m_mutex.unlock();
	return *this;
}



void SocketReciver::Run()
{
	if (m_reciveThread.joinable())
		std::terminate();
	m_reciveThread = std::thread(static_cast<void(*)(SocketReciver&)>(RecvLoop), std::ref(*this));
}

void SocketReciver::RecvLoop()
{
	while (true)
	{
		std::lock_guard<std::mutex> locker(m_mutex);
		if (!m_isRecived)
		{
			SocketBuffer temp;
			if (__ar_recv(m_socket, temp, MSG_PEEK))
			{
				m_recvResult = __ar_recv(m_socket, m_socketBuffer);
				m_isRecived = true;
			}
		}
	}
}
