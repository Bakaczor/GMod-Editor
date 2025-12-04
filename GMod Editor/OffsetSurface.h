#pragma once
#include "Object.h"
#include "IGeometrical.h"

namespace app {
	class OffsetSurface : public IGeometrical {
	public:
		OffsetSurface(Object* obj, float radius);

		virtual XYZBounds WorldBounds() const override;
		virtual UVBounds ParametricBounds() const override;
		virtual bool IsUClosed() const override;
		virtual bool IsVClosed() const override;
		virtual gmod::vector3<double> Point(double u, double v) const override;
		virtual gmod::vector3<double> Tangent(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const override;
		virtual gmod::vector3<double> Normal(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const override;
	private:
		IGeometrical* m_g = nullptr;
		float m_radius;
	};
}