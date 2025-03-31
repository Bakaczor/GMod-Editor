#pragma once
#include "Point.h"

class Pointline : public Object {
public:
	Pointline(std::vector<std::shared_ptr<Object>> points);
	virtual void UpdateMesh(const Device& device);
	virtual void RenderProperties();
private:
	static unsigned short m_globalPolylineNum;
	std::vector<std::shared_ptr<Object>> m_points;
};