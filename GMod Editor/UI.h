#pragma once
#include "pch.h"
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


	float m_a = 0.5f;
	float m_b = 1.0f;
	float m_c = 1.5f;
	float m_scale = 1.0f;
	int m_stepUI = 1;
	int m_step = 1;
	void RenderEllipsoid(bool firstPass);

	// SETTINGS
	bool m_useMMB = true;
	bool m_gridOn = false;
	ImVec4 m_bkgdColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	void Render(bool firstPass);
	bool uiChanged = false;
private:
	int m_selectedObjIdx = -1;

	void RenderRightPanel(bool firstPass);
	void RenderTransforms();
	void RenderObjectTable(bool firstPass);
	void RenderSelectedObject();
	void RenderSettings(bool firstPass);
};