#pragma once
#include "framework.h"
#include <imgui.h>
#include "Cursor.h"
#include "Object.h"
#include "ObjectGroup.h"
#include "SerializationManager.h"
#include "SurfaceBuilder.h"
#include "Intersection.h"
#include "PathParser.h"
#include "Cutter.h"
#include "Milling.h"
#include "PathAnimator.h"
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
		ImVec4 bkgdColor = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
		ImVec4 slctdColor = ImVec4(1.f, 1.f, 0.f, 1.f);
		ImVec4 stereoCyan = ImVec4(0.f, 1.f, 1.f, 1.f);
		ImVec4 stereoRed = ImVec4(1.f, 0.f, 0.f, 1.f);
		float stereoD = 0.01f;
		float stereoF = 1.f;
		bool showCAD = true;

		// INTERSECTION
		bool updatePreview = false;
		Intersection intersection;

		// CAM
		struct Display {
			std::array<float, 3> color = { 1.f, 1.f, 1.f };
			std::array<float, 3> direction = { 0.f, -1.f, 0.f };
			std::array<float, 3> weights = { 1.f, 1.f, 1.f };
			std::array<float, 3> ambient = { 1.f, 1.f, 1.f };
			std::array<float, 3> diffuse = { 1.f, 1.f, 1.f };
			std::array<float, 3> specular = { 1.f, 1.f, 1.f };
			float shininess = 1.f;
		} display;

		Milling milling;

		UI();
		void Render(bool firstPass, Camera& camera);
	private:
		int m_selectedObjType = 0;
		const static std::vector<ObjectType> m_objectTypes;
		const static std::vector<const char*> m_objectTypeNames;
		int m_selectedObjGrpType = 0;
		const static std::vector<ObjectGroupType> m_objectGroupTypes;
		const static std::vector<const char*> m_objectGroupTypeNames;

		PathAnimator m_pathAnimator;
		PathParser m_pathParser;

		SurfaceBuilder m_surfaceBuilder;
		bool m_showSurfaceBuilder = false;

		SerializationManager m_serializationManager;

		bool m_includePatchBoundaries = true;

		std::string m_intersectionInfo = "Intersection info";
		ImVec4 m_intersectionInfoColor = { 1.f, 1.f, 1.f, 1.f };
		std::pair<Intersection::IDIG, Intersection::IDIG> GetIntersectingSurfaces() const;

		void RenderRightPanel_CAD(bool firstPass, Camera& camera);
		void RenderTransforms();
		void RenderCursor();
		void RenderIntersections();
		void RenderObjectTable();
		void RenderProperties();
		void RenderSettings(bool firstPass);
		void RenderIO_CAD();

		void RenderRightPanel_CAM(bool firstPass, Camera& camera);
		void RenderIO_CAM();

		std::string OpenFileDialog_CAD();
		void LoadJSONFile(const std::string& m_path);
		std::string SaveFileDialog_CAD();
		void SaveScene(const std::string& m_path);

		std::string OpenFileDialog_CAM();
		void LoadPathFile(const std::string& m_path);

		inline int tableHeight(int rows) const {
			const float rowHeight = ImGui::GetTextLineHeightWithSpacing();
			const float headerHeight = ImGui::GetFrameHeightWithSpacing();
			const float tableHeight = (rows * rowHeight) + headerHeight;
			const float maxHeight = ImGui::GetContentRegionAvail().y * 0.75f;
			return std::min(tableHeight, maxHeight);
		}
	};
}