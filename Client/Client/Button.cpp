#include "stdafx.h"
#include "Button.h"


StaticButton::StaticButton(LPDIRECT3DTEXTURE9 tex, int posX, int posY, int l, int t, int r, int b, std::function<void()> clickFunc)
	: m_tex(tex)
	, m_posX(posX)
	, m_posY(posY)
	, m_clickArea{ l, t, r, b }

	, m_clickFunc(clickFunc)
{
}

StaticButton::~StaticButton()
{
}



void StaticButton::Update()
{
	if (!m_clickFunc)
		return;

	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		const POINT mousePos = g_inputDevice.MousePos();
		auto IsMouseIn = [&mousePos](int left, int top, int right, int bottom)->bool
		{
			return
				left <= mousePos.x &&
				top <= mousePos.y &&
				mousePos.x < right &&
				mousePos.y < bottom;
		};

		if (IsMouseIn(
			m_posX + m_clickArea.left,
			m_posY + m_clickArea.top,
			m_posX + m_clickArea.right,
			m_posY + m_clickArea.bottom))
			m_clickFunc();
	}
}

void StaticButton::Render()
{
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };
	Draw(m_tex, m_posX, m_posY);
}






OffsetButton::OffsetButton(LPDIRECT3DTEXTURE9 tex, int posX, int posY, int l, int t, int r, int b, std::function<void()> clickFunc)
	: m_tex(tex)
	, m_posX(posX)
	, m_posY(posY)
	, m_clickArea{ l, t, r, b }

	, m_clickFunc(clickFunc)
{
}

OffsetButton::~OffsetButton()
{
}



void OffsetButton::Update(int offsetPosX, int offsetPosY)
{
	if (!m_clickFunc)
		return;

	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		const POINT mousePos = g_inputDevice.MousePos();
		auto IsMouseIn = [&mousePos](int left, int top, int right, int bottom)->bool
		{
			return
				left <= mousePos.x &&
				top <= mousePos.y &&
				mousePos.x < right &&
				mousePos.y < bottom;
		};

		if (IsMouseIn(
			offsetPosX + m_posX + m_clickArea.left,
			offsetPosY + m_posY + m_clickArea.top,
			offsetPosX + m_posX + m_clickArea.right,
			offsetPosY + m_posY + m_clickArea.bottom))
			m_clickFunc();
	}
}

void OffsetButton::Render(int offsetPosX, int offsetPosY)
{
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };
	Draw(m_tex, m_posX + offsetPosX, m_posY + offsetPosY);
}








Button::Button()
	: UIObj()

	, m_eventFunc(nullptr)
	, m_collisionArea{ NULL }
	, m_tex(nullptr)
{
}
Button::~Button()
{
	if (m_tex)
		m_tex->Release();
}



void Button::Update()
{
	if (!m_eventFunc)
		return;

	if (g_inputDevice.IsKeyDown(VK_LBUTTON))
	{
		const POINT mousePos = g_inputDevice.MousePos();
		auto IsMouseIn = [&mousePos](int left, int top, int right, int bottom)->bool
		{
			return
				left <= mousePos.x &&
				top <= mousePos.y &&
				mousePos.x < right &&
				mousePos.y < bottom;
		};

		if (IsMouseIn(
			ObjWorldPos().x + m_collisionArea.left,
			ObjWorldPos().y + m_collisionArea.top,
			ObjWorldPos().x + m_collisionArea.right,
			ObjWorldPos().y + m_collisionArea.bottom))
			m_eventFunc();
	}
}

void Button::Render()
{
	auto Draw = [](LPDIRECT3DTEXTURE9 tex, int x, int y) { g_sprite->Draw(tex, nullptr, nullptr, &D3DXVECTOR3(x, y, 0), D3DXCOLOR(1, 1, 1, 1)); };
	if (m_tex)
		Draw(m_tex, ObjWorldPos().x, ObjWorldPos().y);
}
