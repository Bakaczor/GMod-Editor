#include "Application.h"
#include "Point.h"
#include "Surface.h"
#include <numbers>

using namespace app;

unsigned short Surface::m_globalSurfaceNum = 0;

Surface::Surface(SurfaceType type, float a, float b, unsigned int aPatch, unsigned int bPatch, unsigned int divisions) : m_divisions(divisions) {
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	m_globalSurfaceNum += 1;
	m_patches.reserve(aPatch * bPatch);

	if (type == SurfaceType::Flat) {
		unsigned int aPoints = aPatch * (Patch::rowSize - 1) + 1;
		unsigned int bPoints = bPatch * (Patch::rowSize - 1) + 1;
		m_controlPoints.reserve(aPoints * bPoints);

		for (unsigned int i = 0; i < aPoints; ++i) {
			for (unsigned int j = 0; j < bPoints; ++j) {
				auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.6f);

				float x = (a * i) / (aPoints - 1) - a / 2.0f;
				float z = (b * j) / (bPoints - 1) - b / 2.0f;
				point->SetTranslation(x, 0.0f, z);
				m_controlPoints.push_back(std::move(point));
			}
		}

		for (unsigned int i = 0; i < aPatch; ++i) {
			for (unsigned int j = 0; j < bPatch; ++j) {
				std::array<Object*, Patch::patchSize> patchPoints;

				unsigned int step = Patch::rowSize - 1;
				for (unsigned int u = 0; u < Patch::rowSize; ++u) {
					for (unsigned int v = 0; v < Patch::rowSize; ++v) {
						unsigned int index = (i * step + u) * bPoints + (j * step + v);
						patchPoints[u * Patch::rowSize + v] = m_controlPoints[index].get();
					}
				}

				m_patches.emplace_back();
				m_patches.back().AddRow({ patchPoints[0], patchPoints[1], patchPoints[2], patchPoints[3] });
				m_patches.back().AddRow({ patchPoints[4], patchPoints[5], patchPoints[6], patchPoints[7] });
				m_patches.back().AddRow({ patchPoints[8], patchPoints[9], patchPoints[10], patchPoints[11] });
				m_patches.back().AddRow({ patchPoints[12], patchPoints[13], patchPoints[14], patchPoints[15] });
			}
		}
	} else {
		unsigned int aPoints = aPatch * (Patch::rowSize - 1);
		unsigned int bPoints = bPatch * (Patch::rowSize - 1) + 1;
		m_controlPoints.reserve(aPoints * bPoints);

		for (unsigned int i = 0; i < aPoints; ++i) {
			for (unsigned int j = 0; j < bPoints; ++j) {
				auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.6f);

				float angle = (2 * std::numbers::pi_v<float> * i) / aPoints;
				float x = a * cos(angle);
				float y = a * sin(angle);
				float z = (b * j) / (bPoints - 1) - b / 2.0f;
				point->SetTranslation(x, y, z);
				m_controlPoints.push_back(std::move(point));
			}
		}

		for (unsigned int i = 0; i < aPatch; ++i) {
			for (unsigned int j = 0; j < bPatch; ++j) {
				std::array<Object*, Patch::patchSize> patchPoints;

				unsigned int step = Patch::rowSize - 1;
				for (unsigned int u = 0; u < Patch::rowSize; ++u) {
					for (unsigned int v = 0; v < Patch::rowSize; ++v) {
						unsigned int wrapped_i = (i * step + u) % aPoints;
						unsigned int index = wrapped_i * bPoints + (j * step + v);
						patchPoints[u * Patch::rowSize + v] = m_controlPoints[index].get();
					}
				}

				m_patches.emplace_back();
				m_patches.back().AddRow({ patchPoints[0], patchPoints[1], patchPoints[2], patchPoints[3] });
				m_patches.back().AddRow({ patchPoints[4], patchPoints[5], patchPoints[6], patchPoints[7] });
				m_patches.back().AddRow({ patchPoints[8], patchPoints[9], patchPoints[10], patchPoints[11] });
				m_patches.back().AddRow({ patchPoints[12], patchPoints[13], patchPoints[14], patchPoints[15] });
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

}

void app::Surface::UpdateMesh(const Device& device) {

}

void app::Surface::RenderProperties() {
	Object::RenderProperties();
	ImGui::Separator();
	ImGui::Checkbox("Show net", &m_showNet);

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
	return &m_controlPoints;
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