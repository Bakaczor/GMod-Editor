#include "pch.h"
#include "UI.h"
#include <algorithm>

void UI::Render(bool firstPass) {
	RenderRightPanel();
	RenderSettings(firstPass);
}

void UI::RenderRightPanel() {
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
	const float width = 250.0f;
	const float height = viewportSize.y;

	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - width, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("Right panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
	RenderTransforms();
	RenderRotations(false);
	RenderEllipsoid(false);
	RenderRendering(false);
	ImGui::End();
}

void UI::RenderTransforms() {


}

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

void UI::RenderSettings(bool firstPass) {
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
	ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
	if (firstPass) {
		ImGui::SetNextItemOpen(false);
	}
	if (ImGui::CollapsingHeader("Editor Settings")) {
		ImVec2 size = ImGui::GetWindowSize();
		ImGui::SetWindowSize(ImVec2(size.x, 110.0f), ImGuiCond_Always);

		if (ImGui::ColorEdit3("Background", reinterpret_cast<float*>(&m_bkgdColor))) {
			uiChanged = true;
		}
		if (ImGui::Checkbox("Show Grid", &m_gridOn)) {
			uiChanged = true;
		}
		ImGui::Checkbox("Use MMB", &m_useMMB);
	}
	ImGui::End();
}