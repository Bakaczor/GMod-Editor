#pragma once
#include "Object.h"
#include "Point.h"
#include <memory>

class Selection {
public:
	Point point;
	std::vector<std::shared_ptr<Object>> selected;
	gmod::vector3<double> midpoint();

	void AddObject(std::shared_ptr<Object> obj);
	void RemoveObject(std::shared_ptr<Object> obj);
	void Clear();
	bool Contains(int id) const;

	inline bool isPolyline() const {
		return m_numOfPoints == selected.size();
	}
	inline bool empty() const {
		return selected.empty();
	}
private:
	int m_numOfPoints = 0;
};