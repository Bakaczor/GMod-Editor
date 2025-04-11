#include "ObjectGroup.h"

ObjectGroup::ObjectGroup() {
	m_type = "ObjectGroup";
	name = "selection";
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
	midpoint.SetTranslation(mid.x(), mid.y(), mid.z());
	return mid;
}

void ObjectGroup::AddObject(Object* obj) {
	if (Contains(obj->id)) { return; }
	objects.push_back(obj);
	UpdateMidpoint();
	auto point = dynamic_cast<Point*>(obj);
	if (point != nullptr) {
		m_numOfPoints++;
	}
}

void ObjectGroup::RemoveObject(Object* obj) {
	int erased = std::erase_if(objects, [&obj](const auto& o) {
		return o->id == obj->id;
	});
	if (erased > 0) {
		UpdateMidpoint();
		auto point = dynamic_cast<Point*>(obj);
		if (point != nullptr) {
			m_numOfPoints--;
		}
	}
}

void ObjectGroup::Clear() {
	objects.clear();
	UpdateMidpoint();
}

bool ObjectGroup::Contains(int id) const {
	return std::find_if(objects.begin(), objects.end(), [&id](const auto& o) { return o->id == id; }) != objects.end();
}

#pragma region TRANSFORM_WRAPPER
void ObjectGroup::SetTranslation(double tx, double ty, double tz) {
	Object::SetTranslation(tx, ty, tz);
	for (auto& obj : objects) {
		obj->SetTranslation(tx, ty, tz);
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotation(double rx, double ry, double rz) {
	Object::SetRotationAroundPoint(rx, ry, rz, midpoint.position());
	for (auto& obj : objects) {
		obj->SetRotationAroundPoint(rx, ry, rz, midpoint.position());
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	Object::SetRotationAroundPoint(rx, ry, rz, p);
	for (auto& obj : objects) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScaling(double sx, double sy, double sz) {
	Object::SetScalingAroundPoint(sx, sy, sz, midpoint.position());
	for (auto& obj : objects) {
		obj->SetScalingAroundPoint(sx, sy, sz, midpoint.position());
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	Object::SetScalingAroundPoint(sx, sy, sz, p);
	for (auto& obj : objects) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateTranslation(double dtx, double dty, double dtz) {
	Object::UpdateTranslation(dtx, dty, dtz);
	for (auto& obj : objects) {
		obj->UpdateTranslation(dtx, dty, dtz);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, midpoint.position());
	for (auto& obj : objects) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, midpoint.position());
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	for (auto& obj : objects) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScaling(double dsx, double dsy, double dsz) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, midpoint.position());
	for (auto& obj : objects) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, midpoint.position());
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	for (auto& obj : objects) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	}
	UpdateMidpoint();
}
#pragma endregion