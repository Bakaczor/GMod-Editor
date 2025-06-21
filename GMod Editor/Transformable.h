#pragma once
#include "Object.h"

namespace app {
	class Transformable : public Object {
	public:
		virtual ~Transformable() = default;
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {}

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
	protected:
		std::vector<Object*> m_controlPoints;
		gmod::vector3<double> UpdateMidpoint();
		std::unordered_map<int, Object*> GetUnique() const;
	private:
		gmod::vector3<double> m_midpoint;
	};
}
