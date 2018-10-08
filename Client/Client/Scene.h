#pragma once
class Scene
{
public:
	Scene();
	virtual ~Scene();

public:
	virtual void Init() PURE;
	virtual void Update() PURE;
	virtual void Render() PURE;
	virtual void Release() PURE;
	virtual LRESULT MsgProc(HWND, UINT, WPARAM, LPARAM) PURE;
};

