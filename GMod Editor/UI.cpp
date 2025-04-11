#include "UI.h"
#include "../gmod/utility.h"
#include "Torus.h"
#include "Cube.h"
#include "Point.h"
#include "Pointline.h"

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
	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("Cursor")) {
			RenderCursor();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Objects")) {
			RenderObjectTable(firstPass);
			RenderSelection(firstPass);
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Properties")) {
			RenderSelectedObject();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void UI::RenderTransforms() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_Header]);
	ImGui::BeginChild("TransformsWindow", ImVec2(0, 200), true, ImGuiWindowFlags_NoBackground);
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
	if (!noObjectSelected() && !(currentOrientation == Orientation::Selection && selection.Contains(objects_selectedObjId))) {
		if (ImGui::RadioButton("Rotate", currentMode == Mode::Rotate)) {
			currentMode = Mode::Rotate;
		}
		if (ImGui::RadioButton("Scale", currentMode == Mode::Scale)) {
			currentMode = Mode::Scale;
		}
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
		if (currentMode == Mode::Scale) {
			if (ImGui::RadioButton("All", currentAxis == Axis::All)) {
				currentAxis = Axis::All;
			}
		}
		ImGui::Columns(1);
	}
	ImGui::EndGroup();
	ImGui::PopStyleColor();
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Orientation")) {
		ImGui::BeginGroup();
		if (ImGui::RadioButton("World", currentOrientation == Orientation::World)) {
			currentOrientation = Orientation::World;
		}
		if (ImGui::RadioButton("Cursor", currentOrientation == Orientation::Cursor)) {
			currentOrientation = Orientation::Cursor;
		}
		if (ImGui::RadioButton("Selection", currentOrientation == Orientation::Selection)) {
			currentOrientation = Orientation::Selection;
		}
		ImGui::EndGroup();
	}
	ImGui::EndChild();
}

