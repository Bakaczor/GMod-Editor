#include "Object.h"

unsigned short Object::m_globalObjectId= 1;
unsigned short Object::m_globalObjectNum = 1;

Object::Object() : transform(), m_mesh() {
	std::ostringstream os;
	os << "object_" << m_globalObjectNum;
	name = os.str();
	m_globalObjectNum += 1;

	id = m_globalObjectId;
	m_globalObjectId += 1;
}

void Object::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_mesh.Render(context);
}