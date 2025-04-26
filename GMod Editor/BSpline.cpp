#include "BSpline.h"
#include <numeric>

using namespace app;

unsigned short BSpline::m_globalBSplineNum = 0;

BSpline::BSpline(std::vector<Object*> objects) {
	m_type = "BSpline";
	std::ostringstream os;
	os << "bspline_" << m_globalBSplineNum;
	name = os.str();
	m_globalBSplineNum += 1;

	this->objects = objects;
	for (auto& obj : this->objects) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void BSpline::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showPolyline) {
		map.at(ShaderType::Regular).Set(context);
		m_polylineMesh.Render(context);
	}

	map.at(ShaderType::RegularWithTesselationBSpline).Set(context);
	m_curveMesh.Render(context);
	
	if (showBernstein) {
		map.at(ShaderType::Regular).Set(context);
		m_bernsteinMesh.Render(context);
	}
}

void BSpline::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(objects.size());
	std::vector<USHORT> polyIdxs(objects.size());
	std::iota(polyIdxs.begin(), polyIdxs.end(), 0);

	for (const auto& obj : objects) {
		const auto& pos = obj->position();
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	m_polylineMesh.Update(device, verts, polyIdxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);

	std::vector<USHORT> curveIdxs;
	curveIdxs.reserve(objects.size());

	if (objects.size() == 2) {
		curveIdxs = { 0, 1, 1, 1 };
	} else if (objects.size() == 3) {
		curveIdxs = { 0, 1, 2, 2 };
	} else {
		for (int i = 0; i < objects.size() - 3; i += 1) {
			curveIdxs.push_back(static_cast<USHORT>(i));
			curveIdxs.push_back(static_cast<USHORT>(i + 1));
			curveIdxs.push_back(static_cast<USHORT>(i + 2));
			curveIdxs.push_back(static_cast<USHORT>(i + 3));
		}
	}
	m_curveMesh.Update(device, verts, curveIdxs, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	// TODO : calculate vertices for Bernstein polygon

}

void BSpline::RenderProperties() {
	ImGui::Checkbox("Show polyline", &m_showPolyline);
	bool old = showBernstein;
	ImGui::Checkbox("Show Bernstein polygon", &showBernstein);
	if (showBernstein && !old) {
		geometryChanged = true;
	}
	ImGui::Separator();
	Polyline::RenderProperties();
}
