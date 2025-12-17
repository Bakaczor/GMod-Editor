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
		const gmod::vector3<double> topLeftCorner = { 25, baseY, 25 };
		const gmod::vector3<double> centre = { 100, baseY, 100 };
		const bool translateBack = true;

		StageThree();
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
			bool cutVertical; // should cut parallel to OZ or OX (horizontal)
			int startFrom; // 1 - top, 2 - right, 3 - bottom, 4 - left
			bool useNumericalNormal;
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
				{ -27.f, 0.f, -33.f }, 1.65f, false, 1, true
			},
			// earR - ok
			MillingPartParams{
				"earR", m_baseInterParams, {
					{ "head", false, m_partInterParams }
				},
				{ 27.f, 0.f, -33.f }, 1.65f, false, 1, true
			},
			// head - ok
			MillingPartParams{
				"head", m_baseInterParams, {
					{ "earL", true, m_partInterParams },
					{ "earR", true, m_partInterParams },
					{ "body", false, m_partInterParams }
				},
				{ 0.f, 0.f, 0.f }, 1.5f, false, 1, false
			},
			// tail - ok
			MillingPartParams{
				"tail", m_baseInterParams, {
					{ "legBR", false, m_partInterParams, true, gmod::vector3<double>(25, 19, 55) }
				},
				{ 40.f, 0.f, 55.f }, 1.75f, true, 2, false
			},
			// legBL - ok
			MillingPartParams{
				"legBL", m_baseInterParams, {
					{ "body", false, m_partInterParams },
					{ "legFL", false, m_partInterParams, true, gmod::vector3<double>(-18, 19, 50) }
				},
				{ -24.f, 0.f, 52.f }, 1.75f, false, 3, false
			},
			// legFL - ok
			MillingPartParams{
				"legFL", m_partInterParams, {
					{ "body", false, m_partInterParams },
					{ "legBL", false, m_partInterParams, true, gmod::vector3<double>(-18, 19, 50) }
				},
				{ -10.f, 0.f, 50.f }, 1.75f, false, 3, false
			},
			// legFR - ok
			MillingPartParams{
				"legFR", m_baseInterParams, {
					{ "body", false, m_partInterParams },
					{ "legBR", false, m_partInterParams, true, gmod::vector3<double>(18, 19, 50) }
				},
				{ 10.f, 0.f, 50.f }, 1.75f, false, 3, false
			},
			// legBR - ok
			MillingPartParams{
				"legBR", m_baseInterParams, {
					{ "body", false, m_partInterParams },
					{ "tail", false, m_partInterParams, true, gmod::vector3<double>(25, 19, 55) },
					{ "legFR", false, m_partInterParams, true,  gmod::vector3<double>(18, 19, 50) }
				},
				{ 24.f, 0.f, 52.f }, 1.75f, false, 3, false
			},
			// body - ok
			MillingPartParams{
				"body", m_baseInterParams, {
					{ "head", false, m_partInterParams },
					{ "legBR", false, m_partInterParams },
					{ "legFR", false, m_partInterParams },
					{ "legFL", false, m_partInterParams },
					{ "legBL", false, m_partInterParams }
				},
				{ 0.f, 0.f, 30.f }, 1.75f, true, 4, false
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
