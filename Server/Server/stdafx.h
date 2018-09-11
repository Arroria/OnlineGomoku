#pragma once

//STL
#define _WINSOCKAPI_
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <Windows.h>
#include <iostream>
using std::cout;
using std::endl;

//arSocket
#include <arSocket.h>
#include <arJSON.h>
#include <JSON_Based_Socket.h>
#include <SocketReciver.h>
#include <AsyncConnector.h>

#include <mutex>
using mutex_lock_guard = std::lock_guard<std::mutex>;
extern std::mutex g_coutMutex;
#define cout_region_lock std::lock_guard<std::mutex> locker(g_coutMutex)

#define server_log_error(error)	{ cout_region_lock; std::cout << "Error : " << error; }
#define server_log_note(note)	{ cout_region_lock; std::cout << "Note : " << note; }
#define server_srdlog_error(error)	(std::thread([&, this]() { server_log_error(error); }).detach())
#define server_srdlog_note(note)	(std::thread([&, this]() { server_log_error(note); }).detach())

class GomokuLobby;
extern GomokuLobby* g_server;