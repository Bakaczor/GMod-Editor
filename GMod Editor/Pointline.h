#pragma once
#include "Selection.h"
#include "Point.h"

class Pointline : public Selection {
public:
	Pointline(std::vector<Object*> objects);
	virtual void UpdateMesh(const Device& device) override;
	virtual void RenderProperties() override;
	virtual void RemoveReferences() override;
private:
	static unsigned short m_globalPointlineNum;
};