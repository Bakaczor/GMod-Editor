#pragma once
#include "framework.h"
#include <array>
#include <algorithm>
#include <sstream>
#include "../imgui/imgui.h"
#include "../gmod/Transform.h"
#include "../gmod/vector3.h"
#include "Mesh.h"

class Object {
public:
#pragma region STRUCTURES
	struct VERTEX {
		gmod::vector3<double> pos;
	};
	struct EDGE {
		USHORT v1;
		USHORT v2;
	};
	struct FACE {
		USHORT v1;
		USHORT v2;
		USHORT v3;
	};
#pragma endregion
#pragma region TRANSFORM_WRAPPER
	gmod::vector3<double> position() const;
	gmod::vector3<double> eulerAngles() const;
	gmod::vector3<double> scale() const;
	gmod::quaternion<double> rotation() const;
	gmod::vector3<double> right() const;
	gmod::vector3<double> up() const;
	gmod::vector3<double> forward() const;
	gmod::matrix4<double> modelMatrix() const;
	void UpdateRotation_Euler(double drx, double dry, double drz);

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
#pragma region PARENTS_RELATED
	virtual void AddParent(Object* obj) { return; }
	virtual void RemoveParent(Object* obj) { return; }
	virtual void InformParents() { return; }
	virtual void RemoveReferences() { return; }
#pragma endregion
	int id;
	std::string name;
	std::string type() const { return m_type; }
	std::array<float, 4> color = { 1.0f, 1.0f, 1.0f, 1.0f };
	bool geometryChanged = false;

	Object();
	virtual ~Object() = default;
	virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
	virtual void UpdateMesh(const Device& device);
	virtual void RenderProperties();
protected:
	static int m_globalObjectId;
	std::string m_type;
	Mesh m_mesh;
	gmod::Transform<double> m_transform;
	std::vector<VERTEX> m_vertices;
	std::vector<EDGE> m_edges;
	std::vector<FACE> m_faces;
	std::vector<Object*> m_parents;
private:
	static unsigned short m_globalObjectNum;
};