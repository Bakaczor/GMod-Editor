#include "ObjectGroup.h"

ObjectGroup::ObjectGroup(PointModel* model) : m_model(model) {
	m_type = "ObjectGroup";
	name = "selection";
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
gmod::vector3<double> ObjectGroup::position() const {
	return m_midpoint;
}
gmod::matrix4<double> ObjectGroup::modelMatrix() const {
	return gmod::matrix4<double>::translation(m_midpoint.x(), m_midpoint.y(), m_midpoint.z()) * 
		gmod::matrix4<double>::scaling(m_midpointScale, m_midpointScale, m_midpointScale);
}
void ObjectGroup::SetTranslation(double tx, double ty, double tz) {
	for (auto& obj : objects) {
		obj->SetTranslation(tx, ty, tz);
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotation(double rx, double ry, double rz) {
	for (auto& obj : objects) {
		obj->SetRotationAroundPoint(rx, ry, rz, m_midpoint);
	}
	UpdateMidpoint();
}
void ObjectGroup::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScaling(double sx, double sy, double sz) {
	for (auto& obj : objects) {
		obj->SetScalingAroundPoint(sx, sy, sz, m_midpoint);
	}
	UpdateMidpoint();
}
void ObjectGroup::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateTranslation(double dtx, double dty, double dtz) {
	for (auto& obj : objects) {
		obj->UpdateTranslation(dtx, dty, dtz);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	for (auto& obj : objects) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, m_midpoint);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScaling(double dsx, double dsy, double dsz) {
	for (auto& obj : objects) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, m_midpoint);
	}
	UpdateMidpoint();
}
void ObjectGroup::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	for (auto& obj : objects) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	}
	UpdateMidpoint();
}
#pragma endregion