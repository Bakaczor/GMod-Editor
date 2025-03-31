#pragma once
#include "Point.h"

class Selection : public Object {
public:
	Point midpoint;
	std::vector<Object*> selected;
	gmod::vector3<double> UpdateMidpoint();

	Selection();
	void AddObject(Object* obj);
	void RemoveObject(Object* obj);
	void Clear();
	bool Contains(int id) const;

	inline bool isPolyline() const {
		return m_numOfPoints > 1 && m_numOfPoints == selected.size();
	}
	inline bool empty() const {
		return selected.empty();
	}
private:
	int m_numOfPoints = 0;
};