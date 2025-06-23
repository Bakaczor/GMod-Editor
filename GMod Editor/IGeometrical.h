#pragma once
#include "../gmod/vector3.h"

namespace app {
	class IGeometrical {
	public:
		struct XYZBounds {
			gmod::vector3<double> min;
			gmod::vector3<double> max;
		};
		virtual XYZBounds WorldBounds() const = 0;
		struct UVBounds {
			double uMin;
			double uMax;
			double vMin;
			double vMax;
		};
		virtual UVBounds ParametricBounds() const = 0;
		virtual bool IsUClosed() const = 0;
		virtual bool IsVClosed() const = 0;
		virtual gmod::vector3<double> Point(double u, double v) const = 0;
		virtual gmod::vector3<double> Tangent(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const = 0;
		virtual gmod::vector3<double> Normal(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const = 0;
	};
}
