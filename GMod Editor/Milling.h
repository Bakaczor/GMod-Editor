#pragma once
#include "../gmod/vector3.h"
#include "CutterType.h"
#include <array>

namespace app {
	class Milling {
	public:
		unsigned int baseMeshSize = 5;
		unsigned int resolutionX = 30;
		unsigned int resolutionY = 30;
		std::array<float, 3> size = { 15.f, 15.f, 5.f };
		std::array<float, 3> centre = { 0.f, 0.f, 0.f };
		float baseThickness = 1.5f;
		float margin = 0.5f;


		// milling (all in millimetres)
		float millingPartDiameter = 0.0f;
		float millingPartHeight = 0.0f;
		float totalCutterLength = 0.0f;

		CutterType cutterType = CutterType::Spherical;
		bool useCutterBase = true;
		float maxHorizontalDeviationAngle = 0.0f;

		void ResetScene();
	private:
		bool m_baseMeshSizeChanged = false;
	};
}
