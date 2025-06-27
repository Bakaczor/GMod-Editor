#pragma once
#include "../gmod/Transform.h"
#include "../gmod/vector3.h"
#include <imgui.h>
#include "framework.h"
#include "Mesh.h"
#include "Shaders.h"
#include <algorithm>
#include <array>
#include <optional>
#include <sstream>
#include <unordered_map>

namespace app {
	class Object {
	public:
		int id;
		std::string name;
		std::string type() const { return m_type; }
		std::array<float, 4> color = { 1.0f, 1.0f, 1.0f, 1.0f };
		bool geometryChanged = false;
		bool deletable = true;
		bool intersectable = false;

		Object();
		virtual ~Object();
		virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const = 0;
		virtual void UpdateMesh(const Device& device);
		virtual void RenderProperties();
		virtual std::optional<std::vector<std::unique_ptr<Object>>*> GetSubObjects();
		void RenderPosition(float step = 0.001f, float stepFast = 0.1f);

		static void FixGlobalObjectId(const std::vector<std::unique_ptr<Object>>& sceneObjects);
#pragma region PARENTS
		virtual void AddParent(Object* obj);
		virtual void RemoveParent(Object* obj);
		virtual void InformParents();
		virtual unsigned int NumberOfParents() const;
		virtual void Replace(int id, Object* obj) { /*currently only for ObjectGroup*/ }
		virtual void ReplaceSelf(Object* obj);
#pragma endregion
#pragma region TRANSFORM
		virtual gmod::vector3<double> position() const;
		virtual gmod::vector3<double> eulerAngles() const;
		virtual gmod::vector3<double> scale() const;
		virtual gmod::quaternion<double> rotation() const;
		virtual gmod::vector3<double> right() const;
		virtual gmod::vector3<double> up() const;
		virtual gmod::vector3<double> forward() const;
		virtual gmod::matrix4<double> modelMatrix() const;

		virtual void SetTranslation(double tx, double ty, double tz);
		virtual void SetRotation(double rx, double ry, double rz);
		virtual void SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p);
		virtual void SetScaling(double sx, double sy, double sz);
		virtual void SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p);
		virtual void UpdateTranslation(double dtx, double dty, double dtz);
		virtual void UpdateRotation_Quaternion(double drx, double dry, double drz);
		virtual void UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p);
		virtual void UpdateScaling(double dsx, double dsy, double dsz);
		virtual void UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p);
#pragma endregion
	protected:
		static int m_globalObjectId;
		std::string m_type;
		std::vector<Object*> m_parents;
		Object* m_sender = nullptr;
	private:
		static unsigned short m_globalObjectNum;
		gmod::Transform<double> m_transform;
	};
}