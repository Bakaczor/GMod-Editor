#pragma once
#include "ObjectGroup.h"
#include "Point.h"

namespace app {
	class Polyline : public ObjectGroup {
	public:
		Polyline(std::vector<Object*> objects);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
		virtual gmod::matrix4<double> modelMatrix() const override;
	protected:
		Mesh m_mesh;
	private:
		static unsigned short m_globalPolylineNum;
		int m_selectedIdx = -1;
	};
}