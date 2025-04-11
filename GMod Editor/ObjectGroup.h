#pragma once
#include "Point.h"

class ObjectGroup : public Object {
public:
	Point midpoint;
	std::vector<Object*> objects;
	gmod::vector3<double> UpdateMidpoint();

	ObjectGroup();
	void AddObject(Object* obj);
	void RemoveObject(Object* obj);
	void Clear();
	bool Contains(int id) const;
#pragma region TRANSFORM_WRAPPER
	virtual void SetTranslation(double tx, double ty, double tz) override;
	virtual void SetRotation(double rx, double ry, double rz) override;
	virtual void SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) override;
	virtual void SetScaling(double sx, double sy, double sz) override;
	virtual void SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) override;
	virtual void UpdateTranslation(double dtx, double dty, double dtz) override;
	virtual void UpdateRotation_Quaternion(double drx, double dry, double drz) override;
	virtual void UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) override;
	virtual void UpdateScaling(double dsx, double dsy, double dsz) override;
	virtual void UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) override;
#pragma endregion

	inline bool isPolyline() const {
		return m_numOfPoints > 1 && m_numOfPoints == objects.size();
	}
	inline bool empty() const {
		return objects.empty();
	}
private:
	int m_numOfPoints = 0;
};