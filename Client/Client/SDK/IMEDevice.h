#pragma once
#include <Windows.h>
#include <string>

class IMEDevice
{
private:
	std::wstring m_inputBuffer;
	std::wstring m_typingData;

public:
	std::wstring GetString();
	void Clear();

public:
	void MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

public:
	IMEDevice();
	~IMEDevice();
};

