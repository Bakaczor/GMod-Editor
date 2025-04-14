#pragma once
#include "Object.h"

namespace app {
	class Torus : public Object {
	public:
		Torus(double R = 0.5, double r = 0.25, unsigned int uParts = 32, unsigned int vParts = 32);

		double Get_R() const;
		void Set_R(double R);

		double Get_r() const;
		void Set_r(double r);

		int Get_uParts() const;
		void Set_uParts(int uParts);

		int Get_vParts() const;
		void Set_vParts(int vParts);

		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;
	private:
		const static int m_uPartsMin = 3;
		const static int m_vPartsMin = 3;
		static unsigned short m_globalTorusNum;
		double m_R, m_r;
		int m_uParts, m_vParts;

		Mesh m_mesh;
		std::vector<VERTEX> m_vertices;
		std::vector<EDGE> m_edges;

		void RecalculateGeometry();
	};
}