#include "UI.h"
#include "Application.h"
#include "../gmod/utility.h"
#include "Torus.h"
#include "Cube.h"
#include "Point.h"
#include "Polyline.h"
#include <unordered_set>
#include "Curve.h"

using namespace app;

const std::vector<UI::ObjectType> UI::m_objectTypes = { ObjectType::Cube, ObjectType::Torus, ObjectType::Point };
const std::vector<const char*> UI::m_objectTypeNames = { "Cube", "Torus", "Point" };
const std::vector<UI::ObjectGroupType> UI::m_objectGroupTypes = { ObjectGroupType::Polyline, ObjectGroupType::Curve };
const std::vector<const char*> UI::m_objectGroupTypeNames = { "Polyline", "Curve" };

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
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Properties")) {
			RenderProperties();
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
	ImGui::BeginGroup();
	if (ImGui::RadioButton("Neutral", currentMode == Mode::Neutral)) {
		currentMode = Mode::Neutral;
	}
	if (ImGui::RadioButton("Translate", currentMode == Mode::Translate)) {
		currentMode = Mode::Translate;
	}
	if (!selection.Empty()) {
		if (ImGui::RadioButton("Rotate", currentMode == Mode::Rotate)) {
			currentMode = Mode::Rotate;
		}
		if (ImGui::RadioButton("Scale", currentMode == Mode::Scale)) {
			currentMode = Mode::Scale;
		}
	}
	ImGui::EndGroup();
	if (currentMode != Mode::Neutral) {
		ImGui::NextColumn();
		ImGui::BeginGroup();
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
		ImGui::EndGroup();
		ImGui::Columns(1);
	}
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
	ImGui::Combo("##objtypes", &m_selectedObjType, m_objectTypeNames.data(), m_objectTypeNames.size());
	if (ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		auto pos = cursor.transform.position();
		switch (m_objectTypes.at(m_selectedObjType)) {
			case ObjectType::Cube: {
				auto obj = std::make_unique<Cube>(Application::m_cubeModel.get());
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				sceneObjects.push_back(std::move(obj));
				break;
			}
			case ObjectType::Torus: {
				auto obj = std::make_unique<Torus>();
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				sceneObjects.push_back(std::move(obj));
				break;
			}
			case ObjectType::Point: {
				auto obj = std::make_unique<Point>(Application::m_pointModel.get());
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				sceneObjects.push_back(std::move(obj));
				numOfScenePoints++;

				// if objectgroup is selected, add to that group
				std::optional<Object*> opt = selection.Single();
				if (opt.has_value()) {
					ObjectGroup* grp = dynamic_cast<ObjectGroup*>(opt.value());
					if (nullptr != grp) {
						Object* added = sceneObjects.back().get();
						grp->AddObject(added);
					}
				}
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

	ImGui::Text("Object group types:");
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	if (!selection.isPolyline()) {
		ImGui::BeginDisabled();
	}
	ImGui::Combo("##grptypes", &m_selectedObjGrpType, m_objectGroupTypeNames.data(), m_objectGroupTypeNames.size());
	if (ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		auto pos = cursor.transform.position();
		switch (m_objectGroupTypes.at(m_selectedObjGrpType)) {
			case ObjectGroupType::Polyline: {
				auto obj = std::make_unique<Polyline>(selection.objects);
				sceneObjects.push_back(std::move(obj));
				break;
			}
			case ObjectGroupType::Curve: {
				auto obj = std::make_unique<Curve>(selection.objects);
				sceneObjects.push_back(std::move(obj));
				break;
			}
		}
	}
	if (!selection.isPolyline()) {
		ImGui::EndDisabled();
	}
	ImGui::Spacing();
	if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		if (!selection.Empty()) {
			std::unordered_set<int> toDelete;
			toDelete.reserve(selection.objects.size());

			for (auto& obj : selection.objects) {
				toDelete.insert(obj->id);
				if (nullptr != dynamic_cast<Point*>(obj)) {
					numOfScenePoints--;
				}
			}

			std::erase_if(sceneObjects, [&toDelete](const auto& o) {
				return toDelete.contains(o->id);
			});
		}
	}
	ImGui::Spacing();
	ImGui::BeginChild("ObjectTableWindow", ImVec2(0, tableHeight(sceneObjects.size())), false, ImGuiWindowFlags_None);
	if (ImGui::BeginTable("ObjectTable", 2, ImGuiTableFlags_ScrollY)) {
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type");
		ImGui::TableHeadersRow();

		for (auto& obj : sceneObjects) {
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			bool selected = selection.Contains(obj->id);
			if (ImGui::Selectable(obj->name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
				if (ImGui::GetIO().KeyCtrl) {
					if (selected) {
						selection.RemoveObject(obj.get());
					} else {
						selection.AddObject(obj.get());
					}
				} else if (ImGui::GetIO().KeyAlt && nullptr != dynamic_cast<Point*>(obj.get())) {
					std::optional<Object*> opt = selection.Single();
					ObjectGroup* selectedGrp = dynamic_cast<ObjectGroup*>(opt.has_value() ? opt.value() : nullptr);
					if (nullptr != selectedGrp) {
						selectedGrp->AddObject(obj.get());
					}
				} else {
					selection.Clear();
					selection.AddObject(obj.get());
				}
			}
			ImGui::TableNextColumn();
			ImGui::Text(obj->type().c_str());
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void UI::SelectObjectOnMouseClick(Object* obj) {
	selection.AddObject(obj);
}

void UI::RenderProperties() {
	std::optional<Object*> obj = selection.Single();
	Object* selectedObj = nullptr;
	if (obj.has_value()) {
		selectedObj = obj.value();
	} else {
		if (selection.Empty()) { return; }
		selectedObj = &selection;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::BeginChild("PropertiesWindow", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetCursorPos().y - style.WindowPadding.y), true, ImGuiWindowFlags_NoBackground);
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
		}
	}

	if (nullptr == dynamic_cast<Point*>(selectedObj)) {
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
				case Orientation::World:
				{
					selectedObj->SetScaling(scale.x(), scale.y(), scale.z());
					break;
				}
				case Orientation::Cursor:
				{
					selectedObj->SetScalingAroundPoint(scale.x(), scale.y(), scale.z(), cursor.transform.position());
					break;
				}
			}
		}
	}

	ImGui::Spacing();
	ImGui::Separator();
	selectedObj->RenderProperties();

	auto grp = dynamic_cast<ObjectGroup*>(selectedObj);
	// if objectgroup, but not selection
	if (nullptr != grp && grp->id != selection.id) {
		if (grp->Empty()) {
			std::erase_if(sceneObjects, [&grp](const auto& o) {
				return grp->id == o->id;
			});
		}
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
