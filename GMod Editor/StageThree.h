#pragma once
#include "Object.h"
#include "Intersection.h"

namespace app {
	class StageThree {
	public:
		const std::string stage = "3";
		const std::string type = "k";
		const int diameter = 8;

		const float epsilon = 0.2f; // usage: d - eps * r
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
	};
}

// for part 3 (initial idea, read lectures before that)
// for every surface find offset surface
// for every offset surface find intersection with other surfaces and create bounded parametric surface (make it specific)
// using vertical plane, cut find intersections with milling surface and constrain them using bounds found before
// you will get milling parts like in part two
// try to create paths the same way as in part two (this is basically the same, but we need to have intersections, because here y changes)
// it should be very easy, except for face which has holes and will need special treatment
