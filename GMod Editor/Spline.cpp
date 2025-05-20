#include "Spline.h"
#include <numeric>

using namespace app;

unsigned short Spline::m_globalSplineNum = 0;

Spline::Spline(bool increment) {
	m_type = "Spline";
	std::ostringstream os;
	os << "spline_" << m_globalSplineNum;
	name = os.str();
	if (increment) {
		m_globalSplineNum += 1;
	}
}

Spline::Spline(std::vector<Object*> objects) : m_curveMesh() {
	m_type = "Spline";
	std::ostringstream os;
	os << "spline_" << m_globalSplineNum;
	name = os.str();
	m_globalSplineNum += 1;

	this->objects = objects;
	for (auto& obj : this->objects) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

void Spline::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showPolyline) {
		map.at(ShaderType::Regular).Set(context);
		m_polylineMesh.Render(context);
	}
	if (objects.size() > 1) {
		map.at(ShaderType::RegularWithTesselation).Set(context);
		m_curveMesh.Render(context);
	}
}

void Spline::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(objects.size());

	if (objects.size() > 0) {
		std::vector<USHORT> polyIdxs(objects.size());
		std::iota(polyIdxs.begin(), polyIdxs.end(), 0);

		for (const auto& obj : objects) {
			const auto& pos = obj->position();
			verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
		}

		m_polylineMesh.Update(device, verts, polyIdxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	if (objects.size() > 1) {
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

		m_curveMesh.Update(device, verts, curveIdxs, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	}
	
	Object::UpdateMesh(device);
}

void Spline::RenderProperties() {
	ImGui::Checkbox("Show polyline", &m_showPolyline);
	ImGui::Separator();
	Polyline::RenderProperties();
}
