#include "framework.h"
#include "Surface.h"
#include <numbers>

using namespace app;

unsigned short Surface::m_globalSurfaceNum = 0;

const std::vector<std::pair<USHORT, USHORT>> Surface::m_borderEdges = {
	{0, 1}, {1, 2}, {2, 3},       // góra
	{3, 7}, {7, 11}, {11, 15},    // prawa
	{15, 14}, {14, 13}, {13, 12}, // dó³
	{12, 8}, {8, 4}, {4, 0}       // lewa
};

std::vector<Surface::Cycle3> Surface::FindCycles3(const std::vector<Surface*>& surfaces) {
	std::unordered_map<int, BoundaryPoint> boundaryPoints;
	for (const auto& surface : surfaces) {
		for (const auto& bp : surface->m_boundaryPoints) {
			int bpId = (*bp.thisPoint)->id;
			if (boundaryPoints.contains(bpId)) {
				for (const auto& neighbour : bp.neighbours) {
					boundaryPoints[bpId].neighbours.insert(neighbour);
				}
				boundaryPoints[bpId].isBoundary |= bp.isBoundary;
				boundaryPoints[bpId].isCorner |= bp.isCorner;
			} else {
				boundaryPoints[bpId] = bp;
			}
		}
	}

	std::unordered_set<int> criticalPoints;
	for (const auto& pair : boundaryPoints) {
		const int& id = pair.first;
		const BoundaryPoint& bp = pair.second;
		size_t degree = bp.neighbours.size();
		if (degree > 2 || bp.isCorner) {
			criticalPoints.insert(id);
		}
	}

	std::unordered_map<int, std::vector<Edge>> criticalGraph;
	for (int critical : criticalPoints) {
		const BoundaryPoint& cp = boundaryPoints[critical];
		for (Object** neighbor : cp.neighbours) {
			std::vector<Object**> path;
			Object** prev = cp.thisPoint;
			Object** current = neighbor;
			path.push_back(prev);

			while (true) {
				path.push_back(current);
				int currentId = (*current)->id;
				if (criticalPoints.contains(currentId)) {
					Edge edge;
					edge.start = boundaryPoints[critical].thisPoint;
					edge.end = current;
					edge.intermediate.assign(path.begin() + 1, path.end() - 1);
					criticalGraph[critical].push_back(edge);
					break;
				}

				const BoundaryPoint& bp = boundaryPoints[currentId];
				auto it = bp.neighbours.begin();
				int prevId = (*prev)->id;
				int nextId = (**it)->id;
				Object** next = (prevId == nextId) ? *(++it) : *it;

				prev = current;
				current = next;
			}
		}
	}

	return FindUniqueTrianglesInGraph(criticalGraph);
}

std::vector<Surface::Cycle3> Surface::FindUniqueTrianglesInGraph(const std::unordered_map<int, std::vector<Edge>>& criticalGraph) {
	std::vector<Cycle3> triangles;

	for (const auto& [idA, edgesAB] : criticalGraph) {
		// A-B edges
		for (const auto& edgeAB : edgesAB) {
			int idB = (*edgeAB.end)->id;
			if (idB <= idA) continue; // assert idA < idB

			auto itB = criticalGraph.find(idB);
			if (itB == criticalGraph.end()) continue;

			// B-C edges
			for (const auto& edgeBC : itB->second) {
				int idC = (*edgeBC.end)->id;
				if (idC <= idB || idC == idA) continue; // asset idB < idC and idC != idA

				auto itC = criticalGraph.find(idC);
				if (itC == criticalGraph.end()) continue;

				// now search C-A edges to close triangle
				for (const auto& edgeCA : itC->second) {
					int backToA = (*edgeCA.end)->id;
					if (backToA == idA) {
						Cycle3 cycle = { edgeAB, edgeBC, edgeCA };
						triangles.push_back(cycle);
					}
				}
			}
		}
	}

	return triangles;
}

Surface::Surface(bool increment) : m_divisions(Patch::rowSize), m_aPoints(0), m_bPoints(0), m_surfaceType(SurfaceType::Flat) {
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	if (increment) {
		m_globalSurfaceNum += 1;
	}
}

