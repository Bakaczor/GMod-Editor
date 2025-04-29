#include "Curve.h"
#include <numeric>

using namespace app;

unsigned short Curve::m_globalCurveNum = 0;

Curve::Curve(bool increment) {
	m_type = "Curve";
	std::ostringstream os;
	os << "curve_" << m_globalCurveNum;
	name = os.str();
	if (increment) {
		m_globalCurveNum += 1;
	}
}

Curve::Curve(std::vector<Object*> objects) : m_curveMesh() {
	m_type = "Curve";
	std::ostringstream os;
	os << "curve_" << m_globalCurveNum;
	name = os.str();
	m_globalCurveNum += 1;

	this->objects = objects;
	for (auto& obj : this->objects) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void app::Curve::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showPolyline) {
		map.at(ShaderType::Regular).Set(context);
		m_polylineMesh.Render(context);
	}
	if (objects.size() > 1) {
		map.at(ShaderType::RegularWithTesselation).Set(context);
		m_curveMesh.Render(context);
	}
}

void app::Curve::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(objects.size());
	std::vector<USHORT> polyIdxs(objects.size());
	std::iota(polyIdxs.begin(), polyIdxs.end(), 0);

	for (const auto& obj : objects) {
		const auto& pos = obj->position();
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}
	if (objects.size() > 0) {
		m_polylineMesh.Update(device, verts, polyIdxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	std::vector<USHORT> curveIdxs;
	curveIdxs.reserve(objects.size());

	for (int i = 0; i < objects.size(); ) {
		curveIdxs.push_back(static_cast<USHORT>(i));
		// adjust for full 4-point patches
		if (i + 1 >= objects.size()) {
			// remove the patch if it consists only from one point
			curveIdxs.erase(curveIdxs.end() - 1);
			break;
		} else if (i + 2 >= objects.size()) {
			curveIdxs.push_back(static_cast<USHORT>(i + 1));
			curveIdxs.push_back(curveIdxs.back());
			curveIdxs.push_back(curveIdxs.back());
			break;
		} else if (i + 3 >= objects.size()) {
			curveIdxs.push_back(static_cast<USHORT>(i + 1));
			curveIdxs.push_back(static_cast<USHORT>(i + 2));
			curveIdxs.push_back(curveIdxs.back());
			break;
		} else {
			curveIdxs.push_back(static_cast<USHORT>(i + 1));
			curveIdxs.push_back(static_cast<USHORT>(i + 2));
			curveIdxs.push_back(static_cast<USHORT>(i + 3));
			// next segment starts at current end point
			i += 3;
		}
	}
	if (objects.size() > 1) {
		m_curveMesh.Update(device, verts, curveIdxs, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	}
	
	Object::UpdateMesh(device);
}

void app::Curve::RenderProperties() {
	ImGui::Checkbox("Show polyline", &m_showPolyline);
	ImGui::Separator();
	Polyline::RenderProperties();
}
