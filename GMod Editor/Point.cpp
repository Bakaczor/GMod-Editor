#include "Point.h"
#include "ObjectGroup.h"

using namespace app;

unsigned short Point::m_globalPointNum = 0;

Point::Point(PointModel* model, float modelScale, bool increment) : m_model(model), m_modelScale(modelScale) {
	m_type = "Point";
	std::ostringstream os;
	os << "point_" << m_globalPointNum;
	name = os.str();
	if (increment) {
		m_globalPointNum += 1;
	}
	color = { 1.0f, 0.0f, 0.0f, 1.0f };
}

void Point::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_model == nullptr) {
		std::cerr << "[Point] : Uninitialized model.";
	} else {
		map.at(ShaderType::Regular).Set(context);
		m_model->Render(context);
	}
}

gmod::matrix4<double> Point::modelMatrix() const {
	auto pos = position();
	return gmod::matrix4<double>::translation(pos.x(), pos.y(), pos.z()) * 
		gmod::matrix4<double>::scaling(m_modelScale, m_modelScale, m_modelScale);
}

