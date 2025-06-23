#pragma once
#include "Object.h"
#include "IGeometrical.h"
#include "../gmod/utility.h"

namespace app {
	class Torus : public Object, public IGeometrical {
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

		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual void UpdateMesh(const Device& device) override;
		virtual void RenderProperties() override;

#pragma region IGEOMETRICAL
		virtual XYZBounds WorldBounds() const override;
		virtual UVBounds ParametricBounds() const override;
		virtual bool IsUClosed() const override;
		virtual bool IsVClosed() const override;
		virtual gmod::vector3<double> Point(double u, double v) const override;
		virtual gmod::vector3<double> Tangent(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const override;
		virtual gmod::vector3<double> Normal(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const override;
#pragma endregion
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
		gmod::vector3<double> LocalToWorld(const gmod::vector3<double>& p) const;
	};
}