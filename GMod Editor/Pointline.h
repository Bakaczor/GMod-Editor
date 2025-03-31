#pragma once
#include "Selection.h"
#include "Point.h"

class Pointline : public Selection {
public:
	Pointline();
	virtual void UpdateMesh(const Device& device);
	virtual void RenderProperties();
private:
	static unsigned short m_globalPolylineNum;
};