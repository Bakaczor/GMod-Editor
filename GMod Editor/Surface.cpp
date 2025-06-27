#include "framework.h"
#include "Surface.h"
#include <numbers>
#include "BSurface.h"

using namespace app;

unsigned short Surface::m_globalSurfaceNum = 0;

#pragma region TOPOLOGY
const std::vector<std::pair<USHORT, USHORT>> Surface::m_borderEdges = {
	{0, 1}, {1, 2}, {2, 3},       // góra
	{3, 7}, {7, 11}, {11, 15},    // prawa
	{15, 14}, {14, 13}, {13, 12}, // dó³
	{12, 8}, {8, 4}, {4, 0}       // lewa
};

void Surface::InitializeBoundaryPoints() {
	m_boundaryPoints.resize(m_controlPoints.size());

	for (size_t i = 0; i < m_controlPoints.size(); ++i) {
		m_boundaryPoints[i].data.idx = i;
		m_boundaryPoints[i].thisPoint = &m_controlPoints[i];
	}

	for (size_t i = 0; i < m_patches.size(); ++i) {
		for (const auto& [pia, pib] : m_borderEdges) {
			USHORT a = m_patches[i].indices[pia];
			USHORT b = m_patches[i].indices[pib];

			m_boundaryPoints[a].data.surfIdPatchIdx[this->id].insert(i);
			m_boundaryPoints[b].data.surfIdPatchIdx[this->id].insert(i);

			m_boundaryPoints[a].isPatchBoundary = true;
			m_boundaryPoints[b].isPatchBoundary = true;

			m_boundaryPoints[a].isPatchCorner = isPatchCorner(pia);
			m_boundaryPoints[b].isPatchCorner = isPatchCorner(pib);

			m_boundaryPoints[a].neighbours[&m_controlPoints[b]].idx = b;
			m_boundaryPoints[a].neighbours[&m_controlPoints[b]].surfIdPatchIdx[this->id].insert(i);

			m_boundaryPoints[b].neighbours[&m_controlPoints[a]].idx = a;
			m_boundaryPoints[b].neighbours[&m_controlPoints[a]].surfIdPatchIdx[this->id].insert(i);
		}
	}
}

bool Surface::isPatchCorner(USHORT idx) const {
	return idx == 0 || idx == 3 || idx == 12 || idx == 15;
}

void Surface::DetectSurfaceBoundaryPoints() {
	m_boundaryPoints[0].isSurfaceCorner = true;
	for (int i = 0; i < m_bPoints; ++i) {
		int idx = i;
		m_boundaryPoints[idx].isSurfaceBoundary = true;
	}
	if (m_surfaceType == SurfaceType::Flat) {
		m_boundaryPoints[m_bPoints - 1].isSurfaceCorner = true;
		for (int i = 0; i < m_aPoints; ++i) {
			int idx = m_bPoints * i + (m_bPoints - 1);
			m_boundaryPoints[idx].isSurfaceBoundary = true;
		}

		m_boundaryPoints[(m_aPoints - 1) * m_bPoints].isSurfaceCorner = true;
		for (int i = 0; i < m_aPoints; ++i) {
			int idx = m_bPoints * i;
			m_boundaryPoints[idx].isSurfaceBoundary = true;
		}

		m_boundaryPoints[m_aPoints * m_bPoints - 1].isSurfaceCorner = true;
		for (int i = 0; i < m_bPoints; ++i) {
			int idx = i + (m_aPoints - 1) * m_bPoints;
			m_boundaryPoints[idx].isSurfaceBoundary = true;
		}
	} else {
		m_boundaryPoints[m_bPoints - 1].isSurfaceCorner = true;
		for (int i = 0; i < m_aPoints; ++i) {
			int idx = m_bPoints * i;
			m_boundaryPoints[idx].isSurfaceBoundary = true;
		}
		for (int i = 0; i < m_aPoints; ++i) {
			int idx = m_bPoints * i + (m_bPoints - 1);
			m_boundaryPoints[idx].isSurfaceBoundary = true;
		}
	}
}

std::unordered_map<int, Surface::BoundaryPoint> Surface::CombineBoundaryPoints(const std::vector<Surface*>& surfaces, bool includePatchBoundaries) {
	std::unordered_map<int, BoundaryPoint> boundaryPoints;
	for (const auto& surface : surfaces) {
		for (const auto& bp : surface->m_boundaryPoints) {
			int bpId = (*bp.thisPoint)->id;

			bool isBoundary = includePatchBoundaries ? bp.isPatchBoundary : bp.isSurfaceBoundary;
			if (!isBoundary) { continue; }

			if (boundaryPoints.contains(bpId)) {
				boundaryPoints[bpId].isPatchBoundary |= bp.isPatchBoundary;
				boundaryPoints[bpId].isPatchCorner |= bp.isPatchCorner;
				boundaryPoints[bpId].isSurfaceBoundary |= bp.isSurfaceBoundary;
				boundaryPoints[bpId].isSurfaceCorner |= bp.isSurfaceCorner;

				for (auto& [nbr, data] : bp.neighbours) {
					const auto& nbrBp = surface->m_boundaryPoints[data.idx];
					bool nbrIsBoundary = includePatchBoundaries ? nbrBp.isPatchBoundary : nbrBp.isSurfaceBoundary;
					if (nbrIsBoundary) {
						boundaryPoints[bpId].neighbours[nbr].idx = data.idx;
						for (auto& [surfId, patchIdx] : data.surfIdPatchIdx) {
							boundaryPoints[bpId].neighbours[nbr].surfIdPatchIdx[surfId].insert(patchIdx.begin(), patchIdx.end());
						}
					}
				}
				for (auto& [surfId, patchIdx] : bp.data.surfIdPatchIdx) {
					boundaryPoints[bpId].data.surfIdPatchIdx[surfId].insert(patchIdx.begin(), patchIdx.end());
				}
			} else {
				boundaryPoints[bpId] = bp;

				std::unordered_map<Object**, BpData> neighbours;
				for (auto& [nbr, data] : bp.neighbours) {
					const auto& nbrBp = surface->m_boundaryPoints[data.idx];
					bool nbrIsBoundary = includePatchBoundaries ? nbrBp.isPatchBoundary : nbrBp.isSurfaceBoundary;
					if (nbrIsBoundary) {
						neighbours[nbr] = data;
					}
				}
				boundaryPoints[bpId].neighbours = neighbours;
			}
		}
	}
	// surface boundary points can become "corners" if they are patch corners and are somehow collapsed
	for (auto& [id, bp] : boundaryPoints) {
		if (bp.isSurfaceBoundary && bp.isPatchCorner && bp.neighbours.size() > 2) {
			boundaryPoints[id].isSurfaceCorner = true;
		}
	}
	return boundaryPoints;
}

std::vector<Surface::Cycle3> Surface::FindCycles3(const std::vector<Surface*>& surfaces, bool includePatchBoundaries) {
	std::unordered_map<int, BoundaryPoint> boundaryPoints = CombineBoundaryPoints(surfaces, includePatchBoundaries);

	std::unordered_set<int> criticalPoints;
	for (const auto& [id, bp] : boundaryPoints) {
		bool isCorner = includePatchBoundaries ? bp.isPatchCorner : bp.isSurfaceCorner;
		if (isCorner) {
			criticalPoints.insert(id);
		}
	}

	std::unordered_map<int, std::vector<Edge>> criticalGraph;
	for (int criticalId : criticalPoints) {
		const BoundaryPoint& cp = boundaryPoints[criticalId];
		for (auto& [nbr, data] : cp.neighbours) {
			std::vector<NetPoint> path;
			NetPoint prev = { cp.thisPoint, cp.data.surfIdPatchIdx };
			NetPoint current = { nbr, data.surfIdPatchIdx };
			path.push_back(prev);

			while (true) {
				path.push_back(current);
				int currentId = (*current.thisPoint)->id;
				if (criticalPoints.contains(currentId)) {
					Edge edge;
					edge.start = { cp.thisPoint, cp.data.surfIdPatchIdx };
					edge.end = current;
					edge.intermediate.assign(path.begin() + 1, path.end() - 1);
					criticalGraph[criticalId].push_back(edge);
					break;
				}

				const BoundaryPoint& bp = boundaryPoints[currentId];
				// if (bp.neighbours.size() != 2) break;  // bad topology
				auto it = bp.neighbours.begin();
				int prevId = (*prev.thisPoint)->id;
				int nextId = (*it->first)->id;
				if (prevId == nextId) {
					++it;
				}
				NetPoint next = { it->first, it->second.surfIdPatchIdx };

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
			int idB = (*edgeAB.end.thisPoint)->id;
			if (idB <= idA) continue; // assert idA < idB

			auto itB = criticalGraph.find(idB);
			if (itB == criticalGraph.end()) continue;

			// B-C edges
			for (const auto& edgeBC : itB->second) {
				int idC = (*edgeBC.end.thisPoint)->id;
				if (idC <= idB || idC == idA) continue; // assert idB < idC and idC != idA

				auto itC = criticalGraph.find(idC);
				if (itC == criticalGraph.end()) continue;

				// now search C-A edges to close triangle
				for (const auto& edgeCA : itC->second) {
					int backToA = (*edgeCA.end.thisPoint)->id;
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
#pragma endregion

Surface::Surface(bool increment) : m_aPoints(0), m_bPoints(0), m_surfaceType(SurfaceType::Flat) {
	intersectable = true;
	m_divisions = Patch::rowSize;
	m_type = "Surface";
	std::ostringstream os;
	os << "surface_" << m_globalSurfaceNum;
	name = os.str();
	if (increment) {
		m_globalSurfaceNum += 1;
	}
}

Surface::Surface(SurfaceType type, unsigned int aPoints, unsigned int bPoints, unsigned int divisions, std::vector<Object*> controlPoints, int id) :
	m_surfaceType(type), m_aPoints(aPoints), m_bPoints(bPoints) {
	intersectable = true;
	m_divisions = divisions;
	// required, since Gregory patches require id set at construction stage
	if (id != -1) {
		this->id = id;
	}
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
	InitializeBoundaryPoints();
	DetectSurfaceBoundaryPoints();
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
			ImGui::PushID(i);
			if (ImGui::Selectable(m_controlPoints[i]->name.c_str(), i == m_selectedIdx, ImGuiSelectableFlags_SpanAllColumns)) {
				if (m_selectedIdx == i) {
					m_selectedIdx = -1;
				} else {
					m_selectedIdx = i;
				}
			}
			ImGui::PopID();
		}
		ImGui::EndTable();
	}
}

void Surface::Replace(int id, Object* obj) {
	bool replacedAny = false;

	for (auto& p : m_controlPoints) {
		if (p && p->id == id) {
			p->RemoveParent(this);
			p = obj;
			if (obj != nullptr) {
				obj->AddParent(this);
			}
			replacedAny = true;
		}
	}

	if (replacedAny && obj != nullptr) {
		UpdateMidpoint();
		geometryChanged = true;
	}
}

SurfaceType Surface::GetSurfaceType() const {
	return m_surfaceType;
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

const Patch& Surface::GetPatch(int idx) const {
	return m_patches.at(idx);
}

void Surface::ClearControlPoints() {
	m_controlPoints.clear();
}

std::pair<unsigned int, unsigned int> Surface::NumberOfPatches() const {
	unsigned int aPatch = m_surfaceType == SurfaceType::Flat ?
		(m_aPoints - 1) / (Patch::rowSize - 1) : (m_aPoints) / (Patch::rowSize - 1);
	unsigned int bPatch = (m_bPoints - 1) / (Patch::rowSize - 1);

	return std::make_pair(aPatch, bPatch);
}

std::pair<unsigned int, unsigned int> Surface::GetUVPatches() const {
	auto [aPatch, bPatch] = NumberOfPatches();
	return std::make_pair(bPatch, aPatch);
}

std::array<double, 4> Surface::B3(double t) {
	double tInv = 1 - t;
	return {
		tInv * tInv * tInv,
		3 * t * tInv * tInv,
		3 * t * t * tInv,
		t * t * t
	};
}

std::array<double, 4> Surface::dB3(double t) {
	double tInv = 1 - t;
	return {
		-3 * tInv * tInv,
		 3 * tInv * (1 - 3 * t),
		 3 * t * (2 - 3 * t),
		 3 * t * t
	};
}

gmod::vector3<double> Surface::sumBasis(const std::array<gmod::vector3<double>, 16>& patch, const std::array<double, 4>& basisU, const std::array<double, 4>& basisV) {
	gmod::vector3<double> sum = { 0,0,0 };
	sum = sum + basisV[0] * (basisU[0] * patch[0] +
		basisU[1] * patch[1] +
		basisU[2] * patch[2] +
		basisU[3] * patch[3]);
	sum = sum + basisV[1] * (basisU[0] * patch[4] +
		basisU[1] * patch[5] +
		basisU[2] * patch[6] +
		basisU[3] * patch[7]);
	sum = sum + basisV[2] * (basisU[0] * patch[8] +
		basisU[1] * patch[9] +
		basisU[2] * patch[10] +
		basisU[3] * patch[11]);
	sum = sum + basisV[3] * (basisU[0] * patch[12] +
		basisU[1] * patch[13] +
		basisU[2] * patch[14] +
		basisU[3] * patch[15]);
	return sum;
}

std::array<gmod::vector3<double>, Patch::patchSize> Surface::GetPatch(double u, double v) const {
	const double eps = 1e-12;
	auto [aPatch, bPatch] = NumberOfPatches();

	u = std::clamp(u, eps, static_cast<double>(bPatch) - eps);
	v = std::clamp(v, eps, static_cast<double>(aPatch) - eps);
	unsigned int patchIndex = static_cast<unsigned int>(v) * bPatch + static_cast<unsigned int>(u);

	std::array<gmod::vector3<double>, Patch::patchSize> result;
	const auto& patch = GetPatch(patchIndex);

	for (int k = 0; k < Patch::patchSize; ++k) {
		result[k] = m_controlPoints[patch.indices[k]]->position();
	}
	return result;
}

std::pair<double, double> Surface::LocalUV(double u, double v) const {
	double localU = u - static_cast<int>(u);
	double localV = v - static_cast<int>(v);
	return std::make_pair(localU, localV);
}

#pragma region IGEOMETRICAL
IGeometrical::XYZBounds Surface::WorldBounds() const {
	auto [minXIt, maxXIt] = std::minmax_element(m_controlPoints.begin(), m_controlPoints.end(),
		[](const Object* a, const Object* b) {
		return a->position().x() < b->position().x();
	});

	auto [minYIt, maxYIt] = std::minmax_element(m_controlPoints.begin(), m_controlPoints.end(),
		[](const Object* a, const Object* b) {
		return a->position().y() < b->position().y();
	});

	auto [minZIt, maxZIt] = std::minmax_element(m_controlPoints.begin(), m_controlPoints.end(),
		[](const Object* a, const Object* b) {
		return a->position().z() < b->position().z();
	});

	return {
		gmod::vector3<double>{
			(*minXIt)->position().x(),
			(*minYIt)->position().y(),
			(*minZIt)->position().z()
		},
		gmod::vector3<double>{
			(*maxXIt)->position().x(),
			(*maxYIt)->position().y(),
			(*maxZIt)->position().z()
		}
	};
}

IGeometrical::UVBounds Surface::ParametricBounds() const {
	auto [aPatch, bPatch] = NumberOfPatches();
	return { 0.0, static_cast<double>(bPatch), 0.0, static_cast<double>(aPatch) };
}

bool Surface::IsUClosed() const {
	return false;
}

bool Surface::IsVClosed() const {
	return m_surfaceType == SurfaceType::Cylindric;
}

gmod::vector3<double> Surface::Point(double u, double v) const {
	auto [localU, localV] = LocalUV(u, v);

	std::array<double, 4> basisU = B3(localU);
	std::array<double, 4> basisV = B3(localV);

	return sumBasis(GetPatch(u, v), basisU, basisV);
}

gmod::vector3<double> Surface::Tangent(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	auto [localU, localV] = LocalUV(u, v);

	std::array<double, 4> basisU = B3(localU);
	std::array<double, 4> basisV = B3(localV);

	std::array<double, 4> dBu = dB3(localU);
	std::array<double, 4> dBv = dB3(localV);

	auto patch = GetPatch(u, v);
	gmod::vector3<double> du = normalize(sumBasis(patch, dBu, basisV));
	gmod::vector3<double> dv = normalize(sumBasis(patch, basisU, dBv));

	if (dPu) { *dPu = du; }
	if (dPv) { *dPv = dv; }

	return normalize(du + dv);
}

gmod::vector3<double> Surface::Normal(double u, double v, gmod::vector3<double>* dPu, gmod::vector3<double>* dPv) const {
	gmod::vector3<double> du, dv;
	Tangent(u, v, &du, &dv);

	if (dPu) { *dPu = du; }
	if (dPv) { *dPv = dv; }

	return normalize(cross(du, dv));
}
#pragma endregion