#pragma once
#include "framework.h"
#include "../imgui/imgui.h"
#include "Object.h"
#include "Cursor.h"
#include "ObjectGroup.h"
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
	int objects_selectedObjId = -1;
	int objects_selectedRowIdx = -1;
	std::vector<std::unique_ptr<Object>> objects;

	int selection_selectedObjId = -1;
	int selection_selectedRowIdx = -1;
	ObjectGroup selection;
	int numOfPointObjects = 0;
	void SelectObjectOnMouseClick(Object* obj);

	Cursor cursor;
	enum class ObjectType {
		Cube, Torus, Point
	};

	// ORIENTATION
	enum class Orientation {
		World, Cursor, Selection
	};
	Orientation currentOrientation = Orientation::World;

	// SETTINGS
	bool useMMB = true;
	bool showGrid = false;
	bool showAxes = false;
	ImVec4 bkgdColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);

	void Render(bool firstPass);

	inline bool noObjectSelected() const {
		return objects_selectedObjId == -1;
	}
private:
	int m_selectedObjType = 0;
	std::vector<ObjectType> m_objectTypes = { ObjectType::Cube, ObjectType::Torus, ObjectType::Point };
	std::vector<const char*> m_objectTypeNames = { "Cube", "Torus", "Point" };

	void RenderRightPanel(bool firstPass);
	void RenderTransforms();
	void RenderCursor();
	void RenderObjectTable(bool firstPass);
	void RenderSelection(bool firstPass);
	void RenderSelectedObject();
	void RenderSettings(bool firstPass);

	inline int tableHeight(int rows) const {
		const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
		const float headerHeight = ImGui::GetFrameHeightWithSpacing();
		const float tableHeight = (rows * rowHeight) + headerHeight;
		const float maxHeight = ImGui::GetContentRegionAvail().y * 0.5f;
		return std::min(tableHeight, maxHeight);
	}
};