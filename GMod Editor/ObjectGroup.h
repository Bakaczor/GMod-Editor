#pragma once
#include "Point.h"

class ObjectGroup : public Object {
public:
	std::vector<Object*> objects;
	gmod::vector3<double> Midpoint() const;
	gmod::vector3<double> UpdateMidpoint();

	ObjectGroup(PointModel* model = nullptr);
	virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const override;

	void SetModel(PointModel* model);
	void AddObject(Object* obj);
	void RemoveObject(Object* obj);
	void Clear();
	bool Contains(int id) const;
	inline bool Empty() const {
		return objects.empty();
	}
#pragma region TRANSFORM_WRAPPER
	virtual gmod::vector3<double> position() const override;
	virtual gmod::matrix4<double> modelMatrix() const override;
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
	inline bool IsPolyline() const {
		return m_numOfPoints > 1 && m_numOfPoints == objects.size();
	}
private:
	const float m_midpointScale = 0.75f;
	PointModel* m_model;
	gmod::vector3<double> m_midpoint;
	int m_numOfPoints = 0;
};