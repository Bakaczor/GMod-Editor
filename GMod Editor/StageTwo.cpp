#include "StageTwo.h"
#include "Surface.h"
#include "IGeometrical.h"
#include "Helper.h"

using namespace app;

// create base plane
// for every surface, find intersection with base (may need to use specific parameters for each surface)
// calculate normals and get outer rim (distance of radius)
// combine outer rims into one outer rim, using winding (intersection has uv-s so it is possible to calculate normals) - hard
// need to check and solve loops if they happen - harder
// that's one part of the path
// the second one is crated similarly to before
// for each xVal find for how many zVals they correspond to (should be pairs, so either 0, 2, 4 etc.) 
// this creates milling parts that needs to be traverse
// try to traverse them safe and possibly efficient

std::vector<gmod::vector3<float>> StageTwo::GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	// calculate boundaries 
	const float xLeft = topLeftCorner.x() - m_radius - 1.0f;
	const float xRight = topLeftCorner.x() + width + m_radius + 1.0f;

	// with respect to radius
	const float zTop = topLeftCorner.z() - m_radius - 1.0f;
	const float zBottom = topLeftCorner.z() + length + m_radius + 1.0f;

	const float separation = diameter - epsilon * m_radius;

	// calculate x values
	std::vector<float> xValues;
	float xCurr = xLeft;
	while (xCurr < xRight) {
		xValues.push_back(xCurr);
		xCurr += separation;
	}
	xValues.push_back(xCurr); // last additional

	auto offsetContour = CreateOffsetContour(sceneObjects, intersection);

	std::unordered_map<float, std::vector<SegmentEnd2>> xValIntersectionPoints;
	for (const float& xVal : xValues) {
		xValIntersectionPoints.insert({ xVal, std::vector<SegmentEnd2>() });
	}

	float topCountourZ = 0.f;
	int topCountourIdx = 0;

	int ID = 0; // future vertex index
	std::vector<SegmentEnd2> contourIntersections;
	for (size_t i = 0; i < offsetContour.size(); i++) {
		if (offsetContour[i].pos.z() < topCountourZ) {
			topCountourZ = offsetContour[i].pos.z();
			topCountourIdx = i;
		}

		long long idx1 = i;
		long long idx2 = (i + 1) % offsetContour.size();

		const auto& p1 = offsetContour[idx1];
		const auto& p2 = offsetContour[idx2];

		for (const float& xVal : xValues) {
			// check if this segment crosses any xVal line
			if ((p1.pos.x() <= xVal && p2.pos.x() >= xVal) || (p1.pos.x() >= xVal && p2.pos.x() <= xVal)) {

				// calculate intersection point (linear interpolation)
				float t = (xVal - p1.pos.x()) / (p2.pos.x() - p1.pos.x());
				float z = p1.pos.z() + t * (p2.pos.z() - p1.pos.z());

				SegmentEnd2 p = {
					.x = xVal,
					.z = z,
					.isTopOrBottom = false,
					.isOnContour = true,
					.id = ID,
					.prevIdx = idx1,
					.nextIdx = idx2
				};
				ID++;

				contourIntersections.push_back(p);
				xValIntersectionPoints[xVal].push_back(p);
				break; 
			}
		}
	}

	// verify vertical intersections and sort them
	for (const float& xVal : xValues) {
		auto& intersections = xValIntersectionPoints.at(xVal);

		if (intersections.size() % 2 != 0) {
			throw std::runtime_error("The number of points for each xVal should be even.");
		}

		std::sort(intersections.begin(), intersections.end(), [](const SegmentEnd2& a, const SegmentEnd2& b) {
			return a.z < b.z;
		});
	}

	// create vertical segments
	std::vector<std::pair<float, std::vector<Segment2>>> verticalSegments;
	for (auto& [xVal, ends] : xValIntersectionPoints) {
		verticalSegments.push_back(std::make_pair(xVal, std::vector<Segment2>()));

		SegmentEnd2 top = {
			.x = xVal,
			.z = zTop,
			.isTopOrBottom = true,
			.isOnContour = false,
			.id = ID
		};
		ID++;
		SegmentEnd2 bottom = {
			.x = xVal,
			.z = zBottom,
			.isTopOrBottom = true,
			.isOnContour = false,
			.id = ID
		};
		ID++;

		auto& segments = verticalSegments.back().second;
		if (!ends.empty()) {
			segments.push_back(Segment2{ top, ends.front() });
			for (int i = 1; i < ends.size() - 1; i += 2) {
				segments.push_back(Segment2{ ends[i], ends[i + 1] });
			}
			segments.push_back(Segment2{ ends.back(), bottom });
		} else {
			segments.push_back(Segment2{ top, bottom });
		}
	}

	// make sure vertical segments are sorted in regard to x
	std::sort(verticalSegments.begin(), verticalSegments.end(),
		[](const std::pair<float, std::vector<Segment2>>& a, const std::pair<float, std::vector<Segment2>>& b) {
		return a.first < b.first;
	});

	// create contour segements
	std::vector<Segment2> contourSegements;
	contourSegements.reserve(contourIntersections.size());
	for (size_t i = 0; i < contourIntersections.size(); i++) {
		size_t idx1 = i;
		size_t idx2 = (i + 1) % contourIntersections.size();

		Segment2 seg = {
			.p1 = contourIntersections[idx1],
			.p2 = contourIntersections[idx2],
			.interStartIdx = contourIntersections[idx1].nextIdx,
			.interEndIdx = contourIntersections[idx2].prevIdx
		};

		contourSegements.push_back(seg);
	}

	SegmentGraph G(verticalSegments, contourSegements, ID);
	return GetFinalPath(G, verticalSegments.front().second.front().p1, offsetContour, topCountourIdx, zTop);
}

