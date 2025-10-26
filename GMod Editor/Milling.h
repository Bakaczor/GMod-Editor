#pragma once
#include "../gmod/vector3.h"
#include "CutterType.h"
#include <array>
#include <optional>
#include <string>
#include "Cutter.h"

namespace app {
	class Milling {
	public:
		Cutter cutter;

		bool baseMeshSizeChanged = false;
		unsigned int baseMeshSize = 5;
		unsigned int resolutionX = 30;
		unsigned int resolutionY = 30;

		std::array<float, 3> centre = { 0.f, 0.f, 0.f };

		// all in centimetres
		std::array<float, 3> size = { 15.f, 15.f, 5.f };
		float baseThickness = 1.5f;
		float margin = 0.5f;

		std::optional<std::string> Mill(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos);
		void ResetScene();
	private:

	};
}