Surface::Surface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> controlPoints) :
	m_surfaceType(type), m_aPoints(aPoints), m_bPoints(bPoints), m_divisions(divisions) {
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	m_globalSurfaceNum += 1;

	std::copy(controlPoints.begin(), controlPoints.end(), std::back_inserter(m_controlPoints));
	
	if (m_surfaceType == SurfaceType::Flat) {
		unsigned int aPatch = (m_aPoints - 1) / (Patch::rowSize - 1);
		unsigned int bPatch = (m_bPoints - 1) / (Patch::rowSize - 1);
		for (unsigned int i = 0; i < aPatch; ++i) {
			for (unsigned int j = 0; j < bPatch; ++j) {
				std::array<USHORT, Patch::patchSize> indices;

				USHORT step = Patch::rowSize - 1;
				for (USHORT u = 0; u < Patch::rowSize; ++u) {
					for (USHORT v = 0; v < Patch::rowSize; ++v) {
						indices[u * Patch::rowSize + v] = (i * step + u) * m_bPoints + (j * step + v);
					}
				}
				m_patches.emplace_back(indices);
			}
		}
	} else {
		unsigned int aPatch = (m_aPoints) / (Patch::rowSize - 1);
		unsigned int bPatch = (m_bPoints - 1) / (Patch::rowSize - 1);
		for (unsigned int i = 0; i < aPatch; ++i) {
			for (unsigned int j = 0; j < bPatch; ++j) {
				std::array<USHORT, Patch::patchSize> indices;

				USHORT step = Patch::rowSize - 1;
				for (USHORT u = 0; u < Patch::rowSize; ++u) {
					for (USHORT v = 0; v < Patch::rowSize; ++v) {
						USHORT wrapped_i = (i * step + u) % m_aPoints;
						indices[u * Patch::rowSize + v] = wrapped_i * m_bPoints + (j * step + v);
					}
				}
				m_patches.emplace_back(indices);
			}
		}
	}

	UpdateMidpoint();
	for (auto& obj : m_controlPoints) {
		obj->AddParent(this);
	}
	geometryChanged = true;
	UpdateBoundaryPoints();
}

Surface::~Surface() {
	Object::~Object();
	for (auto& obj : m_controlPoints) {
		if (obj == nullptr) { continue; }
		obj->RemoveParent(this);
		if (obj->NumberOfParents() == 0) {
			obj->deletable = true;
		}
	}
}

void Surface::UpdateBoundaryPoints() {
	m_boundaryPoints.resize(m_controlPoints.size());

	for (size_t i = 0; i < m_controlPoints.size(); ++i) {
		m_boundaryPoints[i].thisPoint = &m_controlPoints[i];
	}

	for (const Patch& patch : m_patches) {
		for (const auto& [pia, pib] : m_borderEdges) {
			USHORT a = patch.indices[pia];
			USHORT b = patch.indices[pib];

			m_boundaryPoints[a].isBoundary = true;
			m_boundaryPoints[b].isBoundary = true;

			m_boundaryPoints[a].isCorner = isCorner(pia);
			m_boundaryPoints[b].isCorner = isCorner(pib);

			m_boundaryPoints[a].neighbours.insert(&m_controlPoints[b]);
			m_boundaryPoints[b].neighbours.insert(&m_controlPoints[a]);
		}
	}
}

bool Surface::isCorner(USHORT idx) const {
	return idx == 0 || idx == 3 || idx == 12 || idx == 15;
}

void Surface::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showNet) {
		map.at(ShaderType::Regular).Set(context);
		m_netMesh.Render(context);
	}
	map.at(ShaderType::RegularWithTesselationSurface).Set(context);
	m_surfaceMesh.Render(context);
}

void Surface::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(m_controlPoints.size());
	for (const auto& point : m_controlPoints) {
		const auto& pos = point->position();
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	if (m_showNet) {
		std::vector<USHORT> netIndices;
		unsigned int aPoints = (m_surfaceType == SurfaceType::Cylindric) ? m_aPoints : (m_aPoints - 1);
		netIndices.reserve(2 * (m_aPoints * (m_bPoints - 1) + aPoints * m_bPoints));

		// A-dim
		for (unsigned int j = 0; j < m_bPoints; ++j) {
			for (unsigned int i = 0; i < aPoints; ++i) {
				unsigned int idx = j + i * m_bPoints;
				unsigned int next_i = (i + 1) % m_aPoints;
				unsigned int nextIdx = j + next_i * m_bPoints;
				netIndices.push_back(static_cast<USHORT>(idx));
				netIndices.push_back(static_cast<USHORT>(nextIdx));
			}
		}
		// B-dim
		for (unsigned int i = 0; i < m_aPoints; ++i) {
			for (unsigned int j = 0; j < m_bPoints - 1; ++j) {
				unsigned int idx = j + i * m_bPoints;
				netIndices.push_back(static_cast<USHORT>(idx));
				netIndices.push_back(static_cast<USHORT>(idx + 1));
			}
		}

		m_netMesh.Update(device, verts, netIndices, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	std::vector<USHORT> surfaceIndices;
	surfaceIndices.reserve(m_patches.size() * Patch::patchSize);
	for (const auto& patch : m_patches) {
		std::copy(patch.indices.begin(), patch.indices.end(), std::back_inserter(surfaceIndices));
	}
	m_surfaceMesh.Update(device, verts, surfaceIndices, D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST);

	Object::UpdateMesh(device);
}

void app::Surface::RenderProperties() {
	Object::RenderProperties();
	int divisions = m_divisions;
	ImGui::InputInt("Divisions", &divisions, 1, static_cast<int>(minDivisions));
	m_divisions = std::min(std::max(static_cast<int>(minDivisions), divisions), static_cast<int>(maxDivisions));

	ImGui::Separator();
	bool old = m_showNet;
	ImGui::Checkbox("Show net", &m_showNet);
	if (m_showNet && m_showNet != old) {
		geometryChanged = true;
	}

	if (m_selectedIdx != -1) {
		ImGui::Separator();
		if (ImGui::CollapsingHeader("Selected point")) {
			m_controlPoints[m_selectedIdx]->RenderPosition();
		}
	}

	if (ImGui::BeginTable("Control points", 1, ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableHeadersRow();
		for (int i = 0; i < m_controlPoints.size(); i++) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			if (ImGui::Selectable(m_controlPoints[i]->name.c_str(), i == m_selectedIdx, ImGuiSelectableFlags_SpanAllColumns)) {
				if (m_selectedIdx == i) {
					m_selectedIdx = -1;
				} else {
					m_selectedIdx = i;
				}
			}
		}
		ImGui::EndTable();
	}
}

void Surface::Replace(int id, Object* obj) {
	auto it = std::find_if(m_controlPoints.begin(), m_controlPoints.end(),
		[id](Object* o) { return o && o->id == id; });

	if (it != m_controlPoints.end()) {
		(*it)->RemoveParent(this);
		*it = obj;
		if (obj != nullptr) {
			obj->AddParent(this);
			UpdateMidpoint();
			geometryChanged = true;
		}
	}
}

SurfaceType Surface::GetSurfaceType() const {
	return m_surfaceType;
}

unsigned int Surface::GetDivisions() const {
	return m_divisions;
}

unsigned int Surface::GetAPoints() const {
	return m_aPoints;
}

unsigned int Surface::GetBPoints() const {
	return m_bPoints;
}

const std::vector<Object*>& Surface::GetControlPoints() const {
	return m_controlPoints;
}

void Surface::ClearControlPoints() {
	m_controlPoints.clear();
}

gmod::vector3<double> Surface::UpdateMidpoint() {
	gmod::vector3<double> mid;
	if (!m_controlPoints.empty()) {
		for (const auto& obj : m_controlPoints) {
			const auto pos = obj->position();
			mid.x() += pos.x();
			mid.y() += pos.y();
			mid.z() += pos.z();
		}
		mid = mid * (1.0 / m_controlPoints.size());
	}
	m_midpoint = mid;
	return m_midpoint;
}

#pragma region TRANSFORM
gmod::vector3<double> Surface::position() const {
	return m_midpoint;
}
gmod::matrix4<double> Surface::modelMatrix() const {
	return gmod::matrix4<double>::identity();
}
void Surface::SetTranslation(double tx, double ty, double tz) {
	Object::SetTranslation(tx, ty, tz);
	auto diff = gmod::vector3<double>(tx, ty, tz) - m_midpoint;
	for (auto& obj : m_controlPoints) {
		obj->UpdateTranslation(diff.x(), diff.y(), diff.z());
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetRotation(double rx, double ry, double rz) {
	Object::SetRotation(rx, ry, rz);
	for (auto& obj : m_controlPoints) {
		obj->SetRotationAroundPoint(rx, ry, rz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetRotationAroundPoint(double rx, double ry, double rz, const gmod::vector3<double>& p) {
	Object::SetRotationAroundPoint(rx, ry, rz, p);
	for (auto& obj : m_controlPoints) {
		obj->SetRotationAroundPoint(rx, ry, rz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetScaling(double sx, double sy, double sz) {
	Object::SetScaling(sx, sy, sz);
	for (auto& obj : m_controlPoints) {
		obj->SetScalingAroundPoint(sx, sy, sz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::SetScalingAroundPoint(double sx, double sy, double sz, const gmod::vector3<double>& p) {
	Object::SetScalingAroundPoint(sx, sy, sz, p);
	for (auto& obj : m_controlPoints) {
		obj->SetScalingAroundPoint(sx, sy, sz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateTranslation(double dtx, double dty, double dtz) {
	Object::UpdateTranslation(dtx, dty, dtz);
	for (auto& obj : m_controlPoints) {
		obj->UpdateTranslation(dtx, dty, dtz);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateRotation_Quaternion(double drx, double dry, double drz) {
	Object::UpdateRotation_Quaternion(drx, dry, drz);
	for (auto& obj : m_controlPoints) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateRotationAroundPoint_Quaternion(double drx, double dry, double drz, const gmod::vector3<double>& p) {
	Object::UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
	for (auto& obj : m_controlPoints) {
		obj->UpdateRotationAroundPoint_Quaternion(drx, dry, drz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateScaling(double dsx, double dsy, double dsz) {
	Object::UpdateScaling(dsx, dsy, dsz);
	for (auto& obj : m_controlPoints) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, m_midpoint);
		obj->InformParents();
	}
	UpdateMidpoint();
}
void Surface::UpdateScalingAroundPoint(double dsx, double dsy, double dsz, const gmod::vector3<double>& p) {
	Object::UpdateScalingAroundPoint(dsx, dsy, dsz, p);
	for (auto& obj : m_controlPoints) {
		obj->UpdateScalingAroundPoint(dsx, dsy, dsz, p);
		obj->InformParents();
	}
	UpdateMidpoint();
}
#pragma endregion