std::vector<StageTwo::InterPoint> StageTwo::CreateOffsetContour(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	// get all surfaces on scene
	std::vector<std::pair<Intersection::IDIG, Intersection::InterParams>> sceneSurfaces;

	for (const auto& so : sceneObjects) {
		if (m_contourSurfaces.contains(so->name)) {
			IGeometrical* g = dynamic_cast<IGeometrical*>(so.get());
			if (g != nullptr) {
				sceneSurfaces.push_back(std::make_pair(
					Intersection::IDIG { so->id, g },
					m_contourSurfaces.at(so->name))
				);
			}
		}
	}

	// create base
	Surface::Plane base = Surface::MakePlane(centre, width, length, { 0,0,0 }, -69);
	Intersection::IDIG baseIDIG = { base.surface->id, dynamic_cast<IGeometrical*>(base.surface.get()) };

	std::vector<StageTwo::InterPoint> offsetCountour;
	for (auto& [surf, params] : sceneSurfaces) {
		intersection.SetIntersectionParameters(params);
		unsigned int res = intersection.FindIntersection(std::make_pair(surf, baseIDIG));
		if (res != 0) { 
			throw std::runtime_error("Should have found intersection, but didn't. Evaluate params.");
		}

		auto& pointsOfIntersection = intersection.GetPointsOfIntersection();
		std::vector<StageTwo::InterPoint> thisOffsetCountour;
		thisOffsetCountour.reserve(pointsOfIntersection.size());

		bool closed = intersection.IsClosed();
		int dir = IsCW(pointsOfIntersection) ? 1 : -1;

		size_t start = 0;
		size_t n = pointsOfIntersection.size();

		if (!closed) {
			start = 1;
			n = pointsOfIntersection.size() - 1;
		}

		if (!closed) { // first
			InterPoint offsetPoi{ .surf = surf.s };
			const auto diff = pointsOfIntersection[1].pos - pointsOfIntersection[0].pos;
			gmod::vector3<double> normal(-diff.z() * dir, 0, diff.x() * dir);
			normal.normalize();
			gmod::vector3<double> offsetPos = pointsOfIntersection[0].pos + normal * m_radius;

			offsetPoi.pos = gmod::vector3<float>(offsetPos.x(), baseY, offsetPos.z());
			offsetPoi.norm = gmod::vector3<float>(normal.x(), 0, normal.z());
			offsetPoi.u = pointsOfIntersection[0].uvs.u1;
			offsetPoi.v = pointsOfIntersection[0].uvs.v1;

			thisOffsetCountour.push_back(offsetPoi);
		}

		// TODO : think how to smoothen the offset contour - maybe add some intermidiate points?
		// it is little jaggy now
		for (size_t i = start; i < n; i++) {
			size_t prevI = i - 1;
			size_t nextI = i + 1;

			if (closed) {
				if (i == 0) {
					prevI = n - 1;
				}
				if (i == n - 1) {
					nextI = 0;
				}
			}
			const auto& prevP = pointsOfIntersection[prevI];
			const auto& currP = pointsOfIntersection[i];
			const auto& nextP = pointsOfIntersection[nextI];

			InterPoint offsetPoi{ .surf = surf.s };
			const auto prevCurr = currP.pos - prevP.pos;
			const auto currNext = nextP.pos - currP.pos;
			gmod::vector3<double> normalPrev(-prevCurr.z(), 0, prevCurr.x());
			normalPrev.normalize();
			gmod::vector3<double> normalNext(-currNext.z(), 0, currNext.x());
			normalNext.normalize();
			gmod::vector3<double> normal(dir * (normalPrev.x() + normalNext.x()) / 2, 0, dir * (normalPrev.z() + normalNext.z()) / 2);
			normal.normalize();
			gmod::vector3<double> offsetPos = currP.pos + normal * m_radius;

			offsetPoi.pos = gmod::vector3<float>(offsetPos.x(), baseY, offsetPos.z());
			offsetPoi.norm = gmod::vector3<float>(normal.x(), 0, normal.z());
			offsetPoi.u = currP.uvs.u1;
			offsetPoi.v = currP.uvs.v1;

			thisOffsetCountour.push_back(offsetPoi);
		}

		if (!closed) { // last
			InterPoint offsetPoi{ .surf = surf.s };
			const auto diff = pointsOfIntersection[n].pos - pointsOfIntersection[n - 1].pos;
			gmod::vector3<double> normal(-diff.z() * dir, 0, diff.x() * dir);
			normal.normalize();
			gmod::vector3<double> offsetPos = pointsOfIntersection[n].pos + normal * m_radius;

			offsetPoi.pos = gmod::vector3<float>(offsetPos.x(), baseY, offsetPos.z());
			offsetPoi.norm = gmod::vector3<float>(normal.x(), 0, normal.z());
			offsetPoi.u = pointsOfIntersection[n].uvs.u1;
			offsetPoi.v = pointsOfIntersection[n].uvs.v1;

			thisOffsetCountour.push_back(offsetPoi);
		}

		Combine(offsetCountour, thisOffsetCountour);
	}

	return offsetCountour;
}

