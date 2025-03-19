#include "pch.h"
#include "UI.h"

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

void UI::RenderObjectTable(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(false);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("List of objects")) {
		
		ImGui::BeginChild("TableWindow", ImVec2(0, 300), false, ImGuiWindowFlags_None);
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
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Properties")) {
		ImGui::BeginChild("PropertiesWindow", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetCursorPos().y - style.WindowPadding.y), true, ImGuiWindowFlags_NoBackground);
		if (m_selectedObjIdx != -1) {
			std::shared_ptr<Object>& selectedObj = objects[m_selectedObjIdx];
			double step = 0.001f;
			double stepFast = 0.1f;

			ImGui::Text("Position");
			gmod::vector3<double> position = selectedObj->transform.position();
			if (ImGui::InputDouble("X##Position", &position.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetTranslation(position.x(), position.y(), position.z());
			}
			if (ImGui::InputDouble("Y##Position", &position.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetTranslation(position.x(), position.y(), position.z());
			}
			if (ImGui::InputDouble("Z##Position", &position.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetTranslation(position.x(), position.y(), position.z());
			}

			ImGui::Text("Euler Angles");
			gmod::vector3<double> eulerAngles = selectedObj->transform.eulerAngles();
			if (ImGui::InputDouble("X##Euler Angles", &eulerAngles.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetRotation(eulerAngles.x(), eulerAngles.y(), eulerAngles.z());
			}
			if (ImGui::InputDouble("Y##Euler Angles", &eulerAngles.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetRotation(eulerAngles.x(), eulerAngles.y(), eulerAngles.z());
			}
			if (ImGui::InputDouble("Z##Euler Angles", &eulerAngles.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetRotation(eulerAngles.x(), eulerAngles.y(), eulerAngles.z());
			}

			ImGui::Text("Scale");
			gmod::vector3<double> scale = selectedObj->transform.scale();
			if (ImGui::InputDouble("X##Scale", &scale.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetScaling(scale.x(), scale.y(), scale.z());
			}
			if (ImGui::InputDouble("Y##Scale", &scale.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetScaling(scale.x(), scale.y(), scale.z());
			}
			if (ImGui::InputDouble("Z##Scale", &scale.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
				selectedObj->transform.SetScaling(scale.x(), scale.y(), scale.z());
			}
			ImGui::Spacing();
			selectedObj->RenderObjectProperties();
		}
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
		ImGui::SetWindowSize(ImVec2(size.x, 130.0f), ImGuiCond_Always);
		ImGui::ColorEdit3("Background", reinterpret_cast<float*>(&bkgdColor));
		ImGui::Checkbox("Show Grid", &showGrid);
		ImGui::Checkbox("Show Axes", &showAxes);
		ImGui::Checkbox("Use MMB", &useMMB);
	}
	ImGui::End();
}