#include "Surface.h"

using namespace app;

unsigned short Surface::m_globalSurfaceNum = 0;

Surface::Surface(SurfaceType type, float a, float b, unsigned int aPatch, unsigned int bPatch, unsigned int divisions) : m_divisions(divisions) {
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	m_globalSurfaceNum += 1;

	m_patches.reserve(aPatch * bPatch);

	// CREATE CONTROL POINTS

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
	ImGui::Checkbox("Show net", &m_showNet);
	ImGui::Separator();
	Object::RenderProperties();
}

std::optional<std::vector<std::unique_ptr<Object>>*> app::Surface::GetSubObjects() {
	return std::optional<std::vector<std::unique_ptr<Object>>*>();
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