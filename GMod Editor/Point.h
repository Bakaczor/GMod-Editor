#pragma once
#include "Object.h"

class Point : public Object {
public:
	Point();
	virtual void UpdateMesh(const Device& device) override;
private:
	static unsigned short m_globalPointNum;
	void InitGeometry();
};