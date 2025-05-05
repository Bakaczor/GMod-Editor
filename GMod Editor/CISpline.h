#pragma once
#include "Curve.h"

namespace app {
	class CISpline : public Curve {
	public:
		CISpline(std::vector<Object*> objects);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
	private:
		static unsigned short m_globalCISplineNum;
		std::vector<gmod::vector3<double>> ComputeCoefficients() const;
	};
}