#include "ObjectGroup.h"

using namespace app;

ObjectGroup::ObjectGroup(PointModel* model) : m_model(model) {
	m_type = "ObjectGroup";
	name = "selection";
}

ObjectGroup::~ObjectGroup() {
	Object::~Object();
	for (auto& obj : objects) {
		obj->RemoveParent(this);
	}
}

void ObjectGroup::SetModel(PointModel* model) {
	m_model = model;
}

void ObjectGroup::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_model->Render(context);
}

gmod::vector3<double> ObjectGroup::Midpoint() const {
	return m_midpoint;
}

gmod::vector3<double> ObjectGroup::UpdateMidpoint() {
	gmod::vector3<double> mid;
	if (!objects.empty()) {
		for (const auto& obj : objects) {
			const auto pos = obj->position();
			mid.x() += pos.x();
			mid.y() += pos.y();
			mid.z() += pos.z();
		}
		mid = mid * (1.0 / objects.size());
	}
	m_midpoint = mid;
	return m_midpoint;
}

void ObjectGroup::AddObject(Object* obj) {
	if (Contains(obj->id)) { return; }
	objects.push_back(obj);
	obj->AddParent(this);
	UpdateMidpoint();
	if (nullptr != dynamic_cast<Point*>(obj)) {
		m_numOfPoints++;
	}
}

void ObjectGroup::RemoveObject(Object* obj) {
	int erased = std::erase_if(objects, [&obj](const auto& o) {
		return o->id == obj->id;
	});
	if (erased > 0) {
		obj->RemoveParent(this);
		UpdateMidpoint();
		if (nullptr != dynamic_cast<Point*>(obj)) {
			m_numOfPoints--;
		}
	}
}

void ObjectGroup::Clear() {
	for (auto& obj : objects) {
		obj->RemoveParent(this);
	}
	objects.clear();
	UpdateMidpoint();
}

bool ObjectGroup::Contains(int id) const {
	return std::find_if(objects.begin(), objects.end(), [&id](const auto& o) { return o->id == id; }) != objects.end();
}

std::optional<Object*> ObjectGroup::Single() const {
	if (objects.size() == 1) {
		return objects.front();
	} else {
		return std::nullopt;
	}
}
#pragma region TRANSFORM
gmod::vector3<double> ObjectGroup::position() const {
	return m_midpoint;
}
gmod::matrix4<double> ObjectGroup::modelMatrix() const {
	return gmod::matrix4<double>::translation(m_midpoint.x(), m_midpoint.y(), m_midpoint.z()) * 
		gmod::matrix4<double>::scaling(m_modelScale, m_modelScale, m_modelScale);
}
void ObjectGroup::SetTranslation(double tx, double ty, double tz) {
	for (auto& obj : objects) {
		obj->SetTranslation(tx, ty, tz);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotation(double rx, double ry, double rz) {
	for (auto& obj : objects) {
		obj->SetRotationAroundPoint(rx, ry, rz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScaling(double sx, double sy, double sz) {
	for (auto& obj : objects) {
		obj->SetScalingAroundPoint(sx, sy, sz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateTranslation(double dtx, double dty, double dtz) {
	for (auto& obj : objects) {
		obj->UpdateTranslation(dtx, dty, dtz);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	for (auto& obj : objects) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScaling(double dsx, double dsy, double dsz) {
	for (auto& obj : objects) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
#pragma endregion