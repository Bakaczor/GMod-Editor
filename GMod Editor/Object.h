#pragma once
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include "../gmod/Transform.h"
#include "../gmod/vector3.h"
#include "Mesh.h"

class Object {
public:
	unsigned short id;
	std::string name;
	std::array<float, 4> color = { 1.0f, 0.0f, 0.0f, 1.0f };

	gmod::Transform<double> transform;
	bool tranformChanged = false;
	bool geometryChanged = false;

	Object();

	void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const;

	virtual void UpdateMesh(const Device& device) = 0;
#pragma region STRUCTURES
	struct VERTEX {
		gmod::vector3<double> pos;
		USHORT idx;
	};
	struct EDGE {
		USHORT v1;
		USHORT v2;
	};
#pragma endregion
protected:
	static unsigned short m_globalObjectNum;
	static unsigned short m_globalObjectId;
	Mesh m_mesh;
};