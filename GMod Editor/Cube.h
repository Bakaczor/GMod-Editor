#pragma once
#include "Object.h"

class Cube : public Object {
public:
	Cube();
private:
	static unsigned short m_globalCubeNum;
	void RecalculateGeometry();
};