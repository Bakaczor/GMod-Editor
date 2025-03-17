#pragma once
#include "pch.h"

struct UI {
	int m_rotation = 0;
	void RenderRotations(bool firstPass);

	float m_a = 0.5f;
	float m_b = 1.0f;
	float m_c = 1.5f;
	float m_scale = 1.0f;
	void RenderEllipsoid(bool firstPass);

	int m_stepUI = 1;
	int m_step = 1;
	int m_currStep = m_step;
	float m_shininess = 4.0f;
	void RenderRendering(bool firstPass);

	bool m_showColors = false;
	bool m_addAmbient = false;

	ImVec4 m_backgroundColor = ImVec4(0.5f, 0.25f, 0.5f, 1.0f);
	ImVec4 m_ellipsoidColor = ImVec4(1.0f, 0.9f, 0.0f, 1.0f);
	void RenderColors(bool firstPass);

	bool uiChanged = false;
};