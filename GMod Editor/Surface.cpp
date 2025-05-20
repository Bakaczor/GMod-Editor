#include "framework.h"
#include "Application.h"
#include "Point.h"
#include "Surface.h"
#include <numbers>

using namespace app;

unsigned short Surface::m_globalSurfaceNum = 0;

Surface::Surface(bool increment) : m_divisions(Patch::rowSize), m_aPoints(0), m_bPoints(0), m_surfaceType(SurfaceType::Flat) {
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	if (increment) {
		m_globalSurfaceNum += 1;
	}
}

Surface::Surface(SurfaceType type, float a, float b, unsigned int aPatch, unsigned int bPatch, unsigned int divisions) : m_divisions(divisions), m_surfaceType(type) {
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	m_globalSurfaceNum += 1;
	m_patches.reserve(aPatch * bPatch);

	if (type == SurfaceType::Flat) {
		m_aPoints = aPatch * (Patch::rowSize - 1) + 1;
		m_bPoints = bPatch * (Patch::rowSize - 1) + 1;
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

				USHORT step = Patch::rowSize - 1;
				for (USHORT u = 0; u < Patch::rowSize; ++u) {
					for (USHORT v = 0; v < Patch::rowSize; ++v) {
						indices[u * Patch::rowSize + v] = (i * step + u) * m_bPoints + (j * step + v);
					}
				}
				m_patches.emplace_back(indices);
			}
		}
	} else {
		m_aPoints = aPatch * (Patch::rowSize - 1);
		m_bPoints = bPatch * (Patch::rowSize - 1) + 1;
		m_controlPoints.reserve(m_aPoints * m_bPoints);

		for (unsigned int i = 0; i < m_aPoints; ++i) {
			for (unsigned int j = 0; j < m_bPoints; ++j) {
				auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);

				float angle = (2 * std::numbers::pi_v<float> * i) / m_aPoints;
				float x = a * cos(angle);
				float y = a * sin(angle);
				float z = (b * j) / (m_bPoints - 1) - b / 2.0f;
				point->SetTranslation(x, z, y);
				m_controlPoints.push_back(std::move(point));
			}
		}

		for (unsigned int i = 0; i < aPatch; ++i) {
			for (unsigned int j = 0; j < bPatch; ++j) {
				std::array<USHORT, Patch::patchSize> indices;

				USHORT step = Patch::rowSize - 1;
				for (USHORT u = 0; u < Patch::rowSize; ++u) {
					for (USHORT v = 0; v < Patch::rowSize; ++v) {
						USHORT wrapped_i = (i * step + u) % m_aPoints;
						indices[u * Patch::rowSize + v] = wrapped_i * m_bPoints + (j * step + v);
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

void Surface::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showNet) {
		map.at(ShaderType::Regular).Set(context);
		m_netMesh.Render(context);
	}
	map.at(ShaderType::RegularWithTesselationSurface).Set(context);
	m_surfaceMesh.Render(context);
}

void Surface::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(m_controlPoints.size());
	for (const auto& point : m_controlPoints) {
		const auto& pos = point->position();
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	if (m_showNet) {
		std::vector<USHORT> netIndices;
		unsigned int aPoints = (m_surfaceType == SurfaceType::Cylindric) ? m_aPoints : (m_aPoints - 1);
		netIndices.reserve(2 * (m_aPoints * (m_bPoints - 1) + aPoints * m_bPoints));

		// A-dim
		for (unsigned int j = 0; j < m_bPoints; ++j) {
			for (unsigned int i = 0; i < aPoints; ++i) {
				unsigned int idx = j + i * m_bPoints;
				unsigned int next_i = (i + 1) % m_aPoints;
				unsigned int nextIdx = j + next_i * m_bPoints;
				netIndices.push_back(static_cast<USHORT>(idx));
				netIndices.push_back(static_cast<USHORT>(nextIdx));
			}
		}
		// B-dim
		for (unsigned int i = 0; i < m_aPoints; ++i) {
			for (unsigned int j = 0; j < m_bPoints - 1; ++j) {
				unsigned int idx = j + i * m_bPoints;
				netIndices.push_back(static_cast<USHORT>(idx));
				netIndices.push_back(static_cast<USHORT>(idx + 1));
			}
		}

		m_netMesh.Update(device, verts, netIndices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	std::vector<USHORT> surfaceIndices;
	surfaceIndices.reserve(m_patches.size() * Patch::patchSize);
	for (const auto& patch : m_patches) {
		std::copy(patch.indices.begin(), patch.indices.end(), std::back_inserter(surfaceIndices));
	}
	m_surfaceMesh.Update(device, verts, surfaceIndices, D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

	Object::UpdateMesh(device);
}

void app::Surface::RenderProperties() {
	Object::RenderProperties();
	int divisions = m_divisions;
	ImGui::InputInt("Divisions", &divisions, 1, static_cast<int>(minDivisions));
	m_divisions = std::min(std::max(static_cast<int>(minDivisions), divisions), static_cast<int>(maxDivisions));

	ImGui::Separator();
	bool old = m_showNet;
	ImGui::Checkbox("Show net", &m_showNet);
	if (m_showNet && m_showNet != old) {
		geometryChanged = true;
	}
	ImGui::Checkbox("Hide points", &m_hidePoints);

	if (m_selectedIdx != -1) {
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Selected point")) {
			m_controlPoints[m_selectedIdx]->RenderPosition();
		}
	}

	if (ImGui::BeginTable("Control points", 1, ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		for (int i = 0; i < m_controlPoints.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (ImGui::Selectable(m_controlPoints[i]->name.c_str(), i == m_selectedIdx, ImGuiSelectableFlags_SpanAllColumns)) {
				if (m_selectedIdx == i) {
					m_selectedIdx = -1;
				} else {
					m_selectedIdx = i;
				}
			}
		}
		ImGui::EndTable();
	}
}

std::optional<std::vector<std::unique_ptr<Object>>*> Surface::GetSubObjects() {
	if (m_hidePoints) {
		return std::nullopt;
	}
	return &m_controlPoints;
}

unsigned int Surface::GetDivisions() const {
	return m_divisions;
}

gmod::vector3<double> Surface::UpdateMidpoint() {
	gmod::vector3<double> mid;
	if (!m_controlPoints.empty()) {
		for (const auto& obj : m_controlPoints) {
			const auto pos = obj->position();
			mid.x() += pos.x();
			mid.y() += pos.y();
			mid.z() += pos.z();
		}
		mid = mid * (1.0 / m_controlPoints.size());
	}
	m_midpoint = mid;
	return m_midpoint;
}

#pragma region TRANSFORM
gmod::vector3<double> Surface::position() const {
	return m_midpoint;
}
gmod::matrix4<double> Surface::modelMatrix() const {
	return gmod::matrix4<double>::identity();
}
void Surface::SetTranslation(double tx, double ty, double tz) {
	Object::SetTranslation(tx, ty, tz);
	auto diff = gmod::vector3<double>(tx, ty, tz) - m_midpoint;
	for (auto& obj : m_controlPoints) {
		obj->UpdateTranslation(diff.x(), diff.y(), diff.z());
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetRotation(double rx, double ry, double rz) {
	Object::SetRotation(rx, ry, rz);
	for (auto& obj : m_controlPoints) {
		obj->SetRotationAroundPoint(rx, ry, rz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	Object::SetRotationAroundPoint(rx, ry, rz, p);
	for (auto& obj : m_controlPoints) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetScaling(double sx, double sy, double sz) {
	Object::SetScaling(sx, sy, sz);
	for (auto& obj : m_controlPoints) {
		obj->SetScalingAroundPoint(sx, sy, sz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	Object::SetScalingAroundPoint(sx, sy, sz, p);
	for (auto& obj : m_controlPoints) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateTranslation(double dtx, double dty, double dtz) {
	Object::UpdateTranslation(dtx, dty, dtz);
	for (auto& obj : m_controlPoints) {
		obj->UpdateTranslation(dtx, dty, dtz);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	Object::UpdateRotation_Quaternion(drx, dry, drz);
	for (auto& obj : m_controlPoints) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	for (auto& obj : m_controlPoints) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateScaling(double dsx, double dsy, double dsz) {
	Object::UpdateScaling(dsx, dsy, dsz);
	for (auto& obj : m_controlPoints) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	for (auto& obj : m_controlPoints) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
#pragma endregion