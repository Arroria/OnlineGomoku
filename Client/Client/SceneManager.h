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
	inline void DontRenderNext() { m_ignoreNextRender = true; }
	void ChangeScene(Scene* nextScene, bool ignoreRender = true);

	void Update();
	void Render();

private:
	Scene * m_currScene;
	Scene * m_nextScene;
	bool m_ignoreNextRender;
};

