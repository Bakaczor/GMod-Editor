#pragma once
#include "../gmod/Transform.h"
#include "../gmod/vector3.h"
#include "Mesh.h"
#include <array>
#include <sstream>
#include <string>
#include <vector>

class Object {
public:
#pragma region STRUCTURES
	struct VERTEX {
		gmod::vector3<double> pos;
		USHORT idx;
	};
	struct EDGE {
		USHORT v1;
		USHORT v2;
	};
	struct COLOR {
		float r, g, b, a;
	};
#pragma endregion
	unsigned short id;
	std::string name;
	COLOR color = { 1.0f, 0.0f, 0.0f, 1.0f };

	gmod::Transform<double> transform;
	bool tranformChanged = false;
	bool geometryChanged = false;

	Object();

	void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const;

	virtual void UpdateMesh(const Device& device) = 0;
protected:
	static unsigned short m_globalObjectNum;
	static unsigned short m_globalObjectId;
	Mesh m_mesh;
};