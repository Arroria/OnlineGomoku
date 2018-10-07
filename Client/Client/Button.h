#pragma once
#include <functional>

#include "UIObj.h"


class StaticButton
{
public:
	StaticButton(LPDIRECT3DTEXTURE9 tex, int posX, int posY, int l, int t, int r, int b, std::function<void()> clickFunc);
	~StaticButton();

public:
	void Update();
	void Render();

private:
	LPDIRECT3DTEXTURE9 m_tex;
	const int m_posX;
	const int m_posY;
	const RECT m_clickArea;

	std::function<void()> m_clickFunc;
};

class OffsetButton
{
public:
	OffsetButton(LPDIRECT3DTEXTURE9 tex, int posX, int posY, int l, int t, int r, int b, std::function<void()> clickFunc);
	~OffsetButton();

public:
	void Update(int offsetPosX, int offsetPosY);
	void Render(int offsetPosX, int offsetPosY);

private:
	LPDIRECT3DTEXTURE9 m_tex;
	const int m_posX;
	const int m_posY;
	const RECT m_clickArea;

	std::function<void()> m_clickFunc;
};

class Button
	: public UIObj
{
public:
	using Event_t = std::function<void()>;
	inline static Button* cast(UIObj* obj) { return dynamic_cast<Button*>(obj); }

public:
	Button();
	~Button();

	inline void SetArea(int l, int t, int r, int b)
	{
		m_collisionArea.left	= l;
		m_collisionArea.top		= t;
		m_collisionArea.right	= r;
		m_collisionArea.bottom	= b;
	}
	inline void SetEvent(Event_t eventFunc) { m_eventFunc = eventFunc; }
	inline void SetTex(LPDIRECT3DTEXTURE9 tex) { if (m_tex) m_tex->Release(); if (m_tex = tex) tex->AddRef(); }

private:
	void Update() override;
	void Render() override;

private:
	Event_t m_eventFunc;

	RECT m_collisionArea;

	LPDIRECT3DTEXTURE9 m_tex;
};
