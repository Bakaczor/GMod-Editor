#include "Point.h"
#include "ObjectGroup.h"

unsigned short Point::m_globalPointNum = 0;

Point::Point(PointModel* model) : m_model(model) {
	m_type = "Point";
	std::ostringstream os;
	os << "point_" << m_globalPointNum;
	name = os.str();
	m_globalPointNum += 1;
	color = { 1.0f, 0.0f, 0.0f, 1.0f };
}

Point::~Point() {
	for (auto& obj : m_parents) {
		obj->geometryChanged = true;
		auto selection = dynamic_cast<ObjectGroup*>(obj);
		if (selection != nullptr) {
			std::erase_if(selection->objects, [this](const auto& o) { return o->id == id; });
		}
	}
}

void Point::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_model->Render(context);
}

void Point::AddParent(Object* obj) {
	if (std::find_if(m_parents.begin(), m_parents.end(), [&obj](const auto& o) { return o->id == obj->id; }) != m_parents.end()) { return; }
	m_parents.push_back(obj);
}

void Point::RemoveParent(Object* obj) {
	std::erase_if(m_parents, [&obj](const auto& o) { return o->id == obj->id; });
}

void Point::InformParents() {
	for (auto& obj : m_parents) {
		obj->geometryChanged = true;
	}
}

