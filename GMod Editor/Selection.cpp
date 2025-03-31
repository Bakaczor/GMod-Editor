#include "Selection.h"

Selection::Selection() {
	m_type = "Selection";
	name = "selection";
}

gmod::vector3<double> Selection::midpoint() {
	gmod::vector3<double> mid;
	for (const auto& obj : selected) {
		const auto pos = obj->transform.position();
		mid.x() += pos.x();
		mid.y() += pos.y();
		mid.z() += pos.z();
	}
	mid = mid * (1.0 / selected.size());
	point.transform.SetTranslation(mid.x(), mid.y(), mid.z());
	return mid;
}

void Selection::AddObject(Object* obj) {
	if (Contains(obj->id)) { return; }
	selected.push_back(obj);
	midpoint();
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
		midpoint();
		auto point = dynamic_cast<Point*>(obj);
		if (point != nullptr) {
			m_numOfPoints--;
		}
	}
}

void Selection::Clear() {
	selected.clear();
	midpoint();
}

bool Selection::Contains(int id) const {
	return std::find_if(selected.begin(), selected.end(), [&id](const auto& o) { return o->id == id; }) != selected.end();
}
