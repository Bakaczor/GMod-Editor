#pragma once
#include "Object.h"
#include "Intersection.h"
#include <map>

namespace app {
	class StageOne {
	public:
		const std::string stage = "1";
		const std::string type = "k";
		const int diameter = 16;

		const float epsilon = 0.2f; // usage: d - eps * r
		const float maxSlopeInDeg = 20.f; // TODO: actual is around double that - investigate
		const float offset = 2.f;
		const float totalHeight = 50.f;

		const float baseY = 15.f;
		const float width = 150.f;
		const float length = 150.f;
		const gmod::vector3<double> topLeftCorner = { -75, baseY, -75 };
		const gmod::vector3<double> centre = { 0, baseY, 0 };

		std::vector<gmod::vector3<float>> GeneratePath(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
	private:
		const float FZERO = 100.f * std::numeric_limits<float>::epsilon();
		const int m_resX = 1500;
		const int m_resZ = 1500;
		const float m_radius = 8.f;
		const Intersection::InterParams m_interParams = {
			.gs = 5 * 1e-3,
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

		std::vector<std::vector<float>> CreateHeightmapByIntersections(const std::vector<std::unique_ptr<Object>>& sceneObjects, Intersection& intersection) const;
		std::vector<std::vector<float>> CreateHeightmapByUVSampling(const std::vector<std::unique_ptr<Object>>& sceneObjects) const;
		std::vector<gmod::vector3<float>> MakeSmooth(const std::vector<gmod::vector3<float>>& path) const;
		float CheckInRange(const std::vector<std::vector<float>>& heightmap, int currX, int currZ) const;
	};
}
