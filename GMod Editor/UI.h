#pragma once
#include "pch.h"
#include "../imgui/imgui.h"
#include "Object.h"
#include <memory>

class UI {
public:
	// TRANSFORMS
	enum class Mode {
		Neutral,
		Translate,
		Rotate,
		Scale
	};
	Mode currentMode = Mode::Neutral;

	enum class Axis {
		X, Y, Z
	};
	Axis currentAxis = Axis::Y;

	// OBJECTS
	int currObjId = -1;
	std::vector<std::shared_ptr<Object>> objects;

	// SETTINGS
	bool useMMB = true;
	bool showGrid = false;
	bool showAxes = false;
	ImVec4 bkgdColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	void Render(bool firstPass);
private:
	int m_selectedObjIdx = -1;

	void RenderRightPanel(bool firstPass);
	void RenderTransforms();
	void RenderObjectTable(bool firstPass);
	void RenderSelectedObject();
	void RenderSettings(bool firstPass);
};