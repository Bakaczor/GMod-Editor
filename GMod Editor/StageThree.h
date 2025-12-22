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

		const float baseY = 15.1f;
		const float width = 150.f;
		const float length = 150.f;
		const gmod::vector3<double> topLeftCorner = { -75, baseY, -75 };
		const gmod::vector3<double> centre = { 0, baseY, 0 };

		std::vector<gmod::vector3<float>> GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
	private:
		const float FZERO = 1e-2 * std::numeric_limits<float>::epsilon();
		const float FZERO_UV = 1e-6 * std::numeric_limits<float>::epsilon();
		const float m_radius = 4.f;
		const int m_samplingRes = 500;
		const float m_offsetBaseY = baseY + m_radius;

		const Intersection::InterParams m_baseInterParams = {
			.gs = 1 * 1e-3,
			.gt = 5 * 1e-5,
			.gmi = 10000,
			.ns = 0.01,
			.nt = 9 * 1e-3,
			.nmi = 20,
			.nmr = 5,
			.mip = 10000,
			.d = 0.1,
			.cpt = 0.09
		};

		const Intersection::InterParams m_partInterParams = {
			.gs = 1 * 1e-3,
			.gt = 5 * 1e-5,
			.gmi = 10000,
			.ns = 0.01,
			.nt = 9 * 1e-3,
			.nmi = 20,
			.nmr = 5,
			.mip = 15000,
			.d = 0.1,
			.cpt = 0.09
		};

		//const Intersection::InterParams m_specialParams = {
		//	.gs = 1 * 1e-3,
		//	.gt = 5 * 1e-5,
		//	.gmi = 10000,
		//	.ns = 0.01,
		//	.nt = 9 * 1e-3,
		//	.nmi = 20,
		//	.nmr = 5,
		//	.mip = 10000,
		//	.d = 0.1,
		//	.cpt = 0.09
		//};

		struct NamedInterParams {
			std::string name;
			bool useNumericalNormal;
			Intersection::InterParams params;
			bool useCursor = false;
			gmod::vector3<double> cursorPos = { 0,0,0 };
		};

		struct MillingPartParams {
			std::string name; // name of the part
			Intersection::InterParams cuttingParams;
			std::vector<NamedInterParams> intersectingSurfaces; // which surfaces intersect with this part
			gmod::vector3<float> insidePoint; // the point that will be milled
			float epsilon; // used for separation: d - eps * r
			float YRotation; // specify rotation of the knife in degrees
			int cuttingDir; // 1 - knive goes to the left, 2 - knife goes to the right
			bool useNumericalNormal;
			int startFrom; // 1(2) - top, 2(1) - right, 3(1) - bottom, 4(2) - left
		};

		std::vector<MillingPartParams> m_millingParams = {
			//MillingPartParams{
			//	"test", m_specialParams, {

			//	},
			//	{ 0.f, 0.f, 0.f }, 1.5f, true, 4
			//},
			// earL - ok
			MillingPartParams{
				"earL", m_baseInterParams, {
					{ "head", false, m_partInterParams }
				},
				{ -30.f, 0.f, -45.f }, 1.7f, 135, 2, true, 4 // 0, 2
			},
			// earR - ok
			MillingPartParams{
				"earR", m_baseInterParams, {
					{ "head", false, m_partInterParams }
				},
				{ 30.f, 0.f, -45.f }, 1.7f, 45, 1, true, 2 // 0, 2
			},
			// head - ok
			MillingPartParams{
				"head", m_baseInterParams, {
					{ "earL", true, m_partInterParams },
					{ "earR", true, m_partInterParams },
					{ "body", false, m_partInterParams }
				},
				{ 0.f, 0.f, -10.f }, 1.7f, 0, 2, false, 1
			},
			// tail - ok
			MillingPartParams{
				"tail", m_baseInterParams, {
					{ "legBR", false, m_partInterParams, true, gmod::vector3<double>(35, 19, 39) },
					{ "body", false, m_partInterParams }
				},
				{ 50.f, 0.f, 40.f }, 1.7f, 90, 1, false, 2
			},
			// legBL - ok
			MillingPartParams{
				"legBL", m_baseInterParams, {
					{ "body", false, m_partInterParams },
					{ "legFL", false, m_partInterParams, true, gmod::vector3<double>(-22.5, 19, 40) }
				},
				{ -32.f, 0.f, 48.f }, 1.7f, 0, 1, false, 3
			},
			// legFL - ok
			MillingPartParams{
				"legFL", m_partInterParams, {
					{ "body", false, m_partInterParams, true, gmod::vector3<double>(0, 15, 55) },
					{ "legBL", false, m_partInterParams, true, gmod::vector3<double>(-22.5, 19, 40) },
					{ "legFR", false, m_partInterParams, true, gmod::vector3<double>(0, 15, 45) }
				},
				{ -10.f, 0.f, 60.f }, 1.7f, 0, 1, false, 3
			},
			// legFR - ok
			MillingPartParams{
				"legFR", m_baseInterParams, {
					{ "body", false, m_partInterParams, true, gmod::vector3<double>(0, 15, 55) },
					{ "legBR", false, m_partInterParams, true, gmod::vector3<double>(22.5, 19, 40) },
					{ "legFL", false, m_partInterParams, true, gmod::vector3<double>(0, 15, 45) }
				},
				{ 10.f, 0.f, 60.f }, 1.7f, 0, 1, false, 3
			},
			// legBR - ok
			MillingPartParams{
				"legBR", m_baseInterParams, {
					{ "body", false, m_partInterParams },
					{ "tail", false, m_partInterParams, true, gmod::vector3<double>(35, 19, 39) },
					{ "legFR", false, m_partInterParams, true,  gmod::vector3<double>(22.5, 19, 40) }
				},
				{ 32.f, 0.f, 48.f }, 1.7f, 0, 1, false, 3
			},
			// body - ok
			MillingPartParams{
				"body", m_baseInterParams, {
					{ "head", false, m_partInterParams },
					{ "legBR", false, m_partInterParams },
					{ "legFR", false, m_partInterParams },
					{ "legFL", false, m_partInterParams },
					{ "legBL", false, m_partInterParams },
					{ "tail", false, m_partInterParams }
				},
				{ 0.f, 0.f, 25.f }, 1.7f, 90, 2, false, 4
			}
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
			const Intersection::IDIG& part, const std::vector<std::pair<Intersection::IDIG, NamedInterParams>>& intersectingSurfaces) const;

		void Combine(std::vector<InterPoint>& finalContour, const std::vector<InterPoint>& intersectionLine,
			float insideU, float insideV, const Intersection::IDIG& part) const;
		
		SegmentGraph CutSurfaceIntoGraph(Intersection& intersection, const std::vector<InterPoint>& contour, const Intersection::InterParams& cuttingParams,
			const Intersection::IDIG& part, float epsilon, float YRotation, int cuttingDir, SegmentEnd3& startingPoint) const;

		void GetInnerSegments(const std::vector<InterPoint>& contour, const std::vector<InterPoint>& intersectionLine,
			std::vector<Segment3>& innerSegements, std::vector<SegmentEnd3>& contourIntersections, int& ID, const Intersection::IDIG& part) const;

		struct PartUVs {
			float u, v;
		};
		bool DoSegementsCross(PartUVs A, PartUVs B, PartUVs C, PartUVs D, PartUVs& intersection) const;
		bool IsInside(const PartUVs& point, const std::vector<StageThree::InterPoint>& closedContour) const;

		bool AreSimilar(const InterPoint& a, const InterPoint& b, const InterPoint& c) const;
	};
}
