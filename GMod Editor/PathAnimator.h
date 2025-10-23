#pragma once
#include "framework.h"
#include <array>

namespace app {
	class PathAnimator {
	public:
		// millimetres per frame
		float stepSize = 15.f;
		bool isRunning = false;

		bool displayPath = true;
		std::array<float, 3> pathColor = { 0.f, 1.f, 0.f };

		void StartAnimation();
		void StopAnimation();
		void RestartAnimation();
		void CompleteAnimation();
	private:

	};
}
