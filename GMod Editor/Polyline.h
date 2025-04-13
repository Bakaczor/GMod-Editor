#pragma once
#include "ObjectGroup.h"
#include "Point.h"

class Polyline : public ObjectGroup {
public:
	Polyline(std::vector<Object*> objects);
	~Polyline() override;
	virtual void UpdateMesh(const Device& device) override;
	virtual void RenderProperties() override;
private:
	static unsigned short m_globalPointlineNum;
	Mesh m_mesh;
};