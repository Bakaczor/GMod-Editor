#include "Gregory.h"
#include <algorithm>
#include <numeric>

using namespace app;

unsigned short Gregory::m_globalGregoryNum = 0;

Gregory::Gregory(const std::array<Edge, 3>& edges, const std::vector<Surface*>& surfaces) {
	m_divisions = Patch::rowSize;
	m_type = "Gregory";
	std::ostringstream os;
	os << "gregory_" << m_globalGregoryNum;
	name = os.str();
	m_globalGregoryNum += 1;

	for (const auto& edge : edges) {
		std::vector<Object*> halfPatch(8);
		halfPatch[0] = *edge.start.thisPoint;
		halfPatch[1] = *edge.intermediate[0].thisPoint;
		halfPatch[2] = *edge.intermediate[1].thisPoint;
		halfPatch[3] = *edge.end.thisPoint;

		auto inward = RetrieveInwardEdge(edge, surfaces);
		halfPatch[4] = inward[0];
		halfPatch[5] = inward[1];
		halfPatch[6] = inward[2];
		halfPatch[7] = inward[3];

		m_controlPoints.insert(m_controlPoints.end(), halfPatch.begin(), halfPatch.end());
	}

	UpdateMidpoint();
	for (auto& obj : m_controlPoints) {
		obj->AddParent(this);
	}
	geometryChanged = true;
}

std::array<Object*, 4> app::Gregory::RetrieveInwardEdge(const Edge& edge, const std::vector<Surface*>& surfaces) {
	const NetPoint& b0 = edge.start;
	const NetPoint& b1 = edge.intermediate[0];
	const NetPoint& b2 = edge.intermediate[1];
	const NetPoint& b3 = edge.end;

	// find common surface among all 4 points
	std::unordered_set<int> commonSurfaces;
	for (auto& [surfId, patchIdx] : b0.surfIdPatchIdx) {
		if (b1.surfIdPatchIdx.contains(surfId) &&
			b2.surfIdPatchIdx.contains(surfId) &&
			b3.surfIdPatchIdx.contains(surfId)) {
			commonSurfaces.insert(surfId);
		}
	}

	for (int surfaceId : commonSurfaces) {
		// find surface
		auto it = std::find_if(surfaces.begin(), surfaces.end(), [surfaceId](const auto& s) {
			return s->id == surfaceId;
		});
		if (it == surfaces.end()) { continue; }
		const Surface* surf = *it;

		// find common patch among all 4 points
		std::unordered_set<int> commonPatches;
		for (int pid : b0.surfIdPatchIdx.at(surfaceId)) {
			if (b1.surfIdPatchIdx.at(surfaceId).contains(pid) &&
				b2.surfIdPatchIdx.at(surfaceId).contains(pid) && 
				b3.surfIdPatchIdx.at(surfaceId).contains(pid)) {
				commonPatches.insert(pid);
			}
		}
		if (commonPatches.empty()) { continue; }

		// choose one patch (if the whole edge belongs to patch, then all are equally good)
		int patchId = *commonPatches.begin();
		const Patch& patch = surf->GetPatch(patchId);
		const std::vector<Object*>& controlPoints = surf->GetControlPoints();

		std::array<Object*, Patch::patchSize> patchPoints;
		for (int i = 0; i < Patch::patchSize; ++i) {
			patchPoints[i] = controlPoints[patch.indices[i]];
		}

		std::array<Object*, 4> edgePoints = { *b0.thisPoint, *b1.thisPoint, *b2.thisPoint, *b3.thisPoint };
		std::optional<std::array<Object*, 4>> opt = MatchInwardEdge(patchPoints, edgePoints);
		if (opt.has_value()) { return opt.value(); } 
		else {
			std::reverse(edgePoints.begin(), edgePoints.end());
			opt = MatchInwardEdge(patchPoints, edgePoints);
			if (opt.has_value()) { 
				std::reverse(opt.value().begin(), opt.value().end());
				return opt.value();
			}
		}
	}
	throw std::runtime_error("Could not find valid inward edge for patch.");
}

std::optional<std::array<Object*, 4>> Gregory::MatchInwardEdge(const std::array<Object*, Patch::patchSize>& patchPoints, const std::array<Object*, Patch::rowSize>& edgePoints) {
	// check rows
	int row, col;
	bool match;

	row = 0;
	match = true;
	for (col = 0; col < Patch::rowSize; ++col) {
		if (patchPoints[row * Patch::rowSize + col]->id != edgePoints[col]->id) {
			match = false;
			break;
		}
	}
	if (match) {
		return std::array<Object*, 4> {
			patchPoints[(row + 1) * Patch::rowSize + 0],
			patchPoints[(row + 1) * Patch::rowSize + 1],
			patchPoints[(row + 1) * Patch::rowSize + 2],
			patchPoints[(row + 1) * Patch::rowSize + 3]
		};
	}

	row = 3;
	match = true;
	for (col = 0; col < Patch::rowSize; ++col) {
		if (patchPoints[row * Patch::rowSize + col]->id != edgePoints[col]->id) {
			match = false;
			break;
		}
	}
	if (match) {
		return std::array<Object*, 4> {
			patchPoints[(row - 1) * Patch::rowSize + 0],
			patchPoints[(row - 1) * Patch::rowSize + 1],
			patchPoints[(row - 1) * Patch::rowSize + 2],
			patchPoints[(row - 1) * Patch::rowSize + 3]
		};
	}
	
	col = 0;
	match = true;
	for (row = 0; row < Patch::rowSize; ++row) {
		if (patchPoints[row * Patch::rowSize + col]->id != edgePoints[row]->id) {
			match = false;
			break;
		}
	}
	if (match) {
		return std::array<Object*, 4> {
			patchPoints[0 * Patch::rowSize + col + 1],
			patchPoints[1 * Patch::rowSize + col + 1],
			patchPoints[2 * Patch::rowSize + col + 1],
			patchPoints[3 * Patch::rowSize + col + 1]
		};
	}

	col = 3;
	match = true;
	for (row = 0; row < Patch::rowSize; ++row) {
		if (patchPoints[row * Patch::rowSize + col]->id != edgePoints[row]->id) {
			match = false;
			break;
		}
	}
	if (match) {
		return std::array<Object*, 4> {
			patchPoints[0 * Patch::rowSize + col - 1],
			patchPoints[1 * Patch::rowSize + col - 1],
			patchPoints[2 * Patch::rowSize + col - 1],
			patchPoints[3 * Patch::rowSize + col - 1]
		};
	}
	
	return std::nullopt;
}

Gregory::DeCasteljauRes Gregory::DeCasteljau(const std::array<gmod::vector3<double>, 4>& controlPoints) {
	gmod::vector3<double> b0 = controlPoints[0];
	gmod::vector3<double> b1 = controlPoints[1];
	gmod::vector3<double> b2 = controlPoints[2];
	gmod::vector3<double> b3 = controlPoints[3];

	DeCasteljauRes result;

	result.R[0] = (b0 + b1) * 0.5;
	result.R[1] = (b1 + b2) * 0.5;
	result.R[2] = (b2 + b3) * 0.5;

	result.S[0] = (result.R[0] + result.R[1]) * 0.5;
	result.S[1] = (result.R[1] + result.R[2]) * 0.5;

	result.T = (result.S[0] + result.S[1]) * 0.5;

	return result;
}

gmod::vector3<double> Gregory::ExtrapolateToward(gmod::vector3<double>& A, gmod::vector3<double>& B) {
	return B * 2 - A;
}

gmod::vector3<double> Gregory::Q(gmod::vector3<double>& P2, gmod::vector3<double>& P3) {
	return (3 * P2 - P3) * 0.5;
}

gmod::vector3<double> Gregory::P(const std::array<gmod::vector3<double>, 3>& Q) {
	return (Q[0] + Q[1] + Q[2]) * (1.0 / 3.0);
}

gmod::vector3<double> Gregory::P1(gmod::vector3<double>& Q, gmod::vector3<double>& P) {
	return (2 * Q - P) * (1.0 / 3.0);
}

std::array<gmod::vector3<double>, 3> Gregory::gVecs(const std::array<TangentData, 2>& tangentData) {
	const gmod::vector3<double> g0 = (tangentData[0].a + tangentData[0].b) * 0.5;
	const gmod::vector3<double> g2 = (tangentData[1].a + tangentData[1].b) * 0.5;
	const gmod::vector3<double> g1 = (g0 + g2) * 0.5;

	return { g0, g1, g2 };
}

