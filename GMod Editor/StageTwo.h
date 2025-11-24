#pragma once
#include "Object.h"
#include "Intersection.h"

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
	};
}