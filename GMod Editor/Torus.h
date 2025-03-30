#pragma once
#include "Object.h"

class Torus : public Object{
public:
	Torus(double R, double r, unsigned int uParts, unsigned int vParts);

	double Get_R() const;
	void Set_R(double R);

	double Get_r() const;
	void Set_r(double r);

	int Get_uParts() const;
	void Set_uParts(int uParts);

	int Get_vParts() const;
	void Set_vParts(int vParts);

	virtual void RenderProperties() override;
private:
	const static int m_uPartsMin = 3;
	const static int m_vPartsMin = 3;
	static unsigned short m_globalTorusNum;
	double m_R, m_r;
	int m_uParts, m_vParts;
	void RecalculateGeometry();
};