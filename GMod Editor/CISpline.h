#pragma once
#include "Spline.h"

namespace app {
	class CISpline : public Spline {
	public:
		CISpline(std::vector<Object*> objects);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
	private:
		static unsigned short m_globalCISplineNum;
		std::vector<gmod::vector3<double>> ComputeCoefficients(const std::vector<Object*>& points) const;
		std::vector<Object*> FilteredPoints(double tol = 1e-6) const;
	};
}