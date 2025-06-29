#pragma once
#include "Polyline.h"

namespace app {
	class Spline : public Polyline {
	public:
		explicit Spline(bool increment = false);
		Spline(std::vector<Object*> objects);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
	protected:
		bool m_showPolyline = true;
		Mesh m_curveMesh;
	private:
		static unsigned short m_globalSplineNum;
	};
}