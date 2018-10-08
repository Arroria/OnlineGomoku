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

//Input Device
#include "./SDK/InputDevice.h"
#if _DEBUG
#pragma comment (lib, "./SDK/InputDeviceD.lib")
#else
#pragma comment (lib, "./SDK/InputDevice.lib")
#endif
inline InputDevice g_inputDevice;

//IME Device
#include "./SDK/IMEDevice.h"
#if _DEBUG
#pragma comment (lib, "./SDK/IMEDeviceD.lib")
#else
#pragma comment (lib, "./SDK/IMEDevice.lib")
#endif


#include "ProcessManager.h"
inline ProcessManager* g_processManager = nullptr;
#define DEVICE (g_processManager->GetDevice())

inline LPD3DXSPRITE g_sprite = nullptr;



#include <mutex>
using mutex_lock_guard = std::lock_guard<std::mutex>;
inline std::mutex g_coutMutex;
#define cout_region_lock std::lock_guard<std::mutex> locker(g_coutMutex)
#define locked_cout cout_region_lock; cout


