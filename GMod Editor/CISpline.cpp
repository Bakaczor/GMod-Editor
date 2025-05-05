#include "CISpline.h"
#include <numeric>

using namespace app;

unsigned short CISpline::m_globalCISplineNum = 0;

CISpline::CISpline(std::vector<Object*> objects) {
	m_type = "CISpline";
	std::ostringstream os;
	os << "cispline_" << m_globalCISplineNum;
	name = os.str();
	m_globalCISplineNum += 1;

	this->objects = objects;
	for (auto& obj : this->objects) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void CISpline::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showPolyline) {
		map.at(ShaderType::Regular).Set(context);
		m_polylineMesh.Render(context);
	}
	map.at(ShaderType::RegularWithTesselationCISpline).Set(context);
	m_curveMesh.Render(context);
}

void CISpline::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> polyVerts;
	polyVerts.reserve(objects.size());
	std::vector<USHORT> polyIdxs(objects.size());
	std::iota(polyIdxs.begin(), polyIdxs.end(), 0);

	for (const auto& obj : objects) {
		const auto& pos = obj->position();
		polyVerts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	m_polylineMesh.Update(device, polyVerts, polyIdxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);


	std::vector<Vertex_PoCoef> curveVerts;
	curveVerts.reserve(objects.size());
	std::vector<USHORT> curveIdxs;
	curveIdxs.reserve(2 * (objects.size() - 1));

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
	m_curveMesh.Update(device, polyVerts, curveIdxs, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	Object::UpdateMesh(device);
}
