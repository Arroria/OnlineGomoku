#pragma once
#include "Singleton.h"

class Scene;
class SceneManager :
	public Singleton<SceneManager>
{
public:
	SceneManager();
	~SceneManager();

public:
	inline bool IsDontRender() { return m_ignoreNextRender; }
	inline void DontRenderNext() { m_ignoreNextRender = true; }
	void ChangeScene(Scene* nextScene, bool ignoreRender = true);

	void Update();
	void Render();
	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	Scene * m_currScene;
	Scene * m_nextScene;
	bool m_ignoreNextRender;
};

