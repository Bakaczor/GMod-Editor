#pragma once
#include "framework.h"
#include <imgui.h>
#include <array>


class PathAnimator {
public:
	// millimetres per frame
	float stepSize = 15.f;

	bool displayPath = true;
	ImVec4 pathColor = ImVec4(0.f, 1.f, 0.f, 1.f);

	std::array<float, 3> size = { 15.f, 15.f, 5.f };
	float baseThickness = 1.5f;
	float millingBoxMargin = 0.0f;
	std::array<unsigned int, 2> resolution = { 150, 150 };


private:
};