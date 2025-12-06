#include "OffsetSurface.h"

using namespace app;

OffsetSurface::OffsetSurface(Object* obj, float radius, bool useNumerical) : m_radius(radius), m_useNumerical(useNumerical) {
	IGeometrical* g = dynamic_cast<IGeometrical*>(obj);
	if (g == nullptr) {
		throw std::exception("This object does not implement IGeometrical interface.");
	}
	m_g = g;
}

IGeometrical::XYZBounds OffsetSurface::WorldBounds() const {
	throw std::logic_error("Unimplemented. Do not use.");
}

IGeometrical::UVBounds OffsetSurface::ParametricBounds() const {
	return m_g->ParametricBounds();
}

bool OffsetSurface::IsUClosed() const {
	return m_g->IsUClosed();
}

bool OffsetSurface::IsVClosed() const {
	return m_g->IsVClosed();
}

gmod::vector3<double> OffsetSurface::Point(double u, double v) const {
	if (m_useNumerical) {
		return m_g->Point(u, v) + m_radius * NumericalNormal(u, v);
	} else {
		return m_g->Point(u, v) + m_radius * m_g->Normal(u, v);
	}
}

gmod::vector3<double> OffsetSurface::Tangent(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	return m_g->Tangent(u, v, dPu, dPv);
}

gmod::vector3<double> OffsetSurface::Normal(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	return m_g->Normal(u, v, dPu, dPv);
}

gmod::vector3<double> OffsetSurface::NumericalNormal(double u, double v) const {
	const auto& bound = m_g->ParametricBounds();
	const double stepU = (bound.uMax - bound.uMin) / m_res;
	const double stepV = (bound.vMax - bound.vMin) / m_res;

	const double u_md = u - stepU;
	const double u_pd = u + stepU;
	const double v_md = v - stepV;
	const double v_pd = v + stepV;

	gmod::vector3<double> left = m_g->Point(u_md, v);
	gmod::vector3<double> rigth = m_g->Point(u_pd, v);
	gmod::vector3<double> top = m_g->Point(u, v_pd);
	gmod::vector3<double> bottom = m_g->Point(u, v_md);

	gmod::vector3<double> du = (rigth - left) * (0.5 / stepU);
	gmod::vector3<double> dv = (top - bottom) * (0.5 / stepV);

	return normalize(cross(du, dv));
}
