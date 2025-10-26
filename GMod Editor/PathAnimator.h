#pragma once
#include <array>
#include "Milling.h"

namespace app {
	class PathAnimator {
	public:
		// millimetres per frame
		float stepSize = 15.f;
		bool isRunning = false;

		bool displayPath = true;
		std::array<float, 3> pathColor = { 0.f, 1.f, 0.f };

		PathAnimator(Milling& milling);

		void StartAnimation();
		void StopAnimation();
		void RestartAnimation();
		void CompleteAnimation();
		void MakeStep();
	private:
		Milling& m_milling;
	};
}
