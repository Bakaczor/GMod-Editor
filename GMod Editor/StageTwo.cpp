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

	std::unordered_map<float, std::vector<SegmentEnd>> xValIntersectionPoints;
	for (const float& xVal : xValues) {
		xValIntersectionPoints.insert({ xVal, std::vector<SegmentEnd>() });
	}

	float topCountourZ = 0.f;
	int topCountourIdx = 0;

	int ID = 0; // future vertex index
	std::vector<SegmentEnd> contourIntersections;
	for (size_t i = 0; i < offsetContour.size(); i++) {
		if (offsetContour[i].pos.z() > topCountourZ) {
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

				SegmentEnd p = {
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

		std::sort(intersections.begin(), intersections.end(), [](const SegmentEnd& a, const SegmentEnd& b) {
			return a.z > b.z;
		});
	}

	// create vertical segments
	std::vector<std::pair<float, std::vector<Segment>>> verticalSegments;
	for (auto& [xVal, ends] : xValIntersectionPoints) {
		verticalSegments.push_back(std::make_pair(xVal, std::vector<Segment>()));

		auto& segments = verticalSegments.back().second;
		segments.reserve(ends.size() / 2);

		SegmentEnd top = {
			.x = xVal,
			.z = zTop,
			.isTopOrBottom = true,
			.isOnContour = false,
			.id = ID
		};
		ID++;
		segments.push_back(Segment{ top, ends.front() });

		for (int i = 1; i < ends.size() - 1; i += 2) {
			segments.push_back(Segment{ ends[i], ends[i + 1] });
		}

		SegmentEnd bottom = {
			.x = xVal,
			.z = zBottom,
			.isTopOrBottom = true,
			.isOnContour = false,
			.id = ID
		};
		ID++;
		segments.push_back(Segment{ ends.back(), bottom});
	}

	// make sure vertical segments are sorted in regard to x
	std::sort(verticalSegments.begin(), verticalSegments.end(),
		[](const std::pair<float, std::vector<Segment>>& a, const std::pair<float, std::vector<Segment>>& b) {
		return a.first < b.first;
	});

	// create contour segements
	std::vector<Segment> contourSegements;
	contourSegements.reserve(contourIntersections.size());
	for (size_t i = 0; i < contourIntersections.size(); i++) {
		size_t idx1 = i;
		size_t idx2 = (i + 1) % contourIntersections.size();

		Segment seg = {
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
	std::vector<std::pair<Intersection::IDIG, StageTwoParams>> sceneSurfaces;

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
	Surface::Plane base = Surface::MakePlane(centre, width, length, true, -69);
	Intersection::IDIG baseIDIG = { base.surface->id, dynamic_cast<IGeometrical*>(base.surface.get()) };

	const gmod::vector3<float> planeNormal = { 0, 1, 0 };
	std::vector<StageTwo::InterPoint> offsetCountour;
	for (auto& [surf, params] : sceneSurfaces) {
		intersection.SetIntersectionParameters(params.interParams);
		unsigned int res = intersection.FindIntersection(std::make_pair(surf, baseIDIG));
		if (res != 0) { 
			throw std::runtime_error("Should have found intersection, but didn't. Evaluate params.");
		}

		auto& pointsOfIntersection = intersection.GetPointsOfIntersection();
		std::vector<StageTwo::InterPoint> thisOffsetCountour;
		thisOffsetCountour.reserve(pointsOfIntersection.size());

		for (const auto& poi : pointsOfIntersection) {
			InterPoint offsetPoi{ .surf = surf.s };

			gmod::vector3<double> normal1 = surf.s->Normal(poi.uvs.u1, poi.uvs.v1);
			if (Helper::AreEqualV3(normal1, planeNormal, FZERO)) { // first set of UVs belongs to the base plane
				gmod::vector3<double> normal2 = surf.s->Normal(poi.uvs.u2, poi.uvs.v2);
				gmod::vector3<double> offsetPos = poi.pos + normal2 * m_radius;

				offsetPoi.pos = gmod::vector3<float>(offsetPos.x(), baseY, offsetPos.z());
				offsetPoi.norm = gmod::vector3<float>(normal2.x(), 0, normal2.z());
				offsetPoi.u = poi.uvs.u2;
				offsetPoi.v = poi.uvs.v2;
			} else {
				gmod::vector3<double> offsetPos = poi.pos + normal1 * m_radius;

				offsetPoi.pos = gmod::vector3<float>(offsetPos.x(), baseY, offsetPos.z());
				offsetPoi.norm = gmod::vector3<float>(normal1.x(), 0, normal1.z());
				offsetPoi.u = poi.uvs.u1;
				offsetPoi.v = poi.uvs.v1;
			}

			thisOffsetCountour.push_back(offsetPoi);
		}

		// TODO: here you could add distance and curve based filtration on thisOffsetCountour
		// basically, if the next point is within given precision and has very similiar normal (1 or 2 degrees difference max) you can skip it
		EnsureClockwiseOrder(thisOffsetCountour);
		Combine(offsetCountour, thisOffsetCountour, params.combineParams);
	}

	return offsetCountour;
}

void StageTwo::Combine(std::vector<StageTwo::InterPoint>& mainContour, const std::vector<StageTwo::InterPoint>& newContour, CombineParams combineParams) const {
	if (newContour.empty()) { return; }

	if (mainContour.empty()) {
		mainContour = newContour;
		return;
	}

	if (combineParams.expectedIntersections < 2) { return; }

	struct InterPair {
		size_t i;
		size_t j;
		float dist;
	};

	size_t startI = -1;
	float bestDistToStart = width;

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

		float distToStart = (mainContour[i].pos - combineParams.startPoint).length();
		if (distToStart < bestDistToStart) {
			bestDistToStart = i;
			bestDistToStart = distToStart;
		}
	}

	std::sort(pairs.begin(), pairs.end(), [](const InterPair& a, const InterPair& b) {
		return a.dist < b.dist;
	});

	// TODO: it is possbile that there will be need to make sure they are not to close to each other
	// it depends how dense contours will be
	std::vector<InterPair> intersections(pairs.begin(), pairs.begin() + combineParams.expectedIntersections);

	std::sort(intersections.begin(), intersections.end(), [](const InterPair& a, const InterPair& b) {
		return a.i < b.i;
	});
	// TODO
	// find between which intersection is startI, put intersections in order startI -> i_j ... -> i_n -> i0 -> ... -> i_k -> startI
	// add maincontour[startI->i_j] to result
	// starting from i_j intersection start on newcontour
	// put everything between i_j - i_j+1 to result
	// switch to main and so on until i_k
	// then add maincontour[i_k->startI]
	// when switching there are two options to follow (increasing or decreasing index)
	// it should be increasing index if the contours are ordered correctly, but make sure using normals 
	// (pick the right direction in regard to normal from the contour you switch to)
}

std::vector<gmod::vector3<float>> StageTwo::GetFinalPath(const SegmentGraph& G, const SegmentEnd& start, 
	const std::vector<StageTwo::InterPoint>& offsetContour, int topContourIdx, float zTop) const {

	std::vector<int> path = G.SpecialDFS(start.id);
	
	gmod::vector3<float> startPoint(0, totalHeight + 1.0f, 0);
	gmod::vector3<float> overMillingStart(start.x, totalHeight + 1.0f, start.z);

	std::vector<gmod::vector3<float>> finalPath;

	finalPath.push_back(startPoint);
	finalPath.push_back(overMillingStart);

	for (int v = 0; v < path.size() - 1; v++) {
		int fromId = path[v];
		int toId = path[v + 1];

		const auto& fromVertex = G.vertices[fromId];
		const auto& toVertex = G.vertices[toId];

		auto it = std::find_if(fromVertex.neighbours.begin(), fromVertex.neighbours.end(), [&toId](const std::pair<int, SegmentGraph::Edge>& e) {
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

	const SegmentEnd& last = G.vertices[path.back()].segEnd;
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

	const auto& contourEnd = offsetContour.back().pos;
	finalPath.push_back(gmod::vector3<float>(contourEnd.x(), totalHeight + 1.0f, contourEnd.z()));
	finalPath.push_back(startPoint);

	return finalPath;
}

void StageTwo::EnsureClockwiseOrder(std::vector<StageTwo::InterPoint>& points) const {
	if (points.size() < 3) return;

	const auto& p1 = points[0].pos;
	const auto& p2 = points[1].pos;
	const auto& p3 = points[2].pos;

	float area = (p2.x() - p1.x()) * (p3.z() - p1.z()) - (p2.z() - p1.z()) * (p3.x() - p1.x());

	// if area is positive, points are counterclockwise -> reverse
	if (area > 0) {
		std::reverse(points.begin(), points.end());
	}
}


// for part 3 (initial idea, read lectures before that)
// for every surface find offset surface
// for every offset surface find intersection with other surfaces and create bounded parametric surface (make it specific)
// using vertical plane, cut find intersections with milling surface and constrain them using bounds found before
// you will get milling parts like in part two
// try to create paths the same way as in part two (this is basically the same, but we need to have intersections, because here y changes)
// it should be very easy, except for face which has holes and will need special treatment