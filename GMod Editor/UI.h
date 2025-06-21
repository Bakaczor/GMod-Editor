#pragma once
#include <imgui.h>
#include "Cursor.h"
#include "framework.h"
#include "Object.h"
#include "ObjectGroup.h"
#include "SerializationManager.h"
#include "SurfaceBuilder.h"
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
		void SelectObjectOnMouseClick(Object* obj);

		Cursor cursor;
		enum class ObjectType {
			Cube, Torus, Point, Surface, BSurface
		};
		enum class ObjectGroupType {
			Polyline, Spline, BSpline, CISpline
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
		bool hideControlPoints = false;
		bool stereoscopicView = false;
		bool stereoscopicChanged = false;
		ImVec4 bkgdColor = ImVec4(0.2f, 0.2f, 0.2f, 0.f);
		ImVec4 slctdColor = ImVec4(1.f, 1.f, 0.f, 1.f);
		ImVec4 stereoCyan = ImVec4(0.f, 1.f, 1.f, 1.f);
		ImVec4 stereoRed = ImVec4(1.f, 0.f, 0.f, 1.f);
		float stereoD = 0.01f;
		float stereoF = 1.f;

		UI();
		void Render(bool firstPass, Camera& camera);
	private:
		int m_selectedObjType = 0;
		const static std::vector<ObjectType> m_objectTypes;
		const static std::vector<const char*> m_objectTypeNames;
		int m_selectedObjGrpType = 0;
		const static std::vector<ObjectGroupType> m_objectGroupTypes;
		const static std::vector<const char*> m_objectGroupTypeNames;

		SurfaceBuilder m_surfaceBuilder;
		bool m_showSurfaceBuilder = false;

		SerializationManager m_serializationManager;

		bool m_includePatchBoundaries = true;

		void RenderRightPanel(bool firstPass, Camera& camera);
		void RenderTransforms();
		void RenderCursor();
		void RenderObjectTable();
		void RenderProperties();
		void RenderSettings(bool firstPass);
		void RenderIO();

		std::string OpenFileDialog();
		void LoadJSONFile(const std::string& path);
		std::string SaveFileDialog();
		void SaveScene(const std::string& path);

		inline int tableHeight(int rows) const {
			const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
			const float headerHeight = ImGui::GetFrameHeightWithSpacing();
			const float tableHeight = (rows * rowHeight) + headerHeight;
			const float maxHeight = ImGui::GetContentRegionAvail().y * 0.75f;
			return std::min(tableHeight, maxHeight);
		}
	};
}