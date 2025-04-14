#include "Point.h"
#include "ObjectGroup.h"

using namespace app;

unsigned short Point::m_globalPointNum = 0;

Point::Point(PointModel* model) : m_model(model) {
	m_type = "Point";
	std::ostringstream os;
	os << "point_" << m_globalPointNum;
	name = os.str();
	m_globalPointNum += 1;
	color = { 1.0f, 0.0f, 0.0f, 1.0f };

	Object::SetScaling(m_modelScale, m_modelScale, m_modelScale);
}

void Point::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	m_model->Render(context);
}

