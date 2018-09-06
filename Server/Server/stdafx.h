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


class GomokuLobby;
extern GomokuLobby* g_server;