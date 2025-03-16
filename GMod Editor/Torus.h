#pragma once
#include "Object.h"

class Torus : public Object{
public:
	Torus(double R, double r, unsigned int uParts, unsigned int vParts);

	virtual void UpdateMesh(const Device& device) override;

	void Set_R(double R);

	void Set_r(double r);

	void Set_uParts(unsigned int uParts);

	void Set_vParts(unsigned int vParts);
private:
	const static unsigned int m_uPartsMin = 4;
	const static unsigned int m_vPartsMin = 4;
	static unsigned short m_globalTorusNum;
	double m_R, m_r;
	unsigned int m_uParts, m_vParts;
	std::vector<VERTEX> m_vertices;
	std::vector<EDGE> m_edges;

	void RecalculateGeometry();
};