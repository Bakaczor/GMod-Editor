#include "Object.h"
#include "ObjectGroup.h"
#include "../imgui/misc/cpp/imgui_stdlib.h"

using namespace app;

int Object::m_globalObjectId = 1;
unsigned short Object::m_globalObjectNum = 0;

Object::Object() : m_transform(), m_type("Object") {
	std::ostringstream os;
	os << "object_" << m_globalObjectNum;
	name = os.str();
	m_globalObjectNum += 1;

	id = m_globalObjectId;
	m_globalObjectId += 1;
}

Object::~Object() {
	for (auto& obj : m_parents) {
		auto selection = dynamic_cast<ObjectGroup*>(obj);
		if (selection != nullptr) {
			selection->geometryChanged = true;
			std::erase_if(selection->objects, [this](const auto& o) { return o->id == id; });
		}
	}
}

void Object::RenderProperties() {
	ImGui::Spacing();
	std::string text = "Properties of " + m_type + ":";
	ImGui::Text(text.c_str());
	ImGui::InputText("Name", &name);
	ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&color));
}

#pragma region PARENTS
void Object::AddParent(Object* obj) {
	if (std::find_if(m_parents.begin(), m_parents.end(), [&obj](const auto& o) { return o->id == obj->id; }) != m_parents.end()) { return; }
	m_parents.push_back(obj);
}

void Object::RemoveParent(Object* obj) {
	std::erase_if(m_parents, [&obj](const auto& o) { return o->id == obj->id; });
}

void Object::InformParents() {
	for (auto& obj : m_parents) {
		obj->geometryChanged = true;
	}
}
#pragma endregion
#pragma region TRANSFORM
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

