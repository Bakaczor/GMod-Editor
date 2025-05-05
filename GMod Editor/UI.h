#pragma once
#include "framework.h"
#include "../imgui/imgui.h"
#include "Object.h"
#include "Cursor.h"
#include "ObjectGroup.h"
#include <memory>

namespace app {
	class Application;

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
		std::vector<std::unique_ptr<Object>> sceneObjects;
		ObjectGroup selection;
		int numOfScenePoints = 0;
		void SelectObjectOnMouseClick(Object* obj);

		Cursor cursor;
		enum class ObjectType {
			Cube, Torus, Point
		};
		enum class ObjectGroupType {
			Polyline, Curve, BSpline, CISpline
		};

		// ORIENTATION
		enum class Orientation {
			World, Cursor
		};
		Orientation currentOrientation = Orientation::World;

		// SETTINGS
		bool useMMB = true;
		bool showGrid = false;
		bool showAxes = false;
		ImVec4 bkgdColor = ImVec4(0.33f, 0.33f, 0.33f, 1.0f);
		ImVec4 slctdColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

		void Render(bool firstPass, Camera& camera);
	private:
		int m_selectedObjType = 0;
		const static std::vector<ObjectType> m_objectTypes;
		const static std::vector<const char*> m_objectTypeNames;
		int m_selectedObjGrpType = 0;
		const static std::vector<ObjectGroupType> m_objectGroupTypes;
		const static std::vector<const char*> m_objectGroupTypeNames;

		void RenderRightPanel(bool firstPass, Camera& camera);
		void RenderTransforms();
		void RenderCursor();
		void RenderObjectTable();
		void RenderProperties();
		void RenderSettings(bool firstPass);

		inline int tableHeight(int rows) const {
			const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
			const float headerHeight = ImGui::GetFrameHeightWithSpacing();
			const float tableHeight = (rows * rowHeight) + headerHeight;
			const float maxHeight = ImGui::GetContentRegionAvail().y * 0.75f;
			return std::min(tableHeight, maxHeight);
		}
	};
}