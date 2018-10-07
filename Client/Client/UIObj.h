#pragma once
class UIObj
{
public:
	UIObj();
	virtual ~UIObj();

public:
	inline void ChangeParent(const UIObj* parent)	{ m_parent = parent; }
	inline void InsertChild	(UIObj* child)			{ m_child.push_back(child); }

	inline D3DXVECTOR3&			ObjLocalPos() { return m_objLocalPos; }
	inline const D3DXVECTOR3&	ObjLocalPos() const { return m_objLocalPos; }
	inline const D3DXVECTOR3&	ObjWorldPos() const { return m_objWorldPos; }

	void UpdateObj();
	void RenderObj();

protected:
	inline const UIObj*	Parent() { return m_parent; }

private:
	virtual void Update() PURE;
	virtual void Render() PURE;
	
	void UpdateChild();
	void RenderChild();

private:
	const UIObj* m_parent;
	std::list<UIObj*> m_child;

	D3DXVECTOR3 m_objLocalPos;
	D3DXVECTOR3 m_objWorldPos;
};

