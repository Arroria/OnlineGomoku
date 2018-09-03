#pragma once

#include <thread>
#include <mutex>
#include "arSocket.h"
class SocketReciver
{
public:
	SocketReciver(const SOCKET& socket, const sockaddr_in& address);
	SocketReciver(SocketReciver&& _other);
	~SocketReciver();
	SocketReciver& operator=(SocketReciver&& other);

	void Run();
	inline bool Joinable() { return m_reciveThread.joinable(); }
	inline void Join() { m_reciveThread.join(); }
	inline void Detach() { m_reciveThread.detach(); }

	inline const SOCKET&		Socket() const { return m_socket; }
	inline const sockaddr_in&	Address() const { return m_address; }
	inline operator const SOCKET&() const { return m_socket; }
	inline operator const sockaddr_in&() const { return m_address; }

	inline bool					IsRecived() const { return m_isRecived; }
	inline int					ReciveResult() const { return m_recvResult; }
	inline SocketBuffer&		RecivedData() { return m_socketBuffer; }
	inline const SocketBuffer&	RecivedData() const { return m_socketBuffer; }
	inline void					ReciveRestart() { m_isRecived = false; }

	inline void	lock() const { m_mutex.lock(); }
	inline void	unlock() const { m_mutex.unlock(); }

private:
	static void RecvLoop(SocketReciver& _this) { _this.RecvLoop(); }
	void RecvLoop();

private:
	SOCKET m_socket;
	sockaddr_in m_address;

	bool m_isRecived;
	int m_recvResult;
	SocketBuffer m_socketBuffer;
	
	std::thread m_reciveThread;
	mutable std::mutex m_mutex;
};

using RegionLocker_SocketReciver = std::lock_guard<SocketReciver>;
