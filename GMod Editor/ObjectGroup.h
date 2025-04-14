#pragma once
#include "Point.h"
#include <optional>

namespace app {
	class ObjectGroup : public Object {
	public:
		std::vector<Object*> objects;
		gmod::vector3<double> Midpoint() const;
		gmod::vector3<double> UpdateMidpoint();

		ObjectGroup(PointModel* model = nullptr);
		virtual ~ObjectGroup() override;
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const override;

		void SetModel(PointModel* model);
		void AddObject(Object* obj);
		void RemoveObject(Object* obj);
		void Clear();
		bool Contains(int id) const;
		std::optional<Object*> Single() const;
		inline bool Empty() const {
			return objects.empty();
		}
		inline bool isPolyline() const {
			return m_numOfPoints > 1 && m_numOfPoints == objects.size();
		}
#pragma region TRANSFORM
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
	private:
		const float m_modelScale = 0.5f;
		PointModel* m_model;
		gmod::vector3<double> m_midpoint;
		int m_numOfPoints = 0;
	};
}