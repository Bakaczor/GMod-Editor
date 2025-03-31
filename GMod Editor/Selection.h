#pragma once
#include "Object.h"
#include "Point.h"
#include <memory>

class Selection : public Object {
public:
	Point point;
	std::vector<Object*> selected;
	gmod::vector3<double> midpoint();

	Selection();
	void AddObject(Object* obj);
	void RemoveObject(Object* obj);
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