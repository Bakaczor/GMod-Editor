#include "Application.h"
#include "BSpline.h"
#include <numeric>

using namespace app;

unsigned short BSpline::m_globalBSplineNum = 0;

gmod::matrix4<double> BSpline::Mbs = (1.0 / 6.0) * gmod::matrix4<double>(
	1, 4, 1, 0,
	0, 4, 2, 0,
	0, 2, 4, 0,
	0, 1, 4, 1
);

gmod::matrix4<double> BSpline::Mbs_inv = gmod::matrix4<double>(
	6, -7,  2, 0,
	0,  2, -1, 0,
	0, -1,  2, 0,
	0,  2, -7, 6
);

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
	
	if (showBernstein && !bernsteinPoints.empty()) {
		map.at(ShaderType::Regular).Set(context);
		m_bernsteinMesh.Render(context);
	}
}

void BSpline::UpdateMesh(const Device& device) {
	if (showBernstein) {
		bool senderIsBernstein = false;
		if (nullptr != m_sender) {
			int senderId = m_sender->id;
			senderIsBernstein = std::find_if(bernsteinPoints.begin(), bernsteinPoints.end(), [&senderId](const auto& o) { return o->id == senderId; }) != bernsteinPoints.end();
		}
		
		if (senderIsBernstein) {
			RecalculateBernstein(m_sender);
			RecalculateDeBoore();
		} else {
			RecalculateBernstein(nullptr);
		}
	}

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

	if (objects.size() == 2) {
		curveIdxs = { 0, 0, 1, 1 };
	} else if (objects.size() == 3) {
		curveIdxs = { 0, 0, 1, 2, 0, 1, 2, 2 };
	} else {
		for (int i = 0; i < objects.size() - 3; ++i) {
			curveIdxs.push_back(static_cast<USHORT>(i));
			curveIdxs.push_back(static_cast<USHORT>(i + 1));
			curveIdxs.push_back(static_cast<USHORT>(i + 2));
			curveIdxs.push_back(static_cast<USHORT>(i + 3));
		}
	}
	if (objects.size() > 1) {
		m_curveMesh.Update(device, verts, curveIdxs, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
	}

	if (showBernstein && !bernsteinPoints.empty()) {
		std::vector<Vertex_Po> bernVerts;
		bernVerts.reserve(bernsteinPoints.size());
		std::vector<USHORT> bernIdxs(bernsteinPoints.size());
		std::iota(bernIdxs.begin(), bernIdxs.end(), 0);

		for (const auto& obj : bernsteinPoints) {
			const auto& pos = obj->position();
			bernVerts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
		}
		m_bernsteinMesh.Update(device, bernVerts, bernIdxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	}

	Object::UpdateMesh(device);
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

std::optional <std::vector<std::unique_ptr<Object>>*> app::BSpline::GetSubObjects() {
	if (showBernstein) {
		return &bernsteinPoints;
	} else {
		return std::nullopt;
	}
}

void BSpline::RecalculateDeBoore() {
	// recalculate de Boore points, after Bernstein polygon was changed
	for (int i = 0, j = 0; i < bernsteinPoints.size() - 3; i += 3, ++j) {
		auto b0 = bernsteinPoints[i]->position();
		auto b1 = bernsteinPoints[i + 1]->position();
		auto b2 = bernsteinPoints[i + 2]->position();
		auto b3 = bernsteinPoints[i + 3]->position();

		if (i == 0) {
			auto p0 = b0 * Mbs_inv(0, 0) + b1 * Mbs_inv(0, 1) + b2 * Mbs_inv(0, 2) + b3 * Mbs_inv(0, 3);
			auto p1 = b0 * Mbs_inv(1, 0) + b1 * Mbs_inv(1, 1) + b2 * Mbs_inv(1, 2) + b3 * Mbs_inv(1, 3);
			auto p2 = b0 * Mbs_inv(2, 0) + b1 * Mbs_inv(2, 1) + b2 * Mbs_inv(2, 2) + b3 * Mbs_inv(2, 3);
			if (j < objects.size()) {
				objects[j]->SetTranslation(p0.x(), p0.y(), p0.z());
			}
			if (j + 1 < objects.size()) {
				objects[j + 1]->SetTranslation(p1.x(), p1.y(), p1.z());
			}
			if (j + 2 < objects.size()) {
				objects[j + 2]->SetTranslation(p2.x(), p2.y(), p2.z());
			}

		}
		auto p3 = b0 * Mbs_inv(3, 0) + b1 * Mbs_inv(3, 1) + b2 * Mbs_inv(3, 2) + b3 * Mbs_inv(3, 3);
		if (j + 3 < objects.size()) {
			objects[j + 3]->SetTranslation(p3.x(), p3.y(), p3.z());
		}
	}
}

void BSpline::RecalculateBernstein(Object* sender) {
	if (sender) {
		// recalculate Bernstein polygons, after Bernstein polygon was changed
		if (bernsteinPoints.size() < 4) { return; }

		auto it = std::find_if(bernsteinPoints.begin(), bernsteinPoints.end(), [sender](const auto& p) { return p.get() == sender; });
		if (it == bernsteinPoints.end()) return;

		const int globalIdx = std::distance(bernsteinPoints.begin(), it);
		const int localIdx = globalIdx % 3;

		int leftStartIdx = -1;
		int leftStartRule = -1;
		int rightStartIdx = -1;
		int rightStartRule = -1;

		switch (localIdx) {
			case 0: {
				if (globalIdx == 0 || globalIdx == bernsteinPoints.size() - 1) {
					return;
				}
				auto vec = (bernsteinPoints[globalIdx + 1]->position() - bernsteinPoints[globalIdx - 1]->position()) * 0.5;
				auto next = bernsteinPoints[globalIdx]->position() + vec;
				auto prev = bernsteinPoints[globalIdx]->position() - vec;
				bernsteinPoints[globalIdx + 1]->SetTranslation(next.x(), next.y(), next.z());
				bernsteinPoints[globalIdx - 1]->SetTranslation(prev.x(), prev.y(), prev.z());
				leftStartIdx = globalIdx - 5;
				rightStartIdx = globalIdx + 5;
				leftStartRule = 2;
				rightStartRule = 2;
				break;
			}
			case 1: {
				leftStartIdx = globalIdx - 2;
				rightStartIdx = globalIdx + 4;
				leftStartRule = 1;
				rightStartRule = 2;
				break;
			}
			case 2: {
				leftStartIdx = globalIdx - 4;
				rightStartIdx = globalIdx + 2;
				leftStartRule = 2;
				rightStartRule = 1;
				break;
			}
		}

		// left propagation (leftStart to begin)
		for (int i = leftStartIdx, r = leftStartRule; i > 0; --i) {
			if (r == 1) {
				auto vec = bernsteinPoints[i + 1]->position() - bernsteinPoints[i + 2]->position();
				auto prev = bernsteinPoints[i + 1]->position() + vec;
				bernsteinPoints[i]->SetTranslation(prev.x(), prev.y(), prev.z());
			} else if (r == 2) {
				auto vec = bernsteinPoints[i + 2]->position() - bernsteinPoints[i + 3]->position();
				auto prev = bernsteinPoints[i + 4]->position() + 4 * vec;
				bernsteinPoints[i]->SetTranslation(prev.x(), prev.y(), prev.z());
			}

			// switch to next rule
			r = (r + 1) % 3;
		}

		// right propagation (rightStart to end)
		for (int i = rightStartIdx, r = rightStartRule; i < bernsteinPoints.size(); ++i) {
			if (r == 1) {
				auto vec = bernsteinPoints[i - 1]->position() - bernsteinPoints[i - 2]->position();
				auto next = bernsteinPoints[i - 1]->position() + vec;
				bernsteinPoints[i]->SetTranslation(next.x(), next.y(), next.z());
			} else if (r == 2) {
				auto vec = bernsteinPoints[i - 2]->position() - bernsteinPoints[i - 3]->position();
				auto next = bernsteinPoints[i - 4]->position() + 4 * vec;
				bernsteinPoints[i]->SetTranslation(next.x(), next.y(), next.z());
			}

			// switch to next rule
			r = (r + 1) % 3;
		}
	} else {
		// recalculate Bernstein polygons, after de Boore point was changed
		bernsteinPoints.clear();

		for (int i = 0; i < objects.size() - 3; ++i) {
			auto f0 = objects[i]->position();
			auto f1 = objects[i + 1]->position();
			auto f2 = objects[i + 2]->position();
			auto f3 = objects[i + 3]->position();

			auto b1 = f0 * Mbs(1, 0) + f1 * Mbs(1, 1) + f2 * Mbs(1, 2) + f3 * Mbs(1, 3);
			auto b2 = f0 * Mbs(2, 0) + f1 * Mbs(2, 1) + f2 * Mbs(2, 2) + f3 * Mbs(2, 3);
			auto b3 = f0 * Mbs(3, 0) + f1 * Mbs(3, 1) + f2 * Mbs(3, 2) + f3 * Mbs(3, 3);

			if (i == 0) {
				// first segment includes all points
				auto b0 = f0 * Mbs(0, 0) + f1 * Mbs(0, 1) + f2 * Mbs(0, 2) + f3 * Mbs(0, 3);
				bernsteinPoints.push_back(std::make_unique<Point>(Application::m_pointModel.get(), 0.6f, false));
				bernsteinPoints.back()->SetTranslation(b0.x(), b0.y(), b0.z());
				bernsteinPoints.back()->color = { 0.0f, 1.0f, 0.0f, 1.0f };
				bernsteinPoints.back()->AddParent(this);
			} 
			// for the next segements b0 == bernsteinPoints.back()
			bernsteinPoints.push_back(std::make_unique<Point>(Application::m_pointModel.get(), 0.6f, false));
			bernsteinPoints.back()->SetTranslation(b1.x(), b1.y(), b1.z());
			bernsteinPoints.back()->color = { 0.0f, 1.0f, 0.0f, 1.0f };
			bernsteinPoints.back()->AddParent(this);
			bernsteinPoints.push_back(std::make_unique<Point>(Application::m_pointModel.get(), 0.6f, false));
			bernsteinPoints.back()->SetTranslation(b2.x(), b2.y(), b2.z());
			bernsteinPoints.back()->color = { 0.0f, 1.0f, 0.0f, 1.0f };
			bernsteinPoints.back()->AddParent(this);
			bernsteinPoints.push_back(std::make_unique<Point>(Application::m_pointModel.get(), 0.6f, false));
			bernsteinPoints.back()->SetTranslation(b3.x(), b3.y(), b3.z());
			bernsteinPoints.back()->color = { 0.0f, 1.0f, 0.0f, 1.0f };
			bernsteinPoints.back()->AddParent(this);
		}
	}
}
