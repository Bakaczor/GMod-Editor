#pragma once
#include "Curve.h"

namespace app {
	class BSpline : public Curve {
	public:
		bool showBernstein = false;
		std::vector<Point*> bernsteinPoints;

		BSpline(std::vector<Object*> objects);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
	private:
		static unsigned short m_globalBSplineNum;
		Mesh m_bernsteinMesh;
	};
}