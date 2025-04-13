#pragma once
#include "Object.h"
#include "PointModel.h"

class Point : public Object {
public:
	Point(PointModel* model);
	~Point() override;
	virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const override;
	virtual void AddParent(Object* obj) override;
	virtual void RemoveParent(Object* obj) override;
	virtual void InformParents() override;
private:
	static unsigned short m_globalPointNum;
	PointModel* m_model;
};