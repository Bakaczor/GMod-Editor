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
		const float FZERO_XYZ = 100.f * std::numeric_limits<float>::epsilon();
		const float FZERO_UV = std::numeric_limits<float>::epsilon();
		const float m_radius = 4.f;
		const int m_samplingRes = 500;

		const Intersection::InterParams m_baseInterParams = {
			.gs = 5 * 1e-3,
			.gt = 5 * 1e-5,
			.gmi = 10000,
			.ns = 0.01,
			.nt = 5 * 1e-2,
			.nmi = 20,
			.nmr = 5,
			.mip = 2000,
			.d = 0.2,
			.cpt = 0.2
		};

		struct NamedInterParams {
			std::string name;
			Intersection::InterParams params;
		};

		struct MillingPartParams {
			std::string name; // name of the part
			Intersection::InterParams cuttingParams;
			std::vector<NamedInterParams> intersectingSurfaces; // which surfaces intersect with this part
			gmod::vector3<float> insidePoint; // the point that will be milled
			float epsilon; // used for separation: d - eps * r
			bool cutVertical; // should cut parallel to OZ or OX (horizontal)
			int startFrom; // 1 - top, 2 - right, 3 - bottom, 4 - left
		};

		const std::vector<MillingPartParams> m_millingParams = {
			MillingPartParams{ "bsurface_1", m_baseInterParams, { { "bsurface_2", m_baseInterParams }, { "bsurface_3", m_baseInterParams } }, { 0.f, 0.f, 0.f }, 1.5f, false, 1 }
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

		void Combine(std::vector<InterPoint>& finalContour, const std::vector<InterPoint>& intersectionLine,
			float insideU, float insideV, const Intersection::IDIG& part) const;
		
		SegmentGraph CutSurfaceIntoGraph(Intersection& intersection, const std::vector<InterPoint>& contour,
			const Intersection::IDIG& part, float epsilon, bool cutVertical, int startFrom, SegmentEnd3& startingPoint) const;

		void GetInnerSegments(const std::vector<InterPoint>& contour, const std::vector<InterPoint>& intersectionLine,
			std::vector<Segment3>& innerSegements, std::vector<SegmentEnd3>& contourIntersections, int& ID, const Intersection::IDIG& part) const;

		struct PartUVs {
			float u, v;
		};
		bool DoSegementsCross(PartUVs A, PartUVs B, PartUVs C, PartUVs D, PartUVs& intersection) const;
		bool IsInside(const PartUVs& point, const std::vector<StageThree::InterPoint>& closedContour) const;
	};
}
