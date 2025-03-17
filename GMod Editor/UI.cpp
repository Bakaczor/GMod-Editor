#include "pch.h"
#include "UI.h"
#include <algorithm>

void UI::RenderRotations(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(true);
	}
	if (ImGui::CollapsingHeader("Rotations")) {
		if (ImGui::RadioButton("X", &m_rotation, 0)) {
			uiChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Y", &m_rotation, 1)) {
			uiChanged = true;
		}
	}
}

void UI::RenderEllipsoid(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(true);
	}
	if (ImGui::CollapsingHeader("Ellipsoid")) {
		if (ImGui::InputFloat("a", &m_a, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			uiChanged = true;
		}
		if (ImGui::InputFloat("b", &m_b, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			uiChanged = true;
		}
		if (ImGui::InputFloat("c", &m_c, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			uiChanged = true;
		}
		if (ImGui::SliderFloat("scale", &m_scale, 0.0f, 2.0f, "%.2f")) {
			m_scale = std::max(0.001f, m_scale);
			uiChanged = true;
		}
	}
}

void UI::RenderRendering(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(true);
	}
	if (ImGui::CollapsingHeader("Rendering")) {
		if (ImGui::InputInt("step", &m_stepUI, 1, 10)) {
			m_stepUI = std::max(1, m_stepUI);
			m_step = m_stepUI;
			uiChanged = true;
		}
		if (ImGui::InputFloat("shininess", &m_shininess, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			m_shininess = std::max(0.0f, m_shininess);
			uiChanged = true;
		}
		ImGui::Separator();
		ImGui::Checkbox("show colors", &m_showColors);
		ImGui::Checkbox("add ambient", &m_addAmbient);
	}
}

void UI::RenderColors(bool firstPass) {
	if (ImGui::ColorEdit3("background", (float*)&m_backgroundColor)) {
		uiChanged = true;
	}
	if (ImGui::ColorEdit3("ellipsoid", (float*)&m_ellipsoidColor)) {
		uiChanged = true;
	}
}