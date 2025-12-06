#include "BSurface.h"
#include <numbers>
#include "Application.h"
#include "../gmod/utility.h"

using namespace app;

unsigned short BSurface::m_globalBSurfaceNum = 0;

BSurface::Plane BSurface::MakePlane(gmod::vector3<double> centrePos, float width, float length, gmod::vector3<double> orientation, int id) {
	Plane plane;

	// BSurface is three times smaller then normal surface
	width *= 3;
	length *= 3;

	const unsigned int aPoints = 4;
	const unsigned int bPoints = 4;
	plane.controlPoints.reserve(aPoints * bPoints);
	for (unsigned int i = 0; i < aPoints; ++i) {
		for (unsigned int j = 0; j < bPoints; ++j) {
			auto point = std::make_unique<app::Point>(Application::m_pointModel.get());
			point->deletable = false;

			float x = (width * i) / (aPoints - 1) - width / 2.0f;
			float z = (length * j) / (bPoints - 1) - length / 2.0f;

			// create plane in XZ plane
			point->SetTranslation(x + centrePos.x(), centrePos.y(), z + centrePos.z());
			plane.controlPoints.push_back(std::move(point));
		}
	}

	std::vector<Object*> referencePoints;
	referencePoints.reserve(aPoints * bPoints);
	for (auto& p : plane.controlPoints) {
		referencePoints.push_back(p.get());
	}
	plane.surface = std::make_unique<BSurface>(SurfaceType::Flat, aPoints, bPoints, 2, referencePoints, id);
	plane.surface->SetRotation(gmod::deg2rad(orientation.x()), gmod::deg2rad(orientation.y()), gmod::deg2rad(orientation.z()));

	return plane;
}


BSurface::BSurface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> controlPoints, int id) {
	intersectable = true;
	m_surfaceType = type;
	m_aPoints = aPoints;
	m_bPoints = bPoints;
	m_divisions = divisions;

	if (id != -1) {
		this->id = id;
	}

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
