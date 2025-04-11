#pragma once
#include "ObjectGroup.h"
#include "Point.h"

class Pointline : public ObjectGroup {
public:
	Pointline(std::vector<Object*> objects);
	~Pointline() override;
	virtual void UpdateMesh(const Device& device) override;
	virtual void RenderProperties() override;
private:
	static unsigned short m_globalPointlineNum;
};