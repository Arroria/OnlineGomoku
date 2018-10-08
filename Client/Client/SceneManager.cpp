#include "stdafx.h"
#include "SceneManager.h"

#include "Scene.h"

SceneManager::SceneManager()
	: m_currScene(nullptr)
	, m_nextScene(nullptr)

	, m_ignoreNextRender(false)
{
}

SceneManager::~SceneManager()
{
}



void SceneManager::ChangeScene(Scene * nextScene, bool ignoreRender)
{
	m_nextScene = nextScene;
	if (ignoreRender)
		DontRenderNext();
}



void SceneManager::Update()
{
	if (m_nextScene)
	{
		if (m_currScene)
		{
			m_currScene->Release();
			delete m_currScene;
		}

		m_nextScene->Init();
		m_currScene = m_nextScene;
		m_nextScene = nullptr;
	}

	if (m_currScene)
		m_currScene->Update();
}

void SceneManager::Render()
{
	if (m_currScene)
	{
		if (m_ignoreNextRender)
			m_ignoreNextRender = false;
		else
		{
			g_processManager->DefaultClear();
			m_currScene->Render();
		}
	}
}

LRESULT SceneManager::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (m_currScene)
		return m_currScene->MsgProc(hWnd, msg, wParam, lParam);
	return NULL;
}
