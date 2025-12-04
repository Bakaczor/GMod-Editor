#include "StageThree.h"
#include "Surface.h"
#include "IGeometrical.h"
#include "Helper.h"
#include "OffsetSurface.h"

using namespace app;

std::vector<gmod::vector3<float>> StageThree::GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const {
	gmod::vector3<double> baseCentre(centre.x(), centre.y() + m_radius, centre.z());
	Surface::Plane base = Surface::MakePlane(baseCentre, width, length, true, -69);
	Intersection::IDIG baseIDIG = { base.surface->id, dynamic_cast<IGeometrical*>(base.surface.get()) };

	std::vector<gmod::vector3<float>> path;
	path.push_back(gmod::vector3<float>(0, totalHeight, 0));
	for (const auto& params : m_millingParams) {
		auto elementsPath = GeneratePathForPart(sceneObjects, intersection, params, baseIDIG);
		std::copy(elementsPath.begin(), elementsPath.end(), std::back_inserter(path));
	}

	return path;
}

std::vector<gmod::vector3<float>> StageThree::GeneratePathForPart(
	const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection,
	const MillingPartParams& params, const Intersection::IDIG& base) const {

	std::vector<std::pair<Intersection::IDIG, Intersection::InterParams>> surfaces;
	std::vector<std::unique_ptr<OffsetSurface>> offsetSurfaceStorage; // the offset surfaces must exist until we exit the function

	// =====
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
			addZ = -m_radius;
			break;
		} case 2: {
			addX = m_radius;
			break;
		} case 3: {
			addZ = m_radius;
			break;
		} case 4: {
			addX = -m_radius;
			break;
		}
	}
	// =====

	std::vector<gmod::vector3<float>> fullPath;
	long long firstIdx = G.vertices3[path.front()].segEnd.contourIdx;
	gmod::vector3<float> first = contour[firstIdx].pos;
	fullPath.push_back(gmod::vector3<float>(first.x() + addX, totalHeight, first.z() + addZ));
	fullPath.push_back(gmod::vector3<float>(first.x() + addX, first.y(), first.z() + addZ));

	// TODO
	// loop through the path
	// if segement contour, take appropriate positions from contour, else from the segment itself (in correct order!)

	long long lastIdx = G.vertices3[path.back()].segEnd.contourIdx;
	gmod::vector3<float> last = contour[lastIdx].pos;
	fullPath.push_back(last);
	fullPath.push_back(gmod::vector3<float>(last.x(), totalHeight, last.z()));

	return fullPath;
}

std::vector<StageThree::InterPoint> StageThree::FindContour(Intersection& intersection,
	const std::vector<InterPoint>& baseContour, const gmod::vector3<float>& insidePoint,
	const Intersection::IDIG& part, const std::vector<std::pair<Intersection::IDIG, Intersection::InterParams>>& intersectingSurfaces) const {

	// TODO : find UVs for the insidePoint (closest x and z above baseY), like in stage one
	float insideU = 0;
	float insideV = 0;

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

		Combine(finalContour, intersectionLine, insideU, insideV);
	}

	return finalContour;
}

void StageThree::Combine(std::vector<StageThree::InterPoint>& finalContour, const std::vector<StageThree::InterPoint>& intersectionLine,
	float insideU, float insideV) const {
	// TODO
	// find two points at which intersectionLine intersects with finalContour
	// take the inside and create two contours
	// check using insidePoint which contour to take and return it
}

SegmentGraph StageThree::CutSurfaceIntoGraph(Intersection& intersection, const std::vector<InterPoint>& contour,
	const Intersection::IDIG& part, float epsilon, bool cutVertical, int startFrom, SegmentEnd3& startingPoint) const {

	// TODO
	// find starting point
	// create vertical plane
	// moving by sep value (use epsilon) from starting point (either vertically or horizontally, use cutVertical) find intersections with the part
	// find which parts of that intersection are inside the contour and create segments from them
	// you should aquire a list of segment ends (ordered by index on contour) and a list of inner segements
	// create list of contour segments and use both to create a graph

	startingPoint = startingPoint;
	std::vector<Segment3> innerSegements;
	std::vector<Segment3> contourSegements;

	return SegmentGraph(innerSegements, contourSegements);
}

bool StageThree::DoSegementsCross(PartUVs A, PartUVs B, PartUVs C, PartUVs D, PartUVs& intersection) const {
	// TODO : find if AB crosses with CD on UV plane and if they do return the intersection UVs

	bool intersect = false;
	if (intersect) {
		intersection = intersection;
	}
	return intersect;
}
