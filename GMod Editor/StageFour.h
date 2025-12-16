#pragma once
#include "Object.h"
#include "Intersection.h"
#include <map>

namespace app {
	class StageFour {
	public:
		const std::string stage = "4";
		const std::string type = "k";
		const int diameter = 1;

		const float totalHeight = 50.f;
		const float saveHeight = 20.f;

		const float baseY = 15.f;
		const float width = 150.f;
		const float length = 150.f;
		const gmod::vector3<double> topLeftCorner = { -75, baseY, -75 };
		const gmod::vector3<double> centre = { 0, baseY, 0 };

		StageFour();
		std::vector<gmod::vector3<float>> GeneratePath() const;
	private:
		const float m_radius = 0.5f;
		const std::string m_texturePath = "./textures/stage_four.bmp";
		int m_resX = 1500;
		int m_resZ = 1500;
		std::vector<std::vector<bool>> m_boolMap;

		void LoadImageSTB();
		struct Pixel {
			int x = 0;
			int z = 0;
		};
		std::vector<Pixel> PixelPath(std::vector<std::vector<bool>>& visited, int xStart, int zStart) const;
		gmod::vector3<float> Pixel2Pos(int x, int z) const;
	};
}
