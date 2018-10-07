#pragma once
#include "UIObj.h"
class UIEmpty final :
	public UIObj
{
public:
	UIEmpty() {}
	~UIEmpty() {}

private:
	void Update() override {}
	void Render() override {}
};

