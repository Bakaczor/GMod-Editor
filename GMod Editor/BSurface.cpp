#include "BSurface.h"
#include <numbers>

using namespace app;

unsigned short BSurface::m_globalBSurfaceNum = 0;

BSurface::BSurface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> controlPoints) {
	m_surfaceType = type;
	m_aPoints = aPoints;
	m_bPoints = bPoints;
	m_divisions = divisions;

	m_type = "BSurface";
	std::ostringstream os;
	os << "bsurface_" << m_globalBSurfaceNum;
	name = os.str();
	m_globalBSurfaceNum += 1;

	std::copy(controlPoints.begin(), controlPoints.end(), std::back_inserter(m_controlPoints));
	
    if (m_surfaceType == SurfaceType::Flat) {
        unsigned int aPatch = m_aPoints - 3;
        unsigned int bPatch = m_bPoints - 3;

        for (unsigned int i = 0; i < aPatch; ++i) {
            for (unsigned int j = 0; j < bPatch; ++j) {
                std::array<USHORT, Patch::patchSize> indices;

                for (USHORT u = 0; u < 4; ++u) {
                    for (USHORT v = 0; v < 4; ++v) {
                        indices[u * 4 + v] = (i + u) * m_bPoints + (j + v);
                    }
                }
                m_patches.emplace_back(indices);
            }
        }
    } else {
        unsigned int aPatch = m_aPoints;
        unsigned int bPatch = m_bPoints - 3;

        for (unsigned int i = 0; i < aPatch; ++i) {
            for (unsigned int j = 0; j < bPatch; ++j) {
                std::array<USHORT, Patch::patchSize> indices;

                for (USHORT u = 0; u < Patch::rowSize; ++u) {
                    for (USHORT v = 0; v < Patch::rowSize; ++v) {
                        USHORT wrapped_i = (i + u) % m_aPoints;
                        indices[u * Patch::rowSize + v] = wrapped_i * m_bPoints + (j + v);
                    }
                }
                m_patches.emplace_back(indices);
            }
        }
    }

	UpdateMidpoint();
	for (auto& obj : m_controlPoints) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void BSurface::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showNet) {
		map.at(ShaderType::Regular).Set(context);
		m_netMesh.Render(context);
	}
	map.at(ShaderType::RegularWithTesselationBSurface).Set(context);
	m_surfaceMesh.Render(context);
}

std::pair<unsigned int, unsigned int> BSurface::NumberOfPatches() const {
	unsigned int aPatch = m_surfaceType == SurfaceType::Flat ? m_aPoints - 3 : m_aPoints;
	unsigned int bPatch = m_bPoints - 3;

	return std::make_pair(aPatch, bPatch);
}

std::array<double, 4> BSurface::N3(double t) {
	const double t2 = t * t;
	const double t3 = t2 * t;
	return {
		(1 + t * (-3 + t * (3 - t))) / 6,       
		(4 + 3 * t2 * (-2 + t)) / 6,          
		(1 + 3 * t * (1 + t * (1 - t))) / 6,
		t3 / 6                          
	};
}

std::array<double, 4> BSurface::dN3(double t) {
	double t2 = t * t;
	return {
		(-1 + t * (2 - t)) / 2,   
		t * (-4 + 3 * t) / 2,      
		(1 + t * (2 - 3 * t)) / 2,    
		t2 / 2               
	};
}

#pragma region IGEOMETRICAL
gmod::vector3<double> BSurface::Point(double u, double v) const {
	auto [localU, localV] = LocalUV(u, v);

	std::array<double, 4> Bu = N3(localU);
	std::array<double, 4> Bv = N3(localV);

	return sumBasis(GetPatch(u, v), Bu, Bv);
}

gmod::vector3<double> BSurface::Tangent(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	auto [localU, localV] = LocalUV(u, v);

	std::array<double, 4> Nu = N3(localU);
	std::array<double, 4> Nv = N3(localV);

	std::array<double, 4> dNu = dN3(localU);
	std::array<double, 4> dNv = dN3(localV);

	auto patch = GetPatch(u, v);
	gmod::vector3<double> du = normalize(sumBasis(patch, dNu, Nv));
	gmod::vector3<double> dv = normalize(sumBasis(patch, Nu, dNv));

	if (dPu) { *dPu = du; }
	if (dPv) { *dPv = dv; }

	return normalize(du + dv);
}
#pragma endregion
