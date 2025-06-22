#include "Transformable.h"

using namespace app;

gmod::vector3<double> Transformable ::UpdateMidpoint() {
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
std::unordered_map<int, Object*> Transformable::GetUnique() const {
	std::unordered_map<int, Object*> unique;
	for (const auto& obj : m_controlPoints) {
		unique.insert(std::make_pair(obj->id, obj));
	}
	return unique;
}

gmod::vector3<double> Transformable::position() const {
	return m_midpoint;
}
gmod::matrix4<double> Transformable::modelMatrix() const {
	return gmod::matrix4<double>::identity();
}
void Transformable::SetTranslation(double tx, double ty, double tz) {
	Object::SetTranslation(tx, ty, tz);
	auto diff = gmod::vector3<double>(tx, ty, tz) - m_midpoint;
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateTranslation(diff.x(), diff.y(), diff.z());
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::SetRotation(double rx, double ry, double rz) {
	Object::SetRotation(rx, ry, rz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetRotationAroundPoint(rx, ry, rz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	Object::SetRotationAroundPoint(rx, ry, rz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::SetScaling(double sx, double sy, double sz) {
	Object::SetScaling(sx, sy, sz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetScalingAroundPoint(sx, sy, sz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	Object::SetScalingAroundPoint(sx, sy, sz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::UpdateTranslation(double dtx, double dty, double dtz) {
	Object::UpdateTranslation(dtx, dty, dtz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateTranslation(dtx, dty, dtz);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	Object::UpdateRotation_Quaternion(drx, dry, drz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::UpdateScaling(double dsx, double dsy, double dsz) {
	Object::UpdateScaling(dsx, dsy, dsz);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Transformable::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	auto unique = GetUnique();
	for (auto& [id, obj] : unique) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}