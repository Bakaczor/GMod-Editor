#pragma once
#include "Object.h"
#include "Intersection.h"
#include "SegmentGraph.h"
#include <unordered_map>

namespace app {
	class StageTwo {
	public:
		const std::string stage = "2";
		const std::string type = "f";
		const int diameter = 10;

		const float epsilon = 0.2f; // usage: d - eps * r
		const float totalHeight = 50.f;

		const float baseY = 15.1f;
		const float width = 150.f;
		const float length = 150.f;
		const gmod::vector3<double> topLeftCorner = { 25, baseY, 25 };
		const gmod::vector3<double> centre = { 100, baseY, 100 };
		const bool translateBack = true;

		std::vector<gmod::vector3<float>> GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
	private:
		const float FZERO = 10.f * std::numeric_limits<float>::epsilon();
		const float m_radius = 5.f;
		const int m_expectedIntersections = 2;
		
		const Intersection::InterParams m_interParams = {
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
		
		// second and any next surface should have at least one intersection with any previous surface
		struct SurfParams {
			int order;
			Intersection::InterParams params;
		};
		const std::unordered_map<std::string, SurfParams> m_contourSurfaces = {
			/*{ "test", { 0, m_specialParams } }*/
			{ "head", { 0, m_interParams } },
			{ "earL", { 1, m_interParams } },
			{ "earR", { 2, m_interParams } },
			{ "body", { 3, m_interParams } },
			{ "legFL", { 4, m_interParams } },
			{ "legFR", { 5, m_interParams } },
			{ "legBL", { 6, m_interParams } },
			{ "legBR", { 7, m_interParams } },
			{ "tail", { 8, m_interParams } }
		};
		const int m_numOfSurfaces = 9;

		struct InterPoint {
			gmod::vector3<float> pos; // at baseY
			gmod::vector3<float> norm; // on XZ
			float u, v;
			const IGeometrical* surf;
		};
		std::vector<InterPoint> CreateOffsetContour(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;

		void Combine(std::vector<StageTwo::InterPoint>& mainContour, const std::vector<StageTwo::InterPoint>& newContour) const;

		std::vector<gmod::vector3<float>> GetFinalPath(const SegmentGraph& G, const SegmentEnd2& start, 
			const std::vector<StageTwo::InterPoint>& offsetContour, int topCountourIdx, float zTop) const;

		bool IsOutside(const StageTwo::InterPoint& point, const std::vector<StageTwo::InterPoint>* contour) const;

		bool IsCW(const std::vector<Intersection::PointOfIntersection>& contour) const;

		bool DoSegementsCross(gmod::vector3<float> A, gmod::vector3<float> B,
			gmod::vector3<float> C, gmod::vector3<float> D, gmod::vector3<float>& intersection) const;
	};
}