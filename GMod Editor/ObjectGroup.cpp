#include "ObjectGroup.h"

using namespace app;

ObjectGroup::ObjectGroup(PointModel* model) : m_model(model) {
	m_type = "ObjectGroup";
	name = "selection";
}

ObjectGroup::~ObjectGroup() {
	Object::~Object();
	for (auto& obj : objects) {
		if (obj == nullptr) { continue; }
		obj->RemoveParent(this);
	}
}

void ObjectGroup::SetModel(PointModel* model) {
	m_model = model;
}

void ObjectGroup::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Regular).Set(context);
	m_model->Render(context);
}

void ObjectGroup::Replace(int id, Object* obj) {
	bool replacedAny = false;

	for (auto& p : objects) {
		if (p && p->id == id) {
			p->RemoveParent(this);
			p = obj;
			if (obj != nullptr) {
				obj->AddParent(this);
			}
			replacedAny = true;
		}
	}

	if (replacedAny && obj != nullptr) {
		UpdateMidpoint();
		geometryChanged = true;
	}
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

std::unordered_map<int, Object*> ObjectGroup::GetUnique() const {
	std::unordered_map<int, Object*> unique;
	for (const auto& obj : objects) {
		unique.insert(std::make_pair(obj->id, obj));
	}
	return unique;
}

void ObjectGroup::AddObject(Object* obj) {
	if (Contains(obj->id)) { return; }
	objects.push_back(obj);
	obj->AddParent(this);
	geometryChanged = true;
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
		geometryChanged = true;
		UpdateMidpoint();
		if (nullptr != dynamic_cast<Point*>(obj)) {
			m_numOfPoints -= erased;
		}
	}
}

void ObjectGroup::Clear() {
	for (auto& obj : objects) {
		obj->RemoveParent(this);
	}
	objects.clear();
	geometryChanged = true;
	UpdateMidpoint();
	m_numOfPoints = 0;
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
	Object::SetTranslation(tx, ty, tz);
	auto diff = gmod::vector3<double>(tx, ty, tz) - m_midpoint;
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateTranslation(diff.x(), diff.y(), diff.z());
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotation(double rx, double ry, double rz) {
	Object::SetRotation(rx, ry, rz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetRotationAroundPoint(rx, ry, rz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	Object::SetRotationAroundPoint(rx, ry, rz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScaling(double sx, double sy, double sz) {
	Object::SetScaling(sx, sy, sz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetScalingAroundPoint(sx, sy, sz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	Object::SetScalingAroundPoint(sx, sy, sz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateTranslation(double dtx, double dty, double dtz) {
	Object::UpdateTranslation(dtx, dty, dtz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateTranslation(dtx, dty, dtz);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	Object::UpdateRotation_Quaternion(drx, dry, drz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScaling(double dsx, double dsy, double dsz) {
	Object::UpdateScaling(dsx, dsy, dsz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
#pragma endregion