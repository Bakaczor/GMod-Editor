#pragma once
#include "Object.h"

class Point : public Object {
public:
	Point();
	void AddParent(const Object* obj);
	void InformParents();
	virtual void UpdateMesh(const Device& device) override;
private:
	std::vector<std::shared_ptr<Object>> m_parents;
	const float m_r = 0.05f;
	const int m_parts = 12;
	static unsigned short m_globalPointNum;
	void InitGeometry();
};