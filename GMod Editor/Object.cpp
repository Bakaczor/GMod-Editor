#include "Object.h"
#include "ObjectGroup.h"
#include "Surface.h"
#include <imgui_stdlib.h>

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
	std::vector<Object*> parentsCopy = m_parents;
	for (auto& obj : parentsCopy) {
		auto selection = dynamic_cast<ObjectGroup*>(obj);
		if (selection != nullptr) {
			selection->geometryChanged = true;
			std::erase_if(selection->objects, [this](const auto& o) { return o->id == id; });
		} else {
			auto surface = dynamic_cast<Surface*>(obj);
			if (surface != nullptr) {
				surface->Replace(this->id, nullptr);
			}
		}
	}
}

void Object::UpdateMesh(const Device& device) {
	geometryChanged = false;
	m_sender = nullptr;
}

void Object::RenderProperties() {
	ImGui::Spacing();
	std::string text = "Properties of " + m_type + ":";
	ImGui::Text(text.c_str());
	ImGui::InputText("Name", &name);
	ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&color));
}

void Object::RenderPosition(float step, float stepFast) {
	bool flag = true;
	ImGui::Text("Position");
	gmod::vector3<double> position = this->position();
	flag = false;
	if (ImGui::InputDouble("X##Position", &position.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		flag = true;
	}
	if (ImGui::InputDouble("Y##Position", &position.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		flag = true;
	}
	if (ImGui::InputDouble("Z##Position", &position.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		flag = true;
	}

	if (flag) {
		this->SetTranslation(position.x(), position.y(), position.z());
		this->InformParents();
	}
}

std::optional<std::vector<std::unique_ptr<Object>>*> app::Object::GetSubObjects() {
	return std::nullopt;
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
		obj->m_sender = this;
	}
}

unsigned int Object::NumberOfParents() const {
	return m_parents.size();
}

void Object::ReplaceSelf(Object* obj) {
	std::vector<Object*> parentsCopy = m_parents;
	for (auto& parent : parentsCopy) {
		if (parent->type() == "ObjectGroup") {
			// skip for non-scene ObjectGroup objects
			continue; 
		}
		parent->Replace(this->id, obj);
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

