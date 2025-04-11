#pragma once
#include "Object.h"

class Point : public Object {
public:
	Point();
	~Point() override;
	virtual void AddParent(Object* obj) override;
	virtual void RemoveParent(Object* obj) override;
	virtual void InformParents() override;
	virtual void UpdateMesh(const Device& device) override;
private:
	const float m_r = 0.05f;
	const int m_parts = 12;
	static unsigned short m_globalPointNum;
	void InitGeometry();
};