std::array<gmod::vector3<double>, 3> app::Gregory::cVecs(const std::array<gmod::vector3<double>, 4>& controlPoints) {
	return { controlPoints[1] - controlPoints[0], controlPoints[2] - controlPoints[1] , controlPoints[3] - controlPoints[2] };
}

gmod::vector3<double> Gregory::B2(double t, const std::array<gmod::vector3<double>, 3>& p) {
	const double tInv = 1.0 - t;
	return
		tInv * tInv * p[0] +
		2.0 * tInv * t * p[1] +
		t * t * p[2];
}

std::array<double, 2> Gregory::evaluateKH(const gmod::vector3<double>& b, const gmod::vector3<double>& g, const gmod::vector3<double>& c) {
	// Solves:
	// k * g.x + h * c.x = b.x
	// k * g.y + h * c.y = b.y
	// k * g.z + h * c.z = b.z
	// Since it's an overdetermined system, solves using 2 equations at a time and uses the third as verification.

	const double eps = 1e-8;

	auto solve = [eps](double bx, double by, double gx, double gy, double cx, double cy) -> std::pair<bool, std::array<double, 2>> {
		double W = gx * cy - gy * cx;
		if (std::abs(W) > eps) {
			double Wk = bx * cy - by * cx;
			double Wh = gx * by - gy * bx;
			return { true, {Wk / W, Wh / W} };
		}
		return { false, {0.0, 0.0} };
	};

	auto check = [eps](std::array<double, 2>& result, double bz, double gz, double cz) -> bool {
		double check = result[0] * gz + result[1] * cz;
		return std::abs(check - bz) < eps;
	};

	auto result = solve(b.x(), b.y(), g.x(), g.y(), c.x(), c.y());
	if (result.first && check(result.second, b.z(), g.z(), c.z())) {
		return result.second;
		
	}

	result = solve(b.y(), b.z(), g.y(), g.z(), c.y(), c.z());
	if (result.first && check(result.second, b.x(), g.x(), c.x())) {
		return result.second;
		
	}

	result = solve(b.x(), b.z(), g.x(), g.z(), c.x(), c.z());
	if (result.first && check(result.second, b.y(), g.y(), c.y())) {
		return result.second;
	}

	// shouldn't happen
	return { 0, 0 };
}

std::array<gmod::vector3<double>, 2> Gregory::D(
	const std::array<gmod::vector3<double>, 2>& bVecs,
	const std::array<gmod::vector3<double>, 3>& gVecs,
	const std::array<gmod::vector3<double>, 4>& controlPoints) {

	std::array<gmod::vector3<double>, 3> c = cVecs(controlPoints);
	std::array<double, 2> k0h0 = evaluateKH(bVecs[0], gVecs[0], c[0]);
	std::array<double, 2> k1h1 = evaluateKH(bVecs[1], gVecs[2], c[2]);
	double& k0 = k0h0[0];
	double& h0 = k0h0[1];
	double& k1 = k1h1[0];
	double& h1 = k1h1[1];
	auto lerp = [](double f0, double f1, double t) -> double {
		return f0 * (1 - t) + f1 * t;
	};
	
	const double one_third = 1.0 / 3.0;
	const double two_third = 2.0 / 3.0;

	gmod::vector3<double> d1 = lerp(k0, k1, one_third) * B2(one_third, gVecs) + lerp(h0, h1, one_third) * B2(one_third, c);
	gmod::vector3<double> d2 = lerp(k0, k1, two_third) * B2(two_third, gVecs) + lerp(h0, h1, two_third) * B2(two_third, c);
	return { controlPoints[1] + d1, controlPoints[2] + d2 };
}

void Gregory::CalculateGregoryPatchesAndTangentVectors(std::array<Gregory::QuadGreg, 3>& gregoryPatches, std::vector<gmod::vector3<double>>& tangentVectorsEnds) const {
	std::array<EdgeData, 3> edgeData;
	std::array<TangentData, 3> edgeTangentDataCCW;
	std::array<TangentData, 3> edgeTangentDataCC;
	for (int edgeIdx = 0; edgeIdx < 3; edgeIdx++) {
		std::array<gmod::vector3<double>, 4> boundaryPoints;
		for (int i = 0; i < 4; i++) {
			boundaryPoints[i] = m_controlPoints[edgeIdx * 8 + i]->position();
		}
		edgeData[edgeIdx].bernsteinPoints = boundaryPoints;
		std::array<gmod::vector3<double>, 4> inwardPoints;
		for (int i = 0; i < 4; i++) {
			inwardPoints[i] = m_controlPoints[edgeIdx * 8 + 4 + i]->position();
		}
		DeCasteljauRes boundaryRes = DeCasteljau(boundaryPoints);
		DeCasteljauRes inwardRes = DeCasteljau(inwardPoints);
		edgeData[edgeIdx].boundaryRes = boundaryRes;

		edgeData[edgeIdx].P3 = boundaryRes.T;
		edgeData[edgeIdx].P2 = ExtrapolateToward(inwardRes.T, boundaryRes.T);

		edgeData[edgeIdx].edgeGregPoints[0] = ExtrapolateToward(inwardRes.R[0], boundaryRes.R[0]);
		edgeData[edgeIdx].edgeGregPoints[1] = ExtrapolateToward(inwardRes.S[0], boundaryRes.S[0]);
		edgeData[edgeIdx].edgeGregPoints[2] = ExtrapolateToward(inwardRes.S[1], boundaryRes.S[1]);
		edgeData[edgeIdx].edgeGregPoints[3] = ExtrapolateToward(inwardRes.R[2], boundaryRes.R[2]);

		tangentVectorsEnds.push_back(boundaryRes.R[0]); tangentVectorsEnds.push_back(edgeData[edgeIdx].edgeGregPoints[0]);
		tangentVectorsEnds.push_back(boundaryRes.S[0]); tangentVectorsEnds.push_back(edgeData[edgeIdx].edgeGregPoints[1]);
		tangentVectorsEnds.push_back(boundaryRes.R[1]); tangentVectorsEnds.push_back(edgeData[edgeIdx].P2);
		tangentVectorsEnds.push_back(boundaryRes.S[1]); tangentVectorsEnds.push_back(edgeData[edgeIdx].edgeGregPoints[2]);
		tangentVectorsEnds.push_back(boundaryRes.R[2]); tangentVectorsEnds.push_back(edgeData[edgeIdx].edgeGregPoints[3]);

		edgeData[edgeIdx].Q = Q(edgeData[edgeIdx].P2, edgeData[edgeIdx].P3);

		edgeTangentDataCCW[edgeIdx].a = boundaryRes.T - boundaryRes.S[0];
		edgeTangentDataCCW[edgeIdx].C = boundaryRes.T;
		edgeTangentDataCCW[edgeIdx].b = boundaryRes.S[1] - boundaryRes.T;

		edgeTangentDataCC[edgeIdx].a = boundaryRes.T - boundaryRes.S[1];
		edgeTangentDataCC[edgeIdx].C = boundaryRes.T;
		edgeTangentDataCC[edgeIdx].b = boundaryRes.S[0] - boundaryRes.T;
	}

	// fill remaining partial data
	gmod::vector3<double> mid = P({ edgeData[0].Q, edgeData[1].Q , edgeData[2].Q });
	edgeData[0].P0 = edgeData[1].P0 = edgeData[2].P0 = mid;

	for (int edgeIdx = 0; edgeIdx < 3; edgeIdx++) {
		edgeData[edgeIdx].P1 = P1(edgeData[edgeIdx].Q, edgeData[edgeIdx].P0);
	}

	std::array<TangentData, 3> midTangentDataCCW;
	std::array<TangentData, 3> midTangentDataCC;
	for (int edgeIdx = 0; edgeIdx < 3; edgeIdx++) {
		int rightEdgeIdx = edgeIdx + 1;
		if (rightEdgeIdx == 3) { rightEdgeIdx = 0; }
		int leftEdgeIdx = edgeIdx - 1;
		if (leftEdgeIdx == -1) { leftEdgeIdx = 2; }

		midTangentDataCCW[edgeIdx].a = edgeData[edgeIdx].P0 - edgeData[leftEdgeIdx].P1;
		midTangentDataCCW[edgeIdx].C = edgeData[edgeIdx].P0;
		midTangentDataCCW[edgeIdx].b = edgeData[rightEdgeIdx].P1 - edgeData[edgeIdx].P0;

		midTangentDataCC[edgeIdx].a = edgeData[edgeIdx].P0 - edgeData[rightEdgeIdx].P1;
		midTangentDataCC[edgeIdx].C = edgeData[edgeIdx].P0;
		midTangentDataCC[edgeIdx].b = edgeData[leftEdgeIdx].P1 - edgeData[edgeIdx].P0;
	}

	std::array<PatchData, 3> patchData;
	for (int patchIdx = 0; patchIdx < 3; patchIdx++) {
		int leftEdgeIdx = patchIdx;
		int rightEdgeIdx = (patchIdx + 1) % 3;

		auto leftGVecs = gVecs({ midTangentDataCCW[leftEdgeIdx], edgeTangentDataCCW[leftEdgeIdx]});
		auto leftGregs = D(
			{ midTangentDataCCW[leftEdgeIdx].b, edgeTangentDataCCW[leftEdgeIdx].b },
			leftGVecs,
			{ edgeData[leftEdgeIdx].P0, edgeData[leftEdgeIdx].P1, edgeData[leftEdgeIdx].P2, edgeData[leftEdgeIdx].P3 }
		);
		patchData[patchIdx].patchGregPoints[0] = leftGregs[0];
		patchData[patchIdx].patchGregPoints[1] = leftGregs[1];

		auto rightGVecs = gVecs({ midTangentDataCC[rightEdgeIdx], edgeTangentDataCC[rightEdgeIdx] });
		auto rightGregs = D(
			{ midTangentDataCC[rightEdgeIdx].b, edgeTangentDataCC[rightEdgeIdx].b },
			rightGVecs,
			{ edgeData[rightEdgeIdx].P0, edgeData[rightEdgeIdx].P1, edgeData[rightEdgeIdx].P2, edgeData[rightEdgeIdx].P3 }
		);
		patchData[patchIdx].patchGregPoints[2] = rightGregs[0];
		patchData[patchIdx].patchGregPoints[3] = rightGregs[1];
	}

	// transfer to QuadGreg, start from left-down corner
	for (int patchIdx = 0; patchIdx < 3; patchIdx++) {
		int leftEdgeIdx = patchIdx;
		int rightEdgeIdx = (patchIdx + 1) % 3;

		gregoryPatches[patchIdx].cornerPoints = {
			edgeData[leftEdgeIdx].P3,
			edgeData[leftEdgeIdx].bernsteinPoints[3],
			edgeData[rightEdgeIdx].P3,
			edgeData[rightEdgeIdx].P0
		};

		gregoryPatches[patchIdx].edgePoints = {
			edgeData[leftEdgeIdx].P2,
			edgeData[leftEdgeIdx].boundaryRes.S[1],
			edgeData[leftEdgeIdx].boundaryRes.R[2],
			edgeData[rightEdgeIdx].boundaryRes.R[0],
			edgeData[rightEdgeIdx].boundaryRes.S[0],
			edgeData[rightEdgeIdx].P2,
			edgeData[rightEdgeIdx].P1,
			edgeData[leftEdgeIdx].P1
		};

		gregoryPatches[patchIdx].facePoints = {
			patchData[patchIdx].patchGregPoints[1],
			edgeData[leftEdgeIdx].edgeGregPoints[2],
			edgeData[leftEdgeIdx].edgeGregPoints[3],
			edgeData[rightEdgeIdx].edgeGregPoints[0],
			edgeData[rightEdgeIdx].edgeGregPoints[1],
			patchData[patchIdx].patchGregPoints[3],
			patchData[patchIdx].patchGregPoints[2],
			patchData[patchIdx].patchGregPoints[0],
		};
	}
}

void Gregory::UpdateMesh(const Device& device) {
	std::array<Gregory::QuadGreg, 3> gregoryPatches;
	std::vector<gmod::vector3<double>> tangentVectorsEnds;
	CalculateGregoryPatchesAndTangentVectors(gregoryPatches, tangentVectorsEnds);

	if (m_showVectors) {
		std::vector<Vertex_Po> verts;
		verts.reserve(tangentVectorsEnds.size() * 2);
		std::vector<USHORT> idxs(tangentVectorsEnds.size() * 2);
		std::iota(idxs.begin(), idxs.end(), 0);

		for (const auto& pos : tangentVectorsEnds) {
			verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
		}
		m_vectorsMesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}


	std::vector<gmod::vector3<double>> positions(49);
	const auto& p0 = gregoryPatches[0];
	const auto& p1 = gregoryPatches[1];
	const auto& p2 = gregoryPatches[2];

	// === Corners (7 unique) ===
	positions[0] = p0.cornerPoints[0]; // p0_0 = p2_2
	positions[1] = p0.cornerPoints[1]; // p1_0
	positions[2] = p0.cornerPoints[2]; // p2_0 = p0_1
	positions[3] = p0.cornerPoints[3]; // p3 shared
	positions[4] = p1.cornerPoints[1]; // p1_1
	positions[5] = p1.cornerPoints[2]; // p2_1 = p0_2
	positions[6] = p2.cornerPoints[1]; // p1_2

	// === Edges (18 unique) ===
	// patch 0
	positions[7] = p0.edgePoints[0]; // e0-_0 = e2+_2
	positions[8] = p0.edgePoints[1]; // e0+_0
	positions[9] = p0.edgePoints[2]; // e1-_0
	positions[10] = p0.edgePoints[3]; // e1+_0
	positions[11] = p0.edgePoints[4]; // e2-_0
	positions[12] = p0.edgePoints[5]; // e2+_0 = e0-_1
	positions[13] = p0.edgePoints[6]; // e3-_0 = e3+_1
	positions[14] = p0.edgePoints[7]; // e3+_0 = e3-_2
	// patch 1
	positions[15] = p1.edgePoints[1]; // e0+_1
	positions[16] = p1.edgePoints[2]; // e1-_1
	positions[17] = p1.edgePoints[3]; // e1+_1
	positions[18] = p1.edgePoints[4]; // e2-_1
	positions[19] = p1.edgePoints[5]; // e2+_1 = e0-_2
	positions[20] = p1.edgePoints[6]; // e3-_1 = e3+_2
	// patch 2
	positions[21] = p2.edgePoints[1]; // e0+_2
	positions[22] = p2.edgePoints[2]; // e1-_2
	positions[23] = p2.edgePoints[3]; // e1+_2
	positions[24] = p2.edgePoints[4]; // e2-_2

	// === Faces (24 unique) ===
	for (int i = 0; i < 8; ++i) { positions[25 + i] = p0.facePoints[i]; }  // 25–32
	for (int i = 0; i < 8; ++i) { positions[33 + i] = p1.facePoints[i]; }  // 33–40
	for (int i = 0; i < 8; ++i) { positions[41 + i] = p2.facePoints[i]; }  // 41–48

	// === Convert to Vertex_Po ===
	std::vector<Vertex_Po> verts;
	verts.reserve(49);
	for (const auto& pos : positions) {
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}

	// === Indices ===
	std::vector<USHORT> idxs;
	idxs.reserve(60);

	// Patch 0
	idxs.insert(idxs.end(), {
		0, 1, 2, 3,				// corners
		7,8,9,10,11,12,13,14,	// edges
		25,26,27,28,29,30,31,32 // faces
	});

	// Patch 1
	idxs.insert(idxs.end(), {
		2, 4, 5, 3,				 // corners
		12,15,16,17,18,19,20,13, // edges
		33,34,35,36,37,38,39,40	 // faces
	});

	// Patch 2
	idxs.insert(idxs.end(), {
		5, 6, 0, 3,				// corners
		17,21,22,23,24,7,14,20, // edges
		41,42,43,44,45,46,47,48 // faces
	});

	m_gregoryMesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST);

	Object::UpdateMesh(device);
}

Gregory::~Gregory() {
	Object::~Object();
	for (auto& obj : m_controlPoints) {
		if (obj == nullptr) { continue; }
		obj->RemoveParent(this);
		if (obj->NumberOfParents() == 0) {
			obj->deletable = true;
		}
	}
}

void Gregory::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	if (m_showVectors) {
		map.at(ShaderType::Regular).Set(context);
		m_vectorsMesh.Render(context);
	}
	map.at(ShaderType::RegularWithTesselationGregory).Set(context);
	m_gregoryMesh.Render(context);
}

void Gregory::RenderProperties() {
	Object::RenderProperties();
	int divisions = m_divisions;
	ImGui::InputInt("Divisions", &divisions, 1, static_cast<int>(minDivisions));
	m_divisions = std::min(std::max(static_cast<int>(minDivisions), divisions), static_cast<int>(maxDivisions));

	ImGui::Separator();
	bool old = m_showVectors;
	ImGui::Checkbox("Show vectors", &m_showVectors);
	if (m_showVectors && m_showVectors != old) {
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

void Gregory::Replace(int id, Object* obj) {
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


