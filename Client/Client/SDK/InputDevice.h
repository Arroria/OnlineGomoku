#pragma once
#include <Windows.h>
#include <array>

class InputDevice
{
private:
	static constexpr size_t c_keyMax = 256;

private:
	enum class KeyState
	{
		None,
		Pressed,
		Up,
		Down,
	};

	std::array<KeyState, c_keyMax> m_keyState;
	POINT m_mousePos;
	POINT m_mouseDelta;
	int m_wheelDelta;

	bool m_isWndActivate;

public:
	inline bool IsKeyNone		(size_t index) const	{ return m_keyState[index] == KeyState::None	|| m_keyState[index] == KeyState::Up; }
	inline bool IsKeyPressed	(size_t index) const	{ return m_keyState[index] == KeyState::Pressed	|| m_keyState[index] == KeyState::Down; }
	inline bool IsKeyUp			(size_t index) const	{ return m_keyState[index] == KeyState::Up; }
	inline bool IsKeyDown		(size_t index) const	{ return m_keyState[index] == KeyState::Down; }

	inline POINT	MousePos()		const	{ return m_mousePos; }
	inline POINT	MouseDelta()	const	{ return m_mouseDelta; }
	inline int		MouseWheel()	const	{ return m_wheelDelta; }

public:
	void BeginFrame(HWND& hWnd);
	void EndFrame();
	void MsgProc(UINT msg, WPARAM wParam, LPARAM lParam);
		
public:
	InputDevice();
	~InputDevice();
};

