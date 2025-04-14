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
}

void Point::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context) const {
	if (m_model == nullptr) {
		std::cerr << "[Point] : Uninitialized model.";
	} else {
		m_model->Render(context);
	}
}

gmod::matrix4<double> app::Point::modelMatrix() const {
	auto pos = position();
	return gmod::matrix4<double>::translation(pos.x(), pos.y(), pos.z()) * 
		gmod::matrix4<double>::scaling(m_modelScale, m_modelScale, m_modelScale);
}

