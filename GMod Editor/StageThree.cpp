#include "StageThree.h"
#include "Surface.h"
#include "IGeometrical.h"
#include "Helper.h"
#include "OffsetSurface.h"
#include <utility.h>

using namespace app;

std::vector<gmod::vector3<float>> StageThree::GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	gmod::vector3<double> baseCentre(centre.x(), centre.y(), centre.z()); // TODO : +m_radius
	Surface::Plane base = Surface::MakePlane(baseCentre, width, length, { 0,0,0 }, -69);
	Intersection::IDIG baseIDIG = { base.surface->id, dynamic_cast<IGeometrical*>(base.surface.get()) };

	std::vector<gmod::vector3<float>> path;
	path.push_back(gmod::vector3<float>(0, totalHeight, 0));
	for (const auto& params : m_millingParams) {
		auto elementsPath = GeneratePathForPart(sceneObjects, intersection, params, baseIDIG);
		std::copy(elementsPath.begin(), elementsPath.end(), std::back_inserter(path));
	}
	path.push_back(gmod::vector3<float>(0, totalHeight, 0));

	return path;
}

std::vector<gmod::vector3<float>> StageThree::GeneratePathForPart(
	const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection,
	const MillingPartParams& params, const Intersection::IDIG& base) const {

	std::vector<std::pair<Intersection::IDIG, Intersection::InterParams>> surfaces;
	std::vector<std::unique_ptr<OffsetSurface>> offsetSurfaceStorage; // the offset surfaces must exist until we exit the function

	// == offset part ==
	auto partIt = std::find_if(sceneObjects.begin(), sceneObjects.end(), [&params](const std::unique_ptr<Object>& obj) {
		return obj->name == params.name;
	});

	if (partIt == sceneObjects.end()) {
		throw std::exception("Wrong name - surface not found.");
	}

	Object* partObj = partIt->get();
	auto op = std::make_unique<OffsetSurface>(partObj, m_radius);
	offsetSurfaceStorage.push_back(std::move(op));

	IGeometrical* partG = dynamic_cast<IGeometrical*>(offsetSurfaceStorage.back().get());
	if (partG == nullptr) {
		throw std::exception("This object does not implement IGeometrical interface.");
	}

	Intersection::IDIG part = { partObj->id, partG };
	// =====

	// == offset surfaces ==
	for (const auto& surf : params.intersectingSurfaces) {
		auto it = std::find_if(sceneObjects.begin(), sceneObjects.end(), [&surf](const std::unique_ptr<Object>& obj) {
			return obj->name == surf.name;
		});

		if (it == sceneObjects.end()) {
			throw std::exception("Wrong name - surface not found.");
		}

		Object* obj = it->get();
		auto os = std::make_unique<OffsetSurface>(obj, m_radius);
		offsetSurfaceStorage.push_back(std::move(os));

		IGeometrical* g = dynamic_cast<IGeometrical*>(offsetSurfaceStorage.back().get());
		if (g == nullptr) {
			throw std::exception("This object does not implement IGeometrical interface.");
		}

		surfaces.push_back(std::make_pair(Intersection::IDIG{ obj->id, g }, surf.params));
	}
	// =====

	// == base contour ==
	intersection.SetIntersectionParameters(m_baseInterParams);
	unsigned int res = intersection.FindIntersection(std::make_pair(part, base));
	if (res != 0) {
		throw std::runtime_error("Should have found intersection, but didn't. Evaluate params.");
	}

	auto& pointsOfIntersection = intersection.GetPointsOfIntersection();
	std::vector<StageThree::InterPoint> baseContour(pointsOfIntersection.size());
	std::transform(pointsOfIntersection.begin(), pointsOfIntersection.end(), baseContour.begin(), 
		[&partG](const Intersection::PointOfIntersection& p) {
		auto normal = partG->Normal(p.uvs.u1, p.uvs.v1);
		return StageThree::InterPoint{
			.pos = gmod::vector3<float>(p.pos.x(), p.pos.y(), p.pos.z()),
			.norm = gmod::vector3<float>(normal.x(), normal.y(), normal.z()),
			.u = static_cast<float>(p.uvs.u1),
			.v = static_cast<float>(p.uvs.v1),
			.surf = partG
		};
	}); 
	// =====

	auto contour = FindContour(intersection, baseContour, params.insidePoint, part, surfaces);

	SegmentEnd3 startingPoint;
	SegmentGraph G = CutSurfaceIntoGraph(intersection, contour, part, params.epsilon, params.cutVertical, params.startFrom, startingPoint);
	std::vector<int> path = G.SpecialDFS3(startingPoint.id);

	// =====
	float addX = 0.f, addZ = 0.f;
	switch (params.startFrom) {
		case 1: {
			addZ = -diameter;
			break;
		} case 2: {
			addX = diameter;
			break;
		} case 3: {
			addZ = diameter;
			break;
		} case 4: {
			addX = -diameter;
			break;
		}
	}
	// =====

	std::vector<gmod::vector3<float>> fullPath;
	long firstIdx = G.vertices3[path.front()].segEnd.contourIdx;
	gmod::vector3<float> first = contour[firstIdx].pos;
	fullPath.push_back(gmod::vector3<float>(first.x() + addX, totalHeight, first.z() + addZ));
	fullPath.push_back(gmod::vector3<float>(first.x() + addX, first.y(), first.z() + addZ)); // TODO : lower y by radius

	for (int v = 0; v < path.size() - 1; v++) {
		int fromId = path[v];
		int toId = path[v + 1];

		const SegmentGraph::Vertex3& fromVertex = G.vertices3[fromId];
		const SegmentGraph::Vertex3& toVertex = G.vertices3[toId];

		auto it = std::find_if(fromVertex.neighbours.begin(), fromVertex.neighbours.end(), [&toId](const std::pair<int, SegmentGraph::Edge3>& e) {
			return e.first == toId;
		});

		if (it == fromVertex.neighbours.end()) {
			throw std::runtime_error("There is an error in created path - edge not found.");
		}

		auto currPos = contour[fromVertex.segEnd.contourIdx].pos;
		fullPath.push_back(currPos); // TODO : lower y by radius

		const SegmentGraph::Edge3& edge = it->second;
		if (edge.contourEdge) { // if we have contour edge, we need to add the points from contour
			int start = edge.seg.p1.contourIdx;
			int end = edge.seg.p2.contourIdx;

			if (start < end) {
				for (int j = start; j <= end; j++) {
					fullPath.push_back(contour[j].pos); // TODO : lower y by radius
				}
			} else {
				for (int j = start; j >= end; j--) {
					fullPath.push_back(contour[j].pos); // TODO : lower y by radius
				}
			}
		} else { // if we have inner edge we take points from segment
			for (const auto& el : edge.seg.p1p2) {
				fullPath.push_back(el); // TODO : lower y by radius
			}
		}
	}

	long lastIdx = G.vertices3[path.back()].segEnd.contourIdx;
	gmod::vector3<float> last = contour[lastIdx].pos;
	fullPath.push_back(last); // TODO : lower y by radius

	long n = contour.size();
	long overlapStart = (lastIdx + 1) % n;
	long overlapEnd = lastIdx;

	for (long k = overlapStart; k != overlapEnd; k = (k + 1) % n) {
		fullPath.push_back(contour[k].pos); // TODO : lower y by radius
	}
	fullPath.push_back(last); // TODO : lower y by radius
	fullPath.push_back(gmod::vector3<float>(last.x(), totalHeight, last.z()));

	// TODO : remove later
	for (auto& el : fullPath) {
		if (el.y() < baseY) {
			el = gmod::vector3<float>(el.x(), baseY, el.z());
		}
	}

	return fullPath;
}

std::vector<StageThree::InterPoint> StageThree::FindContour(Intersection& intersection,
	const std::vector<InterPoint>& baseContour, const gmod::vector3<float>& insidePoint,
	const Intersection::IDIG& part, const std::vector<std::pair<Intersection::IDIG, Intersection::InterParams>>& intersectingSurfaces) const {
	
	// == UV search ==
	const auto& uvBounds = part.s->ParametricBounds();
	const float diffU = uvBounds.uMax - uvBounds.uMin;
	const float diffV = uvBounds.vMax - uvBounds.vMin;
	const float uStep = diffU / m_samplingRes;
	const float vStep = diffV / m_samplingRes;
	const float offsetBaseHeight = baseY + m_radius;

	float insideU = 0;
	float insideV = 0;
	float bestDist = width;

	// sample uv plane
	float u = uvBounds.uMin;
	while (u <= uvBounds.uMax) {
		float v = uvBounds.vMin;
		while (v <= uvBounds.vMax) {
			auto p = part.s->Point(u, v);
			if (p.y() >= offsetBaseHeight) {
				const float diffX = insidePoint.x() - p.x();
				const float diffZ = insidePoint.z() - p.z();
				const float dist = diffX * diffX + diffZ * diffZ;
				if (dist < bestDist) {
					bestDist = dist;
					insideU = u;
					insideV = v;
				}
			}
			v += vStep;
		}
		u += uStep;
	}
	// =====

	std::vector<StageThree::InterPoint> finalContour = baseContour;
	for (auto& [surf, params] : intersectingSurfaces) {
		intersection.SetIntersectionParameters(params);
		unsigned int res = intersection.FindIntersection(std::make_pair(part, surf));
		if (res != 0) {
			throw std::runtime_error("Should have found intersection, but didn't. Evaluate params.");
		}

		auto& pointsOfIntersection = intersection.GetPointsOfIntersection();
		std::vector<StageThree::InterPoint> intersectionLine(pointsOfIntersection.size());
		std::transform(pointsOfIntersection.begin(), pointsOfIntersection.end(), intersectionLine.begin(),
			[&part](const Intersection::PointOfIntersection& p) {
			auto normal = part.s->Normal(p.uvs.u1, p.uvs.v1);
			return StageThree::InterPoint{
				.pos = gmod::vector3<float>(p.pos.x(), p.pos.y(), p.pos.z()),
				.norm = gmod::vector3<float>(normal.x(), normal.y(), normal.z()),
				.u = static_cast<float>(p.uvs.u1),
				.v = static_cast<float>(p.uvs.v1),
				.surf = part.s
			};
		});

		Combine(finalContour, intersectionLine, insideU, insideV, part);
	}

	return finalContour;
}

void StageThree::Combine(std::vector<InterPoint>& finalContour, const std::vector<InterPoint>& intersectionLine,
	float insideU, float insideV, const Intersection::IDIG& part) const {
	// find two points at which intersectionLine (open) intersects with finalContour (closed)
	// take the part of intersectionLine that is inside finalContour and create two contours
	// check using insideU and insideV (which should be inside the finalContour) which contour to take and return it

	struct Crossing {
		size_t i, j;
		float u, v;
	};
	std::vector<Crossing> intersections;

	int n = finalContour.size();
	int m = intersectionLine.size();

	for (size_t i = 0; i < finalContour.size(); i++) {
		auto& A = finalContour[i];
		auto& B = finalContour[(i + 1) % n];

		for (size_t j = 0; j < intersectionLine.size(); j++) {
			auto& C = intersectionLine[j];
			auto& D = intersectionLine[(j + 1) % m];

			PartUVs interUVs;
			if (DoSegementsCross({ A.u, A.v }, { B.u, B.v }, { C.u, C.v }, { D.u, D.v }, interUVs)) {
				intersections.push_back(Crossing{ i, j, interUVs.u, interUVs.v });
			}
		}
		// we assume there will be either 0 or 2 intersections
		if (intersections.size() == 2) {
			break;
		}
	}

	auto newEnd = std::unique(intersections.begin(), intersections.end(), [&](const Crossing& a, const Crossing& b) {
		return Helper::AreEqualF(a.u, b.u, FZERO_UV) && Helper::AreEqualF(a.v, b.v, FZERO_UV);
	});
	intersections.erase(newEnd, intersections.end());

	if (intersections.size() == 0) { return; } // the intersection line is outside the milling zone anyway

	size_t& i1 = intersections[0].i;
	size_t& i2 = intersections[1].i;

	size_t& j1 = intersections[0].j;
	size_t& j2 = intersections[1].j;

	gmod::vector3<double> pointAtI1J1 = part.s->Point(intersections[0].u, intersections[0].v);
	gmod::vector3<double> normAtI1J1 = part.s->Normal(intersections[0].u, intersections[0].v);
	gmod::vector3<double> pointAtI2J2 = part.s->Point(intersections[1].u, intersections[1].v);
	gmod::vector3<double> normAtI2J2 = part.s->Normal(intersections[1].u, intersections[1].v);

	InterPoint ipI1J1 = {
			gmod::vector3<float>(pointAtI1J1.x(), pointAtI1J1.y(), pointAtI1J1.z()),
			gmod::vector3<float>(normAtI1J1.x(), normAtI1J1.y(), normAtI1J1.z()),
			intersections[0].u, intersections[0].v, part.s
	};

	InterPoint ipI2J2 = {
			gmod::vector3<float>(pointAtI2J2.x(), pointAtI2J2.y(), pointAtI2J2.z()),
			gmod::vector3<float>(normAtI2J2.x(), normAtI2J2.y(), normAtI2J2.z()),
			intersections[1].u, intersections[1].v, part.s
	};

	// take the inside part
	std::vector<StageThree::InterPoint> j1j2;
	j1j2.reserve(std::abs(static_cast<long long>(j2) - static_cast<long long>(j1)) + 2);
	j1j2.push_back(ipI1J1);
	if (j1 < j2) {
		for (size_t k = j1; k <= j2; k++) {
			j1j2.push_back(intersectionLine[k]);
		}
	} else {
		for (size_t k = j1; k >= j2; k--) {
			j1j2.push_back(intersectionLine[k]);
		}
	}
	j1j2.push_back(ipI2J2);

	std::vector<StageThree::InterPoint> firstContour; // 0:i1 + j1:j2 + i2:n
	std::vector<StageThree::InterPoint> secondContour; // i1:i2 + j2:j1

	firstContour.reserve(i1 + 1 + j1j2.size() + (n - i2));

	// 0:i1
	for (size_t k = 0; k <= i1; k++) {
		firstContour.push_back(finalContour[k]);
	}
	// j1:j2 
	firstContour.insert(firstContour.end(), j1j2.begin(), j1j2.end());
	// i2:n
	for (size_t k = i2; k < n; k++) {
		firstContour.push_back(finalContour[k]);
	}

	secondContour.reserve((i2 - i1 + 1) + j1j2.size());

	// i1:i2
	for (size_t k = i1; k <= i2; k++) {
		secondContour.push_back(finalContour[k]);
	}
	// j2:j1
	for (auto it = j1j2.rbegin(); it != j1j2.rend(); ++it) {
		secondContour.push_back(*it);
	}

	if (IsInside(PartUVs{ insideU , insideV }, firstContour)) {
		finalContour = firstContour;
	} else {
		finalContour = secondContour;
	}
}

SegmentGraph StageThree::CutSurfaceIntoGraph(Intersection& intersection, const std::vector<InterPoint>& contour,
	const Intersection::IDIG& part, float epsilon, bool cutVertical, int startFrom, SegmentEnd3& startingPoint) const {

	// find starting point
	// create vertical plane
	// moving by sep value (use epsilon) from starting point (either vertically or horizontally, use cutVertical) find intersections with the part
	// find which parts of that intersection are inside the contour and create segments from them
	// you should aquire a list of segment ends (ordered by index on contour) and a list of inner segements
	// create list of contour segments and use both to create a graph

	const float separation = diameter - epsilon * m_radius;

	// == starting point and move vector ==
	float minX = centre.x();
	long minXidx = 0;
	float maxX = centre.x();
	long maxXidx = 0;
	float minZ = centre.z();
	long minZidx = 0;
	float maxZ = centre.z();
	long maxZidx = 0;
	for (long i = 0; i < contour.size(); i++) {
		const auto& p = contour[i];
		if (p.pos.x() < minX) {
			minX = p.pos.x();
			minXidx = i;
		}
		if (p.pos.x() > maxX) {
			maxX = p.pos.x();
			maxXidx = i;
		}
		if (p.pos.z() < minZ) {
			minZ = p.pos.z();
			minZidx = i;
		}
		if (p.pos.z() > maxZ) {
			maxZ = p.pos.z();
			maxZidx = i;
		}
	}

	int ID = 0;
	startingPoint = SegmentEnd3{ .id = ID };
	ID++;

	float startVal = 0.f, endVal = 0.f, step = 0.f;
	switch (startFrom) {
		case 1: { // top->bottom
			startVal = minZ;
			endVal = maxZ;
			step = separation;
			startingPoint.contourIdx = minZidx;
			break;
		} case 2: { // right->left
			startVal = maxX;
			endVal = minX;
			step = -separation;
			startingPoint.contourIdx = maxXidx;
			break;
		} case 3: { // bottom->top
			startVal = maxZ;
			endVal = minZ;
			step = -separation;
			startingPoint.contourIdx = maxZidx;
			break;
		} case 4: { // left->right
			startVal = minX;
			endVal = maxX;
			step = separation;
			startingPoint.contourIdx = minXidx;
			break;
		}
	}
	// =====

	const float baseOffset = baseY; // TODO : +m_radius;
	float knifeWidth = std::max(width, length);
	float knifeHeight = totalHeight - baseOffset + m_radius;
	float knifeY = (totalHeight + baseOffset) / 2.f;

	Surface::Plane knife;
	if (cutVertical) { // parallel to YZ
		knife = Surface::MakePlane(centre, knifeHeight, knifeWidth, gmod::vector3<double>(0, 0, 90), -420);
	} else { // parallel to XY
		knife = Surface::MakePlane(centre, knifeWidth, knifeHeight, gmod::vector3<double>(90, 0, 0), -420);
	}

	std::vector<Segment3> innerSegements;
	std::vector<SegmentEnd3> contourIntersections;

	float currVal = startVal + step;
	while (currVal < endVal) {
		if (cutVertical) {
			knife.surface->SetTranslation(currVal, knifeY, centre.z());
		} else {
			knife.surface->SetTranslation(centre.x(), knifeY, currVal);
		}

		Intersection::IDIG knifeIDIG = { knife.surface->id, dynamic_cast<IGeometrical*>(knife.surface.get()) };

		unsigned int res = intersection.FindIntersection(std::make_pair(part, knifeIDIG));
		if (res != 0) { 
			throw std::runtime_error("Should have found intersection, but didn't. Evaluate params.");
		}

		auto& pointsOfIntersection = intersection.GetPointsOfIntersection();
		std::vector<StageThree::InterPoint> intersectionLine(pointsOfIntersection.size());
		std::transform(pointsOfIntersection.begin(), pointsOfIntersection.end(), intersectionLine.begin(),
			[&part](const Intersection::PointOfIntersection& p) {
			auto normal = part.s->Normal(p.uvs.u1, p.uvs.v1);
			return StageThree::InterPoint{
				.pos = gmod::vector3<float>(p.pos.x(), p.pos.y(), p.pos.z()),
				.norm = gmod::vector3<float>(normal.x(), normal.y(), normal.z()),
				.u = static_cast<float>(p.uvs.u1),
				.v = static_cast<float>(p.uvs.v1),
				.surf = part.s
			};
		});

		GetInnerSegments(contour, intersectionLine, innerSegements, contourIntersections, ID, part);
		currVal += step;
	}

	contourIntersections.push_back(startingPoint); // add the additonal starting point to graph
	std::sort(contourIntersections.begin(), contourIntersections.end(), [](const SegmentEnd3& se1, const SegmentEnd3& se2) {
		return se1.contourIdx < se2.contourIdx;
	});

	std::vector<Segment3> contourSegements;
	contourSegements.reserve(2 * contourIntersections.size());
	for (int j = 0; j < contourIntersections.size(); j++) {
		int nextJ = (j + 1) % contourIntersections.size();

		Segment3 segForward = {
			.p1 = contourIntersections[j],
			.p2 = contourIntersections[nextJ],
			.contourSegment = true
		};
		contourSegements.push_back(segForward);

		Segment3 segBackward = {
			.p1 = contourIntersections[nextJ],
			.p2 = contourIntersections[j],
			.contourSegment = true
		};
		contourSegements.push_back(segBackward);
	}

	return SegmentGraph(innerSegements, contourSegements);
}

void StageThree::GetInnerSegments(const std::vector<InterPoint>& contour, const std::vector<InterPoint>& intersectionLine,
	std::vector<Segment3>& innerSegements, std::vector<SegmentEnd3>& contourIntersections, int& ID, const Intersection::IDIG& part) const {

	struct Crossing {
		long i, j;
		float u, v;
	};
	std::vector<Crossing> intersections;

	int n = contour.size();
	int m = intersectionLine.size();

	for (long j = 0; j < m; j++) {
		auto& A = intersectionLine[j];
		long nextJ = (j + 1) % m;
		auto& B = intersectionLine[nextJ];

		for (long i = 0; i < n; i++) {
			auto& C = contour[i];
			long nextI = (i + 1) % n;
			auto& D = contour[nextI];

			PartUVs interUVs;
			if (DoSegementsCross({ A.u, A.v }, { B.u, B.v }, { C.u, C.v }, { D.u, D.v }, interUVs)) {
				intersections.push_back(Crossing{ i, j, interUVs.u, interUVs.v });\
				break;
			}
		}
	}

	auto newEnd = std::unique(intersections.begin(), intersections.end(), [&](const Crossing& a, const Crossing& b) {
		return Helper::AreEqualF(a.u, b.u, FZERO_UV) && Helper::AreEqualF(a.v, b.v, FZERO_UV);
	});
	intersections.erase(newEnd, intersections.end());

	for (int k = 0; k < intersections.size() - 1; k++) {
		int nextK = k + 1;
		int midJ = (intersections[k].j + intersections[nextK].j) / 2;
		const InterPoint& midpoint = intersectionLine[midJ];

		if (IsInside(PartUVs{ midpoint.u, midpoint.v }, contour)) {
			SegmentEnd3 start = {
				.id = ID,
				.contourIdx = intersections[k].i
			};
			ID++;
			contourIntersections.push_back(start);

			SegmentEnd3 end = {
				.id = ID,
				.contourIdx = intersections[nextK].i
			};
			ID++;
			contourIntersections.push_back(end);

			std::vector<gmod::vector3<float>> innerPoints;
			innerPoints.reserve(intersections[nextK].j - intersections[k].j + 1);

			auto p1 = part.s->Point(intersections[k].u, intersections[k].v);
			innerPoints.push_back(gmod::vector3<float>(p1.x(), p1.y(), p1.z()));

			for (int t = intersections[k].j + 1; t < intersections[nextK].j - 1; t++) {
				const InterPoint& innerPoint = intersectionLine[t];

				auto p = part.s->Point(innerPoint.u, innerPoint.v);
				innerPoints.push_back(gmod::vector3<float>(p.x(), p.y(), p.z()));
			}

			auto p2 = part.s->Point(intersections[nextK].u, intersections[nextK].v);
			innerPoints.push_back(gmod::vector3<float>(p2.x(), p2.y(), p2.z()));

			Segment3 segForward = {
				.p1 = start,
				.p2 = end,
				.contourSegment = false,
				.p1p2 = innerPoints
			};
			innerSegements.push_back(segForward);

			std::reverse(innerPoints.begin(), innerPoints.end());
			Segment3 segBackward = {
				.p1 = end,
				.p2 = start,
				.contourSegment = false,
				.p1p2 = innerPoints
			};
			innerSegements.push_back(segBackward);
		}
	}
}

bool StageThree::DoSegementsCross(PartUVs A, PartUVs B, PartUVs C, PartUVs D, PartUVs& intersection) const {
	//if (std::max(A.u, B.u) < std::min(C.u, D.u) ||
	//	std::max(C.u, D.u) < std::min(A.u, B.u) ||
	//	std::max(A.v, B.v) < std::min(C.v, D.v) ||
	//	std::max(C.v, D.v) < std::min(A.v, B.v)) {
	//	return false;
	//}

	//// 2D cross for vectors OA and OB
	//auto cross = [](const PartUVs& O, const PartUVs& A, const PartUVs& B) -> float {
	//	return (A.u - O.u) * (B.v - O.v) - (A.v - O.v) * (B.u - O.u);
	//};

	//float d1 = cross(A, B, C);
	//float d2 = cross(A, B, D);
	//float d3 = cross(C, D, A);
	//float d4 = cross(C, D, B);

	//if (Helper::AreEqualF(d1, 0, FZERO_UV)) { // C is on AB
	//	intersection = C;
	//	return true;
	//}
	//if (Helper::AreEqualF(d2, 0, FZERO_UV)) { // D is on AB
	//	intersection = D;
	//	return true;
	//}
	//if (Helper::AreEqualF(d3, 0, FZERO_UV)) { // A is on CD
	//	intersection = A;
	//	return true;
	//}
	//if (Helper::AreEqualF(d4, 0, FZERO_UV)) { // B is on CD
	//	intersection = B;
	//	return true;
	//}

	//if ((d1 * d2 < 0) && (d3 * d4 < 0)) {
	//	float denominator = (A.u - B.u) * (C.v - D.v) - (A.v - B.v) * (C.u - D.u);
	//	float t = ((A.u - C.u) * (C.v - D.v) - (A.v - C.v) * (C.u - D.u)) / denominator;

	//	intersection.u = A.u + t * (B.u - A.u);
	//	intersection.v = A.v + t * (B.v - A.v);
	//	return true;
	//}

	//return false;

	const PartUVs p = A;
	const PartUVs r = { B.u - A.u, B.v - A.v };
	const PartUVs q = C;
	const PartUVs s = { D.u - C.u, D.v - C.v };

	const float r_s = r.u * s.v - r.v * s.u; // cross2D equivalent
	if (Helper::AreEqualF(r_s, 0.f, FZERO_UV)) { return false; }

	const PartUVs q_p = { q.u - p.u, q.v - p.v };
	const float t = (q_p.u * s.v - q_p.v * s.u) / r_s; // cross2D(q_p, s)
	const float u = (q_p.u * r.v - q_p.v * r.u) / r_s; // cross2D(q_p, r)

	if (t >= -FZERO_UV && t <= 1 + FZERO_UV && u >= -FZERO_UV && u <= 1 + FZERO_UV) {
		intersection = PartUVs{ p.u + t * r.u, p.v + t * r.v };
		return true;
	}
	return false;
}

bool StageThree::IsInside(const PartUVs& point, const std::vector<StageThree::InterPoint>& closedContour) const {
	int n = closedContour.size();
	if (n < 3) { return 0; };

	int winding = 0;
	for (int i = 0; i < n; i++) {
		int j = (i + 1) % n;
		const StageThree::InterPoint& vi = closedContour[i];
		const StageThree::InterPoint& vj = closedContour[j];

		float cross = (vj.u - vi.u) * (point.v - vi.v) - (vj.v - vi.v) * (point.u - vi.u);

		// check if edge crosses upward
		if (vi.v < point.v) {
			if (vj.v > point.v && cross > 0) {
				winding++;
			}
		}
		// check if edge crosses downward
		else {
			if (vj.v < point.v && cross < 0) {
				winding--;
			}
		}
	}
	return winding != 0;
}
