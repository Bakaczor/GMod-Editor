#include "Selection.h"
#include "Selection.h"

gmod::vector3<double> Selection::midpoint() const {
	gmod::vector3<double> mid;
	for (const auto& obj : selected) {
		const auto pos = obj->transform.position();
		mid.x() += pos.x();
		mid.y() += pos.y();
		mid.z() += pos.z();
	}
	return mid * (1.0 / selected.size());
}

void Selection::AddObject(std::shared_ptr<Object> obj) {
	if (Contains(obj->id)) { return; }
	selected.push_back(obj);
	auto point = dynamic_cast<Point*>(obj.get());
	if (point != nullptr) {
		m_numOfPoints++;
	}
}

void Selection::RemoveObject(std::shared_ptr<Object> obj) {
	int erased = std::erase_if(selected, [&obj](const auto& o) {
		return o->id == obj->id;
	});
	auto point = dynamic_cast<Point*>(obj.get());
	if (erased > 0 && point != nullptr) {
		m_numOfPoints--;
	}
}

bool Selection::Contains(int id) const {
	return std::find_if(selected.begin(), selected.end(), [&id](const auto& o) { return o->id == id; }) != selected.end();
}
