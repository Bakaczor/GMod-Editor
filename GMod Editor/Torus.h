#pragma once
#include "Object.h"

class Torus : public Object{
public:
	Torus(double R, double r, unsigned int uParts, unsigned int vParts);

	double Get_R() const;
	void Set_R(double R);

	double Get_r() const;
	void Set_r(double r);

	unsigned int Get_uParts() const;
	void Set_uParts(unsigned int uParts);

	unsigned int Get_vParts() const;
	void Set_vParts(unsigned int vParts);
private:
	const static unsigned int m_uPartsMin = 4;
	const static unsigned int m_vPartsMin = 4;
	static unsigned short m_globalCubeNum;
	double m_R, m_r;
	unsigned int m_uParts, m_vParts;
	void RecalculateGeometry();
};