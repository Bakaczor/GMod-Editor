#include "../gmod/utility.h"
#include "Application.h"
#include "BSpline.h"
#include "CISpline.h"
#include "Cube.h"
#include "Point.h"
#include "Polyline.h"
#include "Spline.h"
#include "Gregory.h"
#include "tinyfiledialogs.h"
#include "Torus.h"
#include "UI.h"
#include <fstream>
#include <unordered_set>

using namespace app;

const std::vector<UI::ObjectType> UI::m_objectTypes = { ObjectType::Cube, ObjectType::Torus, ObjectType::Point, ObjectType::Surface, ObjectType::BSurface };
const std::vector<const char*> UI::m_objectTypeNames = { "Cube", "Torus", "Point", "Surface", "BSurface" };
const std::vector<UI::ObjectGroupType> UI::m_objectGroupTypes = { ObjectGroupType::Polyline, ObjectGroupType::Spline, ObjectGroupType::BSpline, ObjectGroupType::CISpline };
const std::vector<const char*> UI::m_objectGroupTypeNames = { "Polyline", "Spline", "BSpline", "CISpline" };

UI::UI() : m_surfaceBuilder(sceneObjects) {}

void UI::Render(bool firstPass, Camera& camera) {
	RenderRightPanel(firstPass, camera);
	RenderSettings(firstPass);
	RenderIO();
}

std::pair<const IGeometrical*, const IGeometrical*> UI::GetIntersectingSurfaces() const {
	if (selection.objects.empty() || selection.objects.size() > 2) {
		return std::make_pair(nullptr, nullptr);
	}
	Object* first = selection.objects.front();
	IGeometrical* gFirst = dynamic_cast<IGeometrical*>(first);
	if (!gFirst) {
		return std::make_pair(nullptr, nullptr);
	}
	if (selection.objects.size() == 1) {
		return std::make_pair(gFirst, nullptr);
	}
	Object* second = selection.objects.back();
	IGeometrical* gSecond = dynamic_cast<IGeometrical*>(second);
	if (!gSecond) {
		return std::make_pair(nullptr, nullptr);
	}
	if (IGeometrical::XYZBoundsIntersect(gFirst->WorldBounds(), gSecond->WorldBounds())) {
		return std::make_pair(gFirst, gSecond);
	}
	return std::make_pair(nullptr, nullptr);
}

void UI::RenderRightPanel(bool firstPass, Camera& camera) {
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
	const float width = 250.f;
	const float height = viewportSize.y;

	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - width, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(1.f);

	ImGui::Begin("Right panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
	RenderTransforms();
	if (ImGui::BeginTabBar("MainTabBar")) {
		if (ImGui::BeginTabItem("Cursor")) {
			RenderCursor();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Objects")) {
			RenderObjectTable();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Intersections")) {
			RenderIntersections();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Properties")) {
			RenderProperties();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	/*ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeight() - ImGui::GetStyle().ItemSpacing.y);
	if (ImGui::Button("Focus", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		std::optional<Object*> opt = selection.Single();
		gmod::vector3<double> pos = opt.has_value() ? opt.value()->position() : cursor.transform.position();
		camera.SetTargetPosition(gmod::vector3<float>(pos.x(), pos.y(), pos.z()));
		camera.cameraChanged = true;
	}*/
	ImGui::End();
}

void UI::RenderTransforms() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImGui::PushStyleColor(ImGuiCol_Header, style.Colors[ImGuiCol_Header]);
	ImGui::BeginChild("TransformsWindow", ImVec2(0, 125), true, ImGuiWindowFlags_NoBackground);

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
	ImGui::EndChild();

	ImGui::BeginChild("OrientationWindow", ImVec2(0, 58), true, ImGuiWindowFlags_NoBackground);
	if (ImGui::CollapsingHeader("Orientation")) {
		ImGui::BeginGroup();
		if (ImGui::RadioButton("World", currentOrientation == Orientation::World)) {
			currentOrientation = Orientation::World;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Cursor", currentOrientation == Orientation::Cursor)) {
			currentOrientation = Orientation::Cursor;
		}
		ImGui::EndGroup();
	}
	ImGui::EndChild();
}

void UI::RenderCursor() {
	ImGui::BeginChild("PropertiesWindow", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_NoBackground);
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

	auto pos = cursor.transform.position();
	auto objType = m_objectTypes.at(m_selectedObjType);

	if (ImGui::Button("Add", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		switch (objType) {
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

				// if ObjectGroup is selected, add to that group
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
			case ObjectType::Surface: {
				m_showSurfaceBuilder = true;
				m_surfaceBuilder.SetC2(false);
				break;
			}
			case ObjectType::BSurface: {
				m_showSurfaceBuilder = true;
				m_surfaceBuilder.SetC2(true);
				break;
			}
		}
	}

	if (m_showSurfaceBuilder) {
		ImGui::OpenPopup("Surface Builder");
		m_showSurfaceBuilder = false;
	}

	if (m_surfaceBuilder.RenderProperties()) {
		if (m_surfaceBuilder.shouldBuild) {
			std::unique_ptr<Object> surface(m_surfaceBuilder.Build());
			surface->SetTranslation(pos.x(), pos.y(), pos.z());
			sceneObjects.push_back(std::move(surface));
		}
		m_surfaceBuilder.Reset();
	}
	ImGui::EndChild();
}

void UI::RenderIntersections() {
	ImGui::BeginChild("IntersectionsWindow", ImVec2(0, ImGui::GetContentRegionAvail().y - ImGui::GetFrameHeightWithSpacing()), true, ImGuiWindowFlags_NoBackground);

	ImGui::Text("Gradient Method Step");
	ImGui::InputDouble("###GradientMethodStep", &intersection.gradientStep, 1e-3, 1e-1, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Gradient Method Tolerance");
	ImGui::InputDouble("###GradientMethodTolerance", &intersection.gradientTolerance, 1e-6, 1e-4, "%.6f", ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Gradient Method Max Iterations");
	ImGui::InputInt("###GradientMethodMaxIterations", &intersection.gradientMaxIterations, 1, 10, ImGuiInputTextFlags_CharsDecimal);

	ImGui::Separator();

	ImGui::Text("Newton Method Step");
	ImGui::InputDouble("###NewtonMethodStep", &intersection.newtonStep, 1e-2, 1e-1, "%.2f", ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Newton Method Tolerance");
	ImGui::InputDouble("###NewtonMethodTolerance", &intersection.newtonTolerance, 1e-6, 1e-4, "%.6f", ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Newton Method Max Iterations");
	ImGui::InputInt("###NewtonMethodMaxIterations", &intersection.newtonMaxIterations, 1, 1, ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Newton Method Max Repeats");
	ImGui::InputInt("###NewtonMethodMaxRepeats", &intersection.newtonMaxRepeats, 1, 1, ImGuiInputTextFlags_CharsDecimal);

	ImGui::Separator();

	ImGui::Text("Max Intersection Points");
	ImGui::InputInt("###MaxIntersectionPoints", &intersection.maxIntersectionPoints, 100, 1000, ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Distance");
	ImGui::InputDouble("###Distance", &intersection.distance, 1e-3, 1e-2, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	ImGui::Text("Closing Point Tolerance");
	ImGui::InputDouble("###ClosingPointTolerance", &intersection.closingPointTolerance, 1e-4, 1e-3, "%.4f", ImGuiInputTextFlags_CharsDecimal);

	ImGui::Separator();

	ImGui::Checkbox("Use Cursor as Start", &intersection.useCursorAsStart);
	ImGui::ColorEdit3("Color", reinterpret_cast<float*>(&intersection.color));

	ImGui::Separator();

	if (ImGui::Button("Find Intersection", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		intersection.Clear();
		m_intersectionInfo = "No intersection";
		auto surfaces = GetIntersectingSurfaces();
		if (surfaces.first != nullptr) {
			if (intersection.useCursorAsStart) {
				intersection.cursorPosition = cursor.transform.position();
			}
			unsigned int res = intersection.FindIntersection(surfaces);
			if (res == 0) {
				m_intersectionInfo = "Intersection found";
				updatePreview = true;
			} else if (res == 1) {
				m_intersectionInfo = "Could not locate start";
			} else if (res == 2) {
				m_intersectionInfo = "Point search failed";
			}
		}
	}
	ImGui::SeparatorText(m_intersectionInfo.c_str());

	if (!intersection.availible) {
		ImGui::BeginDisabled();
	}

	if (ImGui::Button("Show UV Planes", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		intersection.showUVPlanes = true;
	}

	if (intersection.showUVPlanes) {
		intersection.RenderUVPlanes();
	}

	ImGui::Separator();

	ImGui::Text("Intersection Curve Control Points");
	ImGui::InputInt("###IntersectionCurveControlPoints", &intersection.intersectionCurveControlPoints, 1, 10, ImGuiInputTextFlags_CharsDecimal);
	if (ImGui::Button("Create Intersection Curve", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		intersection.CreateIntersectionCurve(sceneObjects);
	}

	if (!intersection.IntersectionCurveAvailible()) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Create Interpolation Curve", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		intersection.CreateInterpolationCurve(sceneObjects);
	}
	if (!intersection.IntersectionCurveAvailible()) {
		ImGui::EndDisabled();
	}

	if (!intersection.availible) {
		ImGui::EndDisabled();
	}

	ImGui::EndChild();
}

void UI::RenderObjectTable() {
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
			case ObjectGroupType::Spline: {
				auto obj = std::make_unique<Spline>(selection.objects);
				sceneObjects.push_back(std::move(obj));
				break;
			}
			case ObjectGroupType::BSpline: {
				auto obj = std::make_unique<BSpline>(selection.objects);
				sceneObjects.push_back(std::move(obj));
				break;
			}
			case ObjectGroupType::CISpline: {
				auto obj = std::make_unique<CISpline>(selection.objects);
				sceneObjects.push_back(std::move(obj));
				break;
			}
		}
	}
	if (!selection.isPolyline()) {
		ImGui::EndDisabled();
	}
	if (ImGui::Button("Collapse", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		if (selection.objects.size() == 2) {
			Object* p1 = selection.objects[0];
			Object* p2 = selection.objects[1];

			static const std::string type = "Point";
			if (p1->type() == type && p2->type() == type) {
				auto pos = (p1->position() + p2->position()) * 0.5f;
				auto obj = std::make_unique<Point>(Application::m_pointModel.get());
				obj->SetTranslation(pos.x(), pos.y(), pos.z());
				p1->ReplaceSelf(obj.get());
				p2->ReplaceSelf(obj.get());
				sceneObjects.push_back(std::move(obj));

				std::erase_if(sceneObjects, [&p1, &p2](const auto& o) {
					return o->id == p1->id || o->id == p2->id;
				});
			}
		}
	}
	if (ImGui::Button("Delete", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		if (!selection.Empty()) {
			std::unordered_set<int> toDelete;
			// toDelete.reserve(selection.objects.size());

			for (auto& obj : selection.objects) {
				if (obj->deletable) {
					toDelete.insert(obj->id);
				}
			}

			std::erase_if(sceneObjects, [&toDelete](const auto& o) {
				return toDelete.contains(o->id);
			});
		}
	}
	if (ImGui::Button("Clear", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		sceneObjects.clear();
		intersection.availible = false;
	}
	ImGui::BeginDisabled();
	ImGui::Checkbox("Include Patch Boundaries", &m_includePatchBoundaries);
	ImGui::EndDisabled();
	bool disable = false;
	std::vector<Surface*> surfaces;
	if (!selection.Empty()) {
		for (const auto& obj : selection.objects) {
			if (obj->type() != "Surface") {
				disable = true;
				break;
			} else {
				surfaces.push_back(dynamic_cast<Surface*>(obj));
			}
		}
	} else {
		disable = true;
	}
	if (disable) {
		ImGui::BeginDisabled();
	}
	if (ImGui::Button("Add Gregory Patch", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f))) {
		auto cycles = Surface::FindCycles3(surfaces, m_includePatchBoundaries);
		for (const auto& cycle3 : cycles) {
			bool correctLength = true;
			for (const auto& edge : cycle3) {
				if (edge.intermediate.size() != 2) {
					correctLength = false;
					break;
				}
			}
			if (correctLength) {
				auto obj = std::make_unique<Gregory>(cycle3, surfaces);
				sceneObjects.push_back(std::move(obj));
			}
		}
	}
	if (disable) {
		ImGui::EndDisabled();
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
			ImGui::PushID(obj->id);
			if (ImGui::Selectable(obj->name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
				if (ImGui::GetIO().KeyCtrl) {
					if (selected) {
						selection.RemoveObject(obj.get());
					} else {
						selection.AddObject(obj.get());
					}
				} else if (ImGui::GetIO().KeyAlt && typeid(Point) == typeid(*obj.get())) {
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
			ImGui::PopID();
			ImGui::TableNextColumn();
			ImGui::Text(obj->type().c_str());
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void UI::SelectObjectOnMouseClick(Object* obj) {
	if (obj == nullptr) {
		if (ImGui::GetIO().KeyShift) {
			selection.Clear();
		}
	} else {
		if (!ImGui::GetIO().KeyCtrl) {
			selection.Clear();
		}
		selection.AddObject(obj);
	}
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
	ImGui::BeginChild("PropertiesWindow", ImVec2(0, ImGui::GetWindowHeight() - ImGui::GetCursorPos().y - style.WindowPadding.y),
		true, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_HorizontalScrollbar);
	if (ImGui::CollapsingHeader("Transform")) {
		float step = 0.001f;
		float stepFast = 0.1f;
		bool flag = false;

		selectedObj->RenderPosition(step, stepFast);

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
				case Orientation::World:
				{
					selectedObj->SetRotation(eulerAngles.x(), eulerAngles.y(), eulerAngles.z());
					break;
				}
				case Orientation::Cursor:
				{
					selectedObj->SetRotationAroundPoint(eulerAngles.x(), eulerAngles.y(), eulerAngles.z(), cursor.transform.position());
					break;
				}
			}
		}

		if (typeid(Point) != typeid(*selectedObj)) {
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
	}

	ImGui::Spacing();
	ImGui::Separator();
	selectedObj->RenderProperties();

	auto grp = dynamic_cast<ObjectGroup*>(selectedObj);
	// if ObjectGroup, but not selection
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
	ImGui::SetNextWindowPos(ImVec2(0.f, 0.f), ImGuiCond_Always);
	ImGui::Begin("settings", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);
	if (firstPass) {
		ImGui::SetNextItemOpen(false);
	}
	if (ImGui::CollapsingHeader("Editor Settings")) {
		ImVec2 size = ImGui::GetWindowSize();
		ImGui::SetWindowSize(ImVec2(size.x, 0.f), ImGuiCond_Always);
		ImGui::ColorEdit3("Background", reinterpret_cast<float*>(&bkgdColor));
		ImGui::ColorEdit3("Selected", reinterpret_cast<float*>(&slctdColor));
		ImGui::Checkbox("Show Grid", &showGrid);
		ImGui::Checkbox("Show Axes", &showAxes);
		ImGui::Checkbox("Hide control points", &hideControlPoints);
		ImGui::Checkbox("Use MMB", &useMMB);
		if (ImGui::Checkbox("Stereoscopic view", &stereoscopicView)) {
			stereoscopicChanged = true;
		}
		if (stereoscopicView) {
			ImGui::SetNextItemWidth(125.f);
			ImGui::InputFloat("d", &stereoD, 0.0001f, 0.001f, "%.4f", ImGuiInputTextFlags_CharsDecimal);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(125.f);
			ImGui::InputFloat("f", &stereoF, 0.001f, 0.01f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
		}
	}
	ImGui::End();
}

void UI::RenderIO() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(ImVec2(0.f, viewport->Size.y - 35.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(0.f, 0.f), ImGuiCond_Always);
	ImGui::Begin("io-buttons", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

	if (ImGui::Button("Save", ImVec2(90.f, 0.f))) {
		std::string file = SaveFileDialog();
		if (!file.empty()) {
			SaveScene(file);
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Load", ImVec2(90.f, 0.f))) {
		std::string file = OpenFileDialog();
		if (!file.empty()) {
			LoadJSONFile(file);
		}
	}	
	ImGui::End();
}

std::string UI::OpenFileDialog() {
	char const* filters[1] = { "*.json" };
	char const* file = tinyfd_openFileDialog(
		"Open JSON file",   // Title
		"",                 // Default dir
		1,                  // Filter count
		filters,            // File extensions
		nullptr,            // Filter description
		0                   // Single select
	);
	return file ? file : "";
}

void UI::LoadJSONFile(const std::string& path) {
	std::ifstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + path);
	}
	try {
		std::string content{ std::istreambuf_iterator<char>{file}, {} };
		m_serializationManager.DeserializeScene(boost::json::parse(content), sceneObjects);
		Object::FixGlobalObjectId(sceneObjects);
	} catch (const std::exception& e) {
		throw std::runtime_error("Deserialization error: " + std::string(e.what()));
	} catch (...) {
		throw std::runtime_error("JSON parse error.");
	}
}

std::string UI::SaveFileDialog() {
	char const* filters[1] = { "*.json" };
	char const* file = tinyfd_saveFileDialog(
		"Save scene as...",   
		"scene.json",                 
		1,                  
		filters,            
		nullptr             
	);
	return file ? file : "";
}

void UI::SaveScene(const std::string& path) {
	std::ofstream file(path);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file for writing: " + path);
	}
	try {
		boost::json::value jsonDoc = m_serializationManager.SerializeScene(sceneObjects);
		file << boost::json::serialize(jsonDoc);
	} catch (const std::exception& e) {
		throw std::runtime_error("Serialization error: " + std::string(e.what()));
	} catch (...) {
		throw std::runtime_error("JSON parse error.");
	}
}