void UI::RenderCursor() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::BeginChild("PropertiesWindow", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetCursorPos().y - style.WindowPadding.y), true, ImGuiWindowFlags_NoBackground);
	double step = 0.001f;
	double stepFast = 0.1f;

	ImGui::Text("Position");
	gmod::vector3<double> position = cursor.transform.position();
	if (ImGui::InputDouble("X##Position", &position.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		cursor.transform.SetTranslation(position.x(), position.y(), position.z());
	}
	if (ImGui::InputDouble("Y##Position", &position.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		cursor.transform.SetTranslation(position.x(), position.y(), position.z());
	}
	if (ImGui::InputDouble("Z##Position", &position.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
		cursor.transform.SetTranslation(position.x(), position.y(), position.z());
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("Object types:");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::Combo("##types", &m_selectedObjType, m_objectTypeNames.data(), m_objectTypeNames.size());
	if (ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		auto pos = cursor.transform.position();
		switch (m_objectTypes.at(m_selectedObjType)) {
			case ObjectType::Cube: {
				auto obj = std::make_unique<Cube>();
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				objects.push_back(std::move(obj));
				break;
			}
			case ObjectType::Torus: {
				auto obj = std::make_unique<Torus>();
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				objects.push_back(std::move(obj));
				break;
			}
			case ObjectType::Point: {
				auto obj = std::make_unique<Point>();
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				objects.push_back(std::move(obj));
				numOfPointObjects++;
				break;
			}
		}
	}
	ImGui::EndChild();
}

void UI::RenderObjectTable(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(false);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("List of objects")) {
		if (ImGui::Button("Add to selection", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
			if (objects_selectedRowIdx != -1) {
				selection.AddObject(objects[objects_selectedRowIdx].get());
			}
		}
		if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
			if (objects_selectedRowIdx != -1) {
				auto obj = objects[objects_selectedRowIdx].get();
				selection.RemoveObject(obj);
				obj->RemoveReferences();
				if (nullptr != dynamic_cast<Point*>(obj)) {
					numOfPointObjects--;
				}
				objects.erase(objects.begin() + objects_selectedRowIdx);

				objects_selectedRowIdx = -1;
				objects_selectedObjId = -1;

				selection_selectedRowIdx = -1;
				selection_selectedObjId = -1;
			}
		}
		ImGui::BeginChild("ObjectTableWindow", ImVec2(0, tableHeight(objects.size())), false, ImGuiWindowFlags_None);
		if (ImGui::BeginTable("ObjectTable", 2, ImGuiTableFlags_ScrollY)) {
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableHeadersRow();

			for (int i = 0; i < objects.size(); i++) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (ImGui::Selectable(objects[i]->name.c_str(), objects_selectedRowIdx == i, ImGuiSelectableFlags_SpanAllColumns)) {
					if (objects_selectedRowIdx == i) {
						objects_selectedRowIdx = -1;
						objects_selectedObjId = -1;
					} else {
						objects_selectedRowIdx = i;
						objects_selectedObjId = objects[i]->id;
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

void UI::SelectObjectOnMouseClick(Object* obj) {
	int i;
	objects_selectedObjId = obj->id;
	selection_selectedObjId = obj->id;
	selection.AddObject(obj);

	for (i = 0; i < objects.size(); i++) {
		if (objects[i]->id == objects_selectedObjId) { break; }
	}
	objects_selectedRowIdx = i;

	for (i = 0; i < selection.selected.size(); i++) {
		if (selection.selected[i]->id == selection_selectedObjId) { break; }
	}
	selection_selectedRowIdx = i;
}

void UI::RenderSelection(bool firstPass) {
	if (firstPass) {
		ImGui::SetNextItemOpen(false);
	}
	ImGui::Spacing();
	if (ImGui::CollapsingHeader("Selection")) {
		if (ImGui::Button("Remove from selection", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
			if (selection_selectedRowIdx != -1) {
				selection.RemoveObject(selection.selected[selection_selectedRowIdx]);
				selection_selectedRowIdx = -1;
				selection_selectedObjId = -1;
			}
		}
		if (ImGui::Button("Clear selection", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
			selection.Clear();
			selection_selectedRowIdx = -1;
			selection_selectedObjId = -1;
		}
		ImGui::BeginChild("SelectionWindow", ImVec2(0, tableHeight(selection.selected.size())), false, ImGuiWindowFlags_None);
		if (ImGui::BeginTable("Selected", 2, ImGuiTableFlags_ScrollY)) {
			ImGui::TableSetupColumn("Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableHeadersRow();

			for (int i = 0; i < selection.selected.size(); i++) {
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				if (ImGui::Selectable(selection.selected[i]->name.c_str(), selection_selectedRowIdx == i, ImGuiSelectableFlags_SpanAllColumns)) {
					if (selection_selectedRowIdx == i) {
						selection_selectedRowIdx = -1;
						selection_selectedObjId = -1;
					} else {
						selection_selectedRowIdx = i;
						selection_selectedObjId = selection.selected[i]->id;
					}
				}
				ImGui::TableNextColumn();
				ImGui::Text(selection.selected[i]->type().c_str());
			}
			ImGui::EndTable();
		}
		ImGui::EndChild();
		if (!selection.isPolyline()) {
			ImGui::BeginDisabled();
		}
		if (ImGui::Button("Create polyline", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
			auto obj = std::make_unique<Pointline>(selection.selected);
			objects.push_back(std::move(obj));
		}
		if (!selection.isPolyline()) {
			ImGui::EndDisabled();
		}
	}
}

void UI::RenderSelectedObject() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::BeginChild("PropertiesWindow", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetCursorPos().y - style.WindowPadding.y), true, ImGuiWindowFlags_NoBackground);
	if (objects_selectedRowIdx != -1) {
		Object* selectedObj = objects[objects_selectedRowIdx].get();
		double step = 0.001f;
		double stepFast = 0.1f;
		bool flag = true;

		ImGui::Text("Position");
		gmod::vector3<double> position = selectedObj->position();
		flag = false;
		if (ImGui::InputDouble("X##Position", &position.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			flag = true;
		}
		if (ImGui::InputDouble("Y##Position", &position.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			flag = true;
		}
		if (ImGui::InputDouble("Z##Position", &position.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			flag = true;
		}

		if (flag) {
			selectedObj->SetTranslation(position.x(), position.y(), position.z());
			selectedObj->InformParents();
		}

		ImGui::Text("Euler Angles");
		gmod::vector3<double> eulerAngles = selectedObj->eulerAngles();
		flag = false;
		auto eulerAnglesDeg = gmod::vector3<double>(gmod::rad2deg(eulerAngles.x()), gmod::rad2deg(eulerAngles.y()), gmod::rad2deg(eulerAngles.z()));
		if (ImGui::InputDouble("X##Euler Angles", &eulerAnglesDeg.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			eulerAngles = gmod::vector3<double>(gmod::deg2rad(eulerAnglesDeg.x()), eulerAngles.y(), eulerAngles.z());
			flag = true;
		}
		if (ImGui::InputDouble("Y##Euler Angles", &eulerAnglesDeg.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			eulerAngles = gmod::vector3<double>(eulerAngles.x(), gmod::deg2rad(eulerAnglesDeg.y()), eulerAngles.z());
			flag = true;
		}
		if (ImGui::InputDouble("Z##Euler Angles", &eulerAnglesDeg.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			eulerAngles = gmod::vector3<double>(eulerAngles.x(), eulerAngles.y(), gmod::deg2rad(eulerAnglesDeg.z()));
			flag = true;
		}

		if (flag) {
			switch (currentOrientation) {
				case Orientation::World: {
					selectedObj->SetRotation(eulerAngles.x(), eulerAngles.y(), eulerAngles.z());
					break;
				}
				case Orientation::Cursor: {
					selectedObj->SetRotationAroundPoint(eulerAngles.x(), eulerAngles.y(), eulerAngles.z(), cursor.transform.position());
					break;
				}
				case Orientation::Selection: {
					selectedObj->SetRotationAroundPoint(eulerAngles.x(), eulerAngles.y(), eulerAngles.z(), selection.UpdateMidpoint());
					break;
				}
			}
		}

		ImGui::Text("Scale");
		gmod::vector3<double> scale = selectedObj->scale();
		flag = false;
		if (ImGui::InputDouble("X##Scale", &scale.x(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			flag = true;
		}
		if (ImGui::InputDouble("Y##Scale", &scale.y(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			flag = true;
		}
		if (ImGui::InputDouble("Z##Scale", &scale.z(), step, stepFast, "%.3f", ImGuiInputTextFlags_CharsDecimal)) {
			flag = true;
		}

		if (flag) {
			switch (currentOrientation) {
				case Orientation::World: {
					selectedObj->SetScaling(scale.x(), scale.y(), scale.z());
					break;
				}
				case Orientation::Cursor: {
					selectedObj->SetScalingAroundPoint(scale.x(), scale.y(), scale.z(), cursor.transform.position());
					break;
				}
				case Orientation::Selection: {
					selectedObj->SetScalingAroundPoint(scale.x(), scale.y(), scale.z(), selection.UpdateMidpoint());
					break;
				}
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		selectedObj->RenderProperties();
	}
	ImGui::EndChild();
	selection.UpdateMidpoint();
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