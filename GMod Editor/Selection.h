#pragma once
#include "Object.h"
#include "Point.h"
#include <memory>

class Selection {
public:
	std::vector<std::shared_ptr<Object>> selected;

	gmod::vector3<double> midpoint() const;
	void AddObject(std::shared_ptr<Object> obj);
	void RemoveObject(std::shared_ptr<Object> obj);
	bool Contains(int id) const;
	inline bool isPolyline() const {
		return m_numOfPoints == selected.size();
	}
private:
	int m_numOfPoints = 0;
};