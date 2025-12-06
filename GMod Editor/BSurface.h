#pragma once
#include "Surface.h"

namespace app {
	class BSurface : public Surface {
	public:
		BSurface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> m_controlPoints, int id = -1);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;
		virtual std::pair<unsigned int, unsigned int> NumberOfPatches() const override;

		struct Plane {
			std::vector<std::unique_ptr<app::Point>> controlPoints;
			std::unique_ptr<BSurface> surface;
		};
		static Plane MakePlane(gmod::vector3<double> centrePos, float width, float length, gmod::vector3<double> orientation, int id);
#pragma region IGEOMETRICAL
		virtual gmod::vector3<double> Point(double u, double v) const override;
		virtual gmod::vector3<double> Tangent(double u, double v, gmod::vector3<double>* dPu = nullptr, gmod::vector3<double>* dPv = nullptr) const override;
#pragma endregion
	private:
		static unsigned short m_globalBSurfaceNum;
		static std::array<double, 4> N3(double t);
		static std::array<double, 4> dN3(double t);
	};
}