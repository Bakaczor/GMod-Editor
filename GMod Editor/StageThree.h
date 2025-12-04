#pragma once
#include "Object.h"
#include "Intersection.h"
#include "SegmentGraph.h"

namespace app {
	class StageThree {
	public:
		const std::string stage = "3";
		const std::string type = "k";
		const int diameter = 8;

		const float totalHeight = 50.f;

		const float baseY = 15.f;
		const float width = 150.f;
		const float length = 150.f;
		const gmod::vector3<double> topLeftCorner = { -75, baseY, -75 };
		const gmod::vector3<double> centre = { 0, baseY, 0 };

		std::vector<gmod::vector3<float>> GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
	private:
		const float FZERO = 100.f * std::numeric_limits<float>::epsilon();
		const float m_radius = 4.f;

		const Intersection::InterParams m_baseInterParams = {
			.gs = 5 * 1e-3,
			.gt = 5 * 1e-5,
			.gmi = 10000,
			.ns = 0.01,
			.nt = 5 * 1e-2,
			.nmi = 11,
			.nmr = 5,
			.mip = 500,
			.d = 0.5,
			.cpt = 0.5
		};

		struct NamedInterParams {
			std::string name;
			Intersection::InterParams params;
		};

		struct MillingPartParams {
			std::string name; // name of the part
			std::vector<NamedInterParams> intersectingSurfaces; // which surfaces intersect with this part
			gmod::vector3<float> insidePoint; // the point that will be milled
			float epsilon; // used for separation: d - eps * r
			bool cutVertical; // should cut parallel to OZ or OX (horizontal)
			int startFrom; // 1 - top, 2 - right, 3 - bottom, 4 - left
		};

		const std::vector<MillingPartParams> m_millingParams = {
			MillingPartParams{ "bsurface_1", { { "bsurface_2", m_baseInterParams }, { "bsurface_3", m_baseInterParams } }, { 0.f, 0.f, 0.f }, 0.5f, true, 1 }
		};

		struct InterPoint {
			gmod::vector3<float> pos; // offseted
			gmod::vector3<float> norm; // in space
			float u, v;
			const IGeometrical* surf;
		};

		std::vector<gmod::vector3<float>> GeneratePathForPart(
			const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection,
			const MillingPartParams& params, const Intersection::IDIG& base) const;

		std::vector<InterPoint> FindContour(Intersection& intersection,
			const std::vector<InterPoint>& baseContour, const gmod::vector3<float>& insidePoint,
			const Intersection::IDIG& part, const std::vector<std::pair<Intersection::IDIG, Intersection::InterParams>>& intersectingSurfaces) const;

		void Combine(std::vector<StageThree::InterPoint>& finalContour, const std::vector<StageThree::InterPoint>& intersectionLine,
			float insideU, float insideV) const;
		
		SegmentGraph CutSurfaceIntoGraph(Intersection& intersection, const std::vector<InterPoint>& contour,
			const Intersection::IDIG& part, float epsilon, bool cutVertical, int startFrom, SegmentEnd3& startingPoint) const;

		struct PartUVs {
			float u, v;
		};
		bool DoSegementsCross(PartUVs A, PartUVs B, PartUVs C, PartUVs D, PartUVs& intersection) const;
	};
}

// for part 3 (initial idea, read lectures before that)
// for every surface find offset surface
// for every offset surface find intersection with other surfaces and create bounded parametric surface (make it specific)
// using vertical plane, cut find intersections with milling surface and constrain them using bounds found before
// you will get milling parts like in part two
// try to create paths the same way as in part two (this is basically the same, but we need to have intersections, because here y changes)
// it should be very easy, except for face which has holes and will need special treatment
