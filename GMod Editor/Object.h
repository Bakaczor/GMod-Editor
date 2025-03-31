#pragma once
#include "pch.h"
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
	int id;
	std::string name;
	std::string type() const { return m_type; }
	std::array<float, 4> color = { 1.0f, 1.0f, 1.0f, 1.0f };

	gmod::Transform<double> transform;
	bool geometryChanged = false;

	Object();
	virtual ~Object() = default;
	virtual void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const;
	virtual void UpdateMesh(const Device& device);
	virtual void RenderProperties();

	virtual void AddParent(Object* obj) { return; }
	virtual void RemoveParent(Object* obj) { return; }
	virtual void InformParents() { return; }
	virtual void RemoveReferences() { return; }
protected:
	static int m_globalObjectId;
	std::string m_type;
	Mesh m_mesh;
	std::vector<VERTEX> m_vertices;
	std::vector<EDGE> m_edges;
	std::vector<FACE> m_faces;
	std::vector<Object*> m_parents;
private:
	static unsigned short m_globalObjectNum;
};