#pragma once
#include <thread>
#include <mutex>
#include <functional>
#include "arSocket.h"
class AsyncConnector
{
public:
	using Returner_t = std::function<bool(AsyncConnector&, int, SocketBuffer&)>;


public:
	AsyncConnector(const SOCKET& socket, const sockaddr_in& address);
	~AsyncConnector();

	inline bool operator==(SOCKET _other) const { return m_socket == _other; }
	inline bool operator==(const AsyncConnector& _other) const { return m_socket == _other.m_socket; }
	inline bool operator>(const AsyncConnector& _other) const { return m_socket > _other.m_socket; }
	inline bool operator<(const AsyncConnector& _other) const { return m_socket < _other.m_socket; }


	void Run();
	inline bool Joinable()	{ return m_reciveThread.joinable(); }
	inline void Join()		{ m_reciveThread.join(); }
	inline void Detach()	{ m_reciveThread.detach(); }

	inline const SOCKET&		Socket() const	{ return m_socket; }
	inline const sockaddr_in&	Address() const	{ return m_address; }
	inline Returner_t	Returner() const				{ std::lock_guard<std::mutex> locker(m_mtxReturnerReferenced);	return m_returner; }
	inline void			Returner(Returner_t returner)	{ std::lock_guard<std::mutex> locker(m_mtxReturnerReferenced); m_returner = returner; }

	inline std::mutex&	ReturnerAccessMutex() const { return m_mtxReturnerAccessed; }

	inline operator const SOCKET&() const		{ return m_socket; }
	inline operator const sockaddr_in&() const	{ return m_address; }

private:
	void RecvLoop();
	
private:
	SOCKET m_socket;
	sockaddr_in m_address;

	std::thread m_reciveThread;
	Returner_t m_returner;
	mutable std::mutex m_mtxReturnerReferenced;
	mutable std::mutex m_mtxReturnerAccessed;
};

