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

		const float epsilon = 2.f;
		const float totalHeight = 50.f;

		const float baseY = 15.f;
		const float width = 150.f;
		const float length = 150.f;
		const gmod::vector3<double> topLeftCorner = { -75, baseY, -75 };
		const gmod::vector3<double> centre = { 0, baseY, 0 };

		std::vector<gmod::vector3<float>> GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
	private:
		const float FZERO = 100.f * std::numeric_limits<float>::epsilon();
		const float m_radius = 5.f;
		
		const Intersection::InterParams m_interParams = {
			.gs = 5 * 1e-3,
			.gt = 5 * 1e-5,
			.gmi = 10000,
			.ns = 0.01,
			.nt = 5 * 1e-3,
			.nmi = 6,
			.nmr = 7,
			.mip = 100,
			.d = 1e-2,
			.cpt = 1e-3
		};

		struct CombineParams {
			int expectedIntersections;
			gmod::vector3<float> startPoint;
		};

		struct StageTwoParams {
			Intersection::InterParams interParams;
			CombineParams combineParams;
		};
		
		// second and any next surface should have at least one intersection with any previous surface
		const std::unordered_map<std::string, StageTwoParams> m_contourSurfaces = {
			{ "name1", { m_interParams, 0 }},
			{ "name2", { m_interParams, 2 }}
		};

		struct InterPoint {
			gmod::vector3<float> pos; // at baseY
			gmod::vector3<float> norm; // projected onto XZ
			float u, v;
			const IGeometrical* surf;
		};
		std::vector<InterPoint> CreateOffsetContour(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
		void Combine(std::vector<StageTwo::InterPoint>& mainContour, const std::vector<StageTwo::InterPoint>& newContour, CombineParams combineParams) const;
		std::vector<gmod::vector3<float>> GetFinalPath(const SegmentGraph& G, const SegmentEnd& start, 
			const std::vector<StageTwo::InterPoint>& offsetContour, int topCountourIdx, float zTop) const;
		void EnsureClockwiseOrder(std::vector<StageTwo::InterPoint>& points) const;
	};
}