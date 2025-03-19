#include "pch.h"
#include "UI.h"
#include <algorithm>

void UI::Render(bool firstPass) {
	RenderRightPanel(firstPass);
	RenderSettings(firstPass);
}

void UI::RenderRightPanel(bool firstPass) {
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
	const float width = 250.0f;
	const float height = viewportSize.y;

	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - width, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(1.0f);

	ImGui::Begin("Right panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
	RenderTransforms();
	RenderObjectTable(firstPass);
	RenderSelectedObject();
	ImGui::End();
}

void UI::RenderTransforms() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_Header]);
	ImGui::BeginGroup();

	const float padx = style.WindowPadding.x / 2;
	const float pady = style.WindowPadding.y / 2;
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 pMin = ImVec2(ImGui::GetWindowPos().x + padx, ImGui::GetWindowPos().y + pady);
	ImVec2 pMax = ImVec2(pMin.x + ImGui::GetWindowWidth() - 2 * padx, pMin.y + ImGui::GetTextLineHeightWithSpacing() + pady);
	drawList->AddRectFilled(pMin, pMax, ImGui::GetColorU32(ImGuiCol_Header));
	ImGui::Text("Mode:");
	if (currentMode != Mode::Neutral) {
		ImGui::SameLine(ImGui::GetWindowWidth() * 0.5f);
		ImGui::Text("Axes:");
	}
	float currentY = ImGui::GetCursorPosY();
	ImGui::SetCursorPosY(currentY + pady);

	if (currentMode != Mode::Neutral) {
		ImGui::Columns(2, "ModeAndAxisColumns", false);
	}
	if (ImGui::RadioButton("Neutral", currentMode == Mode::Neutral)) {
		currentMode = Mode::Neutral;
	}
	if (ImGui::RadioButton("Translate", currentMode == Mode::Translate)) {
		currentMode = Mode::Translate;
	}
	if (ImGui::RadioButton("Rotate", currentMode == Mode::Rotate)) {
		currentMode = Mode::Rotate;
	}
	if (ImGui::RadioButton("Scale", currentMode == Mode::Scale)) {
		currentMode = Mode::Scale;
	}
	if (currentMode != Mode::Neutral) {
		ImGui::NextColumn();
		if (ImGui::RadioButton("X", currentAxis == Axis::X)) {
			currentAxis = Axis::X;
		}
		if (ImGui::RadioButton("Y", currentAxis == Axis::Y)) {
			currentAxis = Axis::Y;
		}
		if (ImGui::RadioButton("Z", currentAxis == Axis::Z)) {
			currentAxis = Axis::Z;
		}
		ImGui::Columns(1);
	}
	ImGui::EndGroup();
	ImGui::PopStyleColor();
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
		if (ImGui::InputInt("step", &m_stepUI, 1, 10)) {
			m_stepUI = std::max(1, m_stepUI);
			m_step = m_stepUI;
			uiChanged = true;
		}
	}
}

void UI::RenderObjectTable(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(false);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("List of objects")) {
		ImGui::BeginChild("TableWindow", ImVec2(0, 200), false, ImGuiWindowFlags_None);
		if (ImGui::BeginTable("ObjectTable", 2, ImGuiTableFlags_ScrollY)) {
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableHeadersRow();

			for (int i = 0; i < objects.size(); i++) {
				ImGui::TableNextRow();

				ImGui::TableNextColumn();
				if (ImGui::Selectable(objects[i]->name.c_str(), m_selectedObjIdx == i, ImGuiSelectableFlags_SpanAllColumns)) {
					if (m_selectedObjIdx == i) {
						m_selectedObjIdx = -1;
						currObjId = -1;
					} else {
						m_selectedObjIdx = i;
						currObjId = objects[i]->id;
					}
				}
				ImGui::TableNextColumn();
				ImGui::Text(objects[i]->type().c_str());
			}

			ImGui::EndTable();
		}
		ImGui::EndChild();
	}
}

void UI::RenderSelectedObject() {
	if (m_selectedObjIdx != -1) {
		ImGui::Spacing();
		ImGui::BeginChild("PropertiesWindow", ImVec2(0, 200), true, ImGuiWindowFlags_None);
		ImGui::Text("Properties");
		// TODO
		ImGui::EndChild();
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