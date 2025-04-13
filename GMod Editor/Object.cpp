#include "Object.h"
#include "../imgui/misc/cpp/imgui_stdlib.h"

int Object::m_globalObjectId = 1;
unsigned short Object::m_globalObjectNum = 0;

Object::Object() : m_transform(), m_mesh(), m_type("Object") {
	std::ostringstream os;
	os << "object_" << m_globalObjectNum;
	name = os.str();
	m_globalObjectNum += 1;

	id = m_globalObjectId;
	m_globalObjectId += 1;
}

void Object::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}

void Object::RenderProperties() {
	ImGui::Spacing();
	std::string text = "Properties of " + m_type + ":";
	ImGui::Text(text.c_str());
	ImGui::InputText("Name", &name);
	ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&color));
}

#pragma region TRANSFORM_WRAPPER
gmod::vector3<double> Object::position() const { 
	return m_transform.position();
}
gmod::vector3<double> Object::eulerAngles() const {
	return m_transform.eulerAngles();
}
gmod::vector3<double> Object::scale() const {
	return m_transform.scale(); 
}
gmod::quaternion<double> Object::rotation() const {
	return m_transform.rotation();
}
gmod::vector3<double> Object::right() const {
	return m_transform.right(); 
}
gmod::vector3<double> Object::up() const { 
	return m_transform.up();
}
gmod::vector3<double> Object::forward() const {
	return m_transform.forward();
}
gmod::matrix4<double> Object::modelMatrix() const { 
	return m_transform.modelMatrix();
}
void Object::SetTranslation(double tx, double ty, double tz) {
	m_transform.SetTranslation(tx, ty, tz);
}
void Object::SetRotation(double rx, double ry, double rz) {
	m_transform.SetRotation(rx, ry, rz);
}
void Object::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	m_transform.SetRotationAroundPoint(rx, ry, rz, p);
}
void Object::SetScaling(double sx, double sy, double sz) {
	m_transform.SetScaling(sx, sy, sz);
}
void Object::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	m_transform.SetScalingAroundPoint(sx, sy, sz, p);
}
void Object::UpdateTranslation(double dtx, double dty, double dtz) {
	m_transform.UpdateTranslation(dtx, dty, dtz);
}
void Object::UpdateRotation_Euler(double drx, double dry, double drz) {
	m_transform.UpdateRotation_Euler(drx, dry, drz);
}
void Object::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	m_transform.UpdateRotation_Quaternion(drx, dry, drz);
}
void Object::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	m_transform.UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
}
void Object::UpdateScaling(double dsx, double dsy, double dsz) {
	m_transform.UpdateScaling(dsx, dsy, dsz);
}
void Object::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	m_transform.UpdateScalingAroundPoint(dsx, dsy, dsz, p);
}
#pragma endregion