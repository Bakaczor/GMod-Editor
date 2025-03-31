#include "Selection.h"

Selection::Selection() {
	m_type = "Selection";
	name = "selection";
}

gmod::vector3<double> Selection::UpdateMidpoint() {
	gmod::vector3<double> mid;
	if (!selected.empty()) {
		for (const auto& obj : selected) {
			const auto pos = obj->position();
			mid.x() += pos.x();
			mid.y() += pos.y();
			mid.z() += pos.z();
		}
		mid = mid * (1.0 / selected.size());
	}
	midpoint.SetTranslation(mid.x(), mid.y(), mid.z());
	return mid;
}

void Selection::AddObject(Object* obj) {
	if (Contains(obj->id)) { return; }
	selected.push_back(obj);
	UpdateMidpoint();
	auto point = dynamic_cast<Point*>(obj);
	if (point != nullptr) {
		m_numOfPoints++;
	}
}

void Selection::RemoveObject(Object* obj) {
	int erased = std::erase_if(selected, [&obj](const auto& o) {
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

void Selection::Clear() {
	selected.clear();
	UpdateMidpoint();
}

bool Selection::Contains(int id) const {
	return std::find_if(selected.begin(), selected.end(), [&id](const auto& o) { return o->id == id; }) != selected.end();
}

#pragma region TRANSFORM_WRAPPER
void Selection::SetTranslation(double tx, double ty, double tz) {
	Object::SetTranslation(tx, ty, tz);
	for (auto& obj : selected) {
		obj->SetTranslation(tx, ty, tz);
	}
	UpdateMidpoint();
}
void Selection::SetRotation(double rx, double ry, double rz) {
	Object::SetRotationAroundPoint(rx, ry, rz, midpoint.position());
	for (auto& obj : selected) {
		obj->SetRotationAroundPoint(rx, ry, rz, midpoint.position());
	}
	UpdateMidpoint();
}
void Selection::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	Object::SetRotationAroundPoint(rx, ry, rz, p);
	for (auto& obj : selected) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
	}
	UpdateMidpoint();
}
void Selection::SetScaling(double sx, double sy, double sz) {
	Object::SetScalingAroundPoint(sx, sy, sz, midpoint.position());
	for (auto& obj : selected) {
		obj->SetScalingAroundPoint(sx, sy, sz, midpoint.position());
	}
	UpdateMidpoint();
}
void Selection::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	Object::SetScalingAroundPoint(sx, sy, sz, p);
	for (auto& obj : selected) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
	}
	UpdateMidpoint();
}
void Selection::UpdateTranslation(double dtx, double dty, double dtz) {
	Object::UpdateTranslation(dtx, dty, dtz);
	for (auto& obj : selected) {
		obj->UpdateTranslation(dtx, dty, dtz);
	}
	UpdateMidpoint();
}
void Selection::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, midpoint.position());
	for (auto& obj : selected) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, midpoint.position());
	}
	UpdateMidpoint();
}
void Selection::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	for (auto& obj : selected) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	}
	UpdateMidpoint();
}
void Selection::UpdateScaling(double dsx, double dsy, double dsz) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, midpoint.position());
	for (auto& obj : selected) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, midpoint.position());
	}
	UpdateMidpoint();
}
void Selection::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	for (auto& obj : selected) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	}
	UpdateMidpoint();
}
#pragma endregion