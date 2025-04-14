#pragma once
#include "Object.h"
#include "PointModel.h"

namespace app {
	class Point : public Object {
	public:
		Point(PointModel* model);
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const override;
#pragma region TRANSFORM
		// disable direct rotation and scaling for points
		virtual void SetRotation(double rx, double ry, double rz) override { return; }
		virtual void SetScaling(double sx, double sy, double sz) override { return; }
		virtual void UpdateRotation_Quaternion(double drx, double dry, double drz) override { return; }
		virtual void UpdateScaling(double dsx, double dsy, double dsz) override { return; }
#pragma endregion
	private:
		static unsigned short m_globalPointNum;
		const float m_modelScale = 0.75f;
		PointModel* m_model;
	};
}