void StageTwo::Combine(std::vector<StageTwo::InterPoint>& mainContour, const std::vector<StageTwo::InterPoint>& newContour) const {
	if (newContour.empty()) { return; }

	if (mainContour.empty()) {
		mainContour = newContour;
		return;
	}

	struct InterPair {
		size_t i;
		size_t j;
		float dist;
	};

	bool fromMain = true;
	size_t s = -1;
	float smallestX = centre.x() + width;

	std::vector<InterPair> pairs(mainContour.size());
	for (size_t i = 0; i < mainContour.size(); i++) {

		size_t bestJ = 0;
		float bestDist = width;
		for (size_t j = 0; j < newContour.size(); j++) {
			float dist = (mainContour[i].pos - newContour[j].pos).length();
			if (dist < bestDist) { 
				bestJ = j;
				bestDist = dist;
			}
		}

		pairs[i] = InterPair(i, bestJ, bestDist);

		if (mainContour[i].pos.x() < smallestX) {
			s = i;
			smallestX = mainContour[i].pos.x();
		}
	}

	for (size_t j = 0; j < newContour.size(); j++) {
		if (newContour[j].pos.x() < smallestX) {
			fromMain = false;
			s = j;
			smallestX = newContour[j].pos.x();
		}
	}

	std::sort(pairs.begin(), pairs.end(), [](const InterPair& a, const InterPair& b) {
		return a.dist < b.dist;
	});

	// TODO: it is possbile that there will be need to make sure they are not to close to each other
	// it depends how dense contours will be - yeah, it happens
	// instead of this method, use the method stage 3 - find intersecting segments and remove duplicates
	std::vector<InterPair> intersections(pairs.begin(), pairs.begin() + m_expectedIntersections);

	const std::vector<StageTwo::InterPoint>* contourI = &mainContour;
	const std::vector<StageTwo::InterPoint>* contourJ = &newContour;

	if (!fromMain) {
		contourI = &newContour;
		contourJ = &mainContour;

		size_t temp;
		temp = intersections[0].i;
		intersections[0].i = intersections[0].j;
		intersections[0].j = temp;

		temp = intersections[1].i;
		intersections[1].i = intersections[1].j;
		intersections[1].j = temp;
	}
	// we make sure that s refers to contourI
	 
	if (intersections[0].i > intersections[1].i) {
		InterPair temp = intersections[0];
		intersections[0] = intersections[1];
		intersections[1] = temp;
	}
	// i1 < i2

	size_t& i1 = intersections[0].i;
	size_t& i2 = intersections[1].i;

	size_t& j1 = intersections[0].j;
	size_t& j2 = intersections[1].j;

	size_t in = (j1 + j2) / 2;
	bool takeIn = IsOutside(contourJ->at(in), contourI);

	bool takeInAndj1j2 = takeIn && j1 < j2; // j1:j2
	bool takeInAndj2j1 = takeIn && j1 > j2; // j2:j1
	bool takeOutAndj1j2 = !takeIn && j1 < j2; // 0:j1 + j2:m
	bool takeOutAndj2j1 = !takeIn && j1 > j2; // 0:j2 + j1:m

	std::vector<StageTwo::InterPoint> result;
	if (i1 < s && s < i2) { // i1:i2
		for (size_t k = i1; k <= i2; k++) {
			result.push_back(contourI->at(k));
		}

		if (takeInAndj2j1) { // j2:j1

			// i1:i2 + j2:j1
			for (size_t k = j2; k <= j1; k++) {
				result.push_back(contourJ->at(k));
			}
		}
		if (takeOutAndj1j2) { // 0:j1 + j2:m

			// i1:i2 + j2:m + 0:j1
			for (size_t k = j2; k < contourJ->size(); k++) {
				result.push_back(contourJ->at(k));
			}
			for (size_t k = 0; k <= j1; k++) {
				result.push_back(contourJ->at(k));
			}
		}
	} else { // 0:i1 + i2:n
		for (size_t k = 0; k <= i1; k++) {
			result.push_back(contourI->at(k));
		}

		if (takeInAndj1j2) { // j1:j2

			// 0:i1 + j1:j2 + i2:n
			for (size_t k = j1; k <= j2; k++) {
				result.push_back(contourJ->at(k));
			}
		}
		if (takeOutAndj2j1) { // 0:j2 + j1:m

			// 0:i1 + j1:m + 0:j2 + i2:n
			for (size_t k = j1; k < contourJ->size(); k++) {
				result.push_back(contourJ->at(k));
			}
			for (size_t k = 0; k <= j2; k++) {
				result.push_back(contourJ->at(k));
			}
		}

		for (size_t k = i2; k < contourI->size(); k++) {
			result.push_back(contourI->at(k));
		}
	}

	mainContour = result;
}

std::vector<gmod::vector3<float>> StageTwo::GetFinalPath(const SegmentGraph& G, const SegmentEnd2& start, 
	const std::vector<StageTwo::InterPoint>& offsetContour, int topContourIdx, float zTop) const {

	std::vector<int> path = G.SpecialDFS2(start.id);
	
	gmod::vector3<float> startPoint(0, totalHeight + 1.0f, 0);
	gmod::vector3<float> overMillingStart(start.x, totalHeight + 1.0f, start.z);

	std::vector<gmod::vector3<float>> finalPath;

	finalPath.push_back(startPoint);
	finalPath.push_back(overMillingStart);

	for (int v = 0; v < path.size() - 1; v++) {
		int fromId = path[v];
		int toId = path[v + 1];

		const auto& fromVertex = G.vertices2[fromId];
		const auto& toVertex = G.vertices2[toId];

		auto it = std::find_if(fromVertex.neighbours.begin(), fromVertex.neighbours.end(), [&toId](const std::pair<int, SegmentGraph::Edge2>& e) {
			return e.first == toId;
		});

		if (it == fromVertex.neighbours.end()) {
			throw std::runtime_error("There is an error in created path - edge not found.");
		}

		auto currPos = gmod::vector3<float>(fromVertex.segEnd.x, baseY, fromVertex.segEnd.z);
		finalPath.push_back(currPos);

		const auto& edge = it->second;
		if (!edge.isVertical && !edge.isHorizontal) { // if we have contour edge, we need to add the points from contour
			int start = edge.seg.interStartIdx;
			int end = edge.seg.interEndIdx;

			auto& startPos = offsetContour[start].pos;
			auto& endPos = offsetContour[end].pos;
			// determine on which side of edge are we now
			if ((currPos - startPos).length() > (currPos - endPos).length()) {
				std::swap(start, end);
			};

			if (start < end) {
				for (int j = start; j <= end; j++) {
					finalPath.push_back(offsetContour[j].pos);
				}
			} else {
				for (int j = start; j >= end; j--) {
					finalPath.push_back(offsetContour[j].pos);
				}
			}
		} 
	}

	const SegmentEnd2& last = G.vertices2[path.back()].segEnd;
	finalPath.push_back(gmod::vector3<float>(last.x, baseY, last.z));
	finalPath.push_back(gmod::vector3<float>(last.x, totalHeight + 1.0f, last.z));

	// now go around contour
	const auto& contourStart = offsetContour[topContourIdx].pos;
	finalPath.push_back(gmod::vector3<float>(contourStart.x(), totalHeight + 1.0f, zTop));
	finalPath.push_back(gmod::vector3<float>(contourStart.x(), baseY, zTop));

	for (int j = 0; j < offsetContour.size(); j++) {
		int currIdx = (j + topContourIdx) % offsetContour.size();
		finalPath.push_back(offsetContour[currIdx].pos);
	}

	auto& back = finalPath.back();
	finalPath.push_back(gmod::vector3<float>(back.x(), totalHeight + 1.0f, back.z()));
	finalPath.push_back(startPoint);

	return finalPath;
}

bool StageTwo::IsOutside(const StageTwo::InterPoint& point, const std::vector<StageTwo::InterPoint>* contour) const {
	int n = contour->size();
	if (n < 3) { return 0; };

	int winding = 0;
	for (int i = 0; i < n; i++) {
		int j = (i + 1) % n;
		const StageTwo::InterPoint& vi = contour->at(i);
		const StageTwo::InterPoint& vj = contour->at(j);

		float cross = (vj.pos.x() - vi.pos.x()) * (point.pos.z() - vi.pos.z()) 
			- (vj.pos.z() - vi.pos.z()) * (point.pos.x() - vi.pos.x());

		// check if edge crosses upward
		if (vi.pos.z() < point.pos.z()) {
			if (vj.pos.z() > point.pos.z() && cross > 0) {
				winding++; 
			}
		}
		// check if edge crosses downward
		else {
			if (vj.pos.z() < point.pos.z() && cross < 0) {
				winding--; 
			}
		}
	}
	return winding == 0;
}

bool StageTwo::IsCW(const std::vector<Intersection::PointOfIntersection>& contour) const {
	double area = 0;
	for (size_t i = 0; i < contour.size(); i++) {
		const auto& currP = contour[i].pos;
		const auto& nextP = contour[(i + 1) % contour.size()].pos;
		area += (currP.x() * nextP.z() - nextP.x() * currP.z());
	}
	return area < 0;
}

// old, universal idea:
// find between which intersection is startI, put intersections in order startI -> i_j ... -> i_n -> i0 -> ... -> i_k -> startI
// add maincontour[startI->i_j] to result
// starting from i_j intersection start on newcontour
// put everything between i_j - i_j+1 to result
// switch to main and so on until i_k
// then add maincontour[i_k->startI]
// when switching there are two options to follow (increasing or decreasing index)
// it should be increasing index if the contours are ordered correctly, but make sure using normals 
// (pick the right direction in regard to normal from the contour you switch to)