#include "stdafx.h"
#include "UIObj.h"


UIObj::UIObj()
	: m_parent(nullptr)

	, m_objLocalPos(0, 0, 0)
	, m_objWorldPos(0, 0, 0)
{
}
UIObj::~UIObj()
{
	for (auto iter : m_child)
		if (iter)
			delete iter;
}



void UIObj::UpdateObj()
{
	m_objWorldPos = m_parent ?
		m_parent->ObjWorldPos() + m_objLocalPos :
		m_objLocalPos;

	Update();
	UpdateChild();
}

void UIObj::RenderObj()
{
	Render();
	RenderChild();
}



void UIObj::UpdateChild()
{
	for (auto iter : m_child)
		if (iter)
			iter->Update();
}

void UIObj::RenderChild()
{
	for (auto iter : m_child)
		if (iter)
			iter->Render();
}
