#include "OffsetSurface.h"

using namespace app;

OffsetSurface::OffsetSurface(Object* obj, float radius) : m_radius(radius) {
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
	return m_g->Point(u, v) + m_radius * m_g->Normal(u, v);
}

gmod::vector3<double> OffsetSurface::Tangent(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	return m_g->Tangent(u, v, dPu, dPv);
}

gmod::vector3<double> OffsetSurface::Normal(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	return m_g->Normal(u, v, dPu, dPv);
}
