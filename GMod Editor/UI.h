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
		X, Y, Z, All
	};
	Axis currentAxis = Axis::Y;

	// OBJECTS
	int selectedObjId = -1;
	int selectedRowIdx = -1;
	std::vector<std::shared_ptr<Object>> objects;

	// SETTINGS
	bool useMMB = true;
	bool showGrid = false;
	bool showAxes = false;
	ImVec4 bkgdColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	void Render(bool firstPass);
private:
	void RenderRightPanel(bool firstPass);
	void RenderTransforms();
	void RenderCursor();
	void RenderObjectTable(bool firstPass);
	void RenderSelectionTable(bool firstPass);
	void RenderSelectedObject();
	void RenderSettings(bool firstPass);
};