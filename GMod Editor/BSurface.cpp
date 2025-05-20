#include "Application.h"
#include "BSurface.h"
#include "Point.h"
#include <numbers>

using namespace app;

unsigned short BSurface::m_globalBSurfaceNum = 0;

BSurface::BSurface(SurfaceType type, float a, float b, unsigned int aPatch, unsigned int bPatch, unsigned int divisions) {
	m_divisions = divisions;
	m_surfaceType = type;
	m_type = "BSurface";
	std::ostringstream os;
	os << "bsurface_" << m_globalBSurfaceNum;
	name = os.str();
	m_globalBSurfaceNum += 1;
	m_patches.reserve(aPatch * bPatch);

	if (type == SurfaceType::Flat) {
		m_aPoints = aPatch + 3;
		m_bPoints = bPatch + 3;
		m_controlPoints.reserve(m_aPoints * m_bPoints);

		for (unsigned int i = 0; i < m_aPoints; ++i) {
			for (unsigned int j = 0; j < m_bPoints; ++j) {
				auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);
				float x = (a * i) / (m_aPoints - 1) - a / 2.0f;
				float z = (b * j) / (m_bPoints - 1) - b / 2.0f;
				point->SetTranslation(x, 0.0f, z);
				m_controlPoints.push_back(std::move(point));
			}
		}

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
		m_aPoints = aPatch;
		m_bPoints = bPatch + 3;
		m_controlPoints.reserve(m_aPoints * m_bPoints);

		for (unsigned int i = 0; i < m_aPoints; ++i) {
			for (unsigned int j = 0; j < m_bPoints; ++j) {
				auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);
				float angle = (2 * std::numbers::pi_v<float> * i) / m_aPoints;
				float x = a * std::cos(angle);
				float y = a * std::sin(angle);
				float z = (b * j) / (m_bPoints - 1) - b / 2.0f;
				point->SetTranslation(x, z, y);
				m_controlPoints.push_back(std::move(point));
			}
		}

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
