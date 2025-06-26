#pragma once
#include "../gmod/vector3.h"
#include "IGeometrical.h"
#include "Polyline.h"
#include <vector>

namespace app {
	class Intersection {
	public:
		// TRIMMING
		enum class TrimDisplayMode {
			Whole, Inside, Outside
		};
		const std::array<const char*, 3> trimDisplayModes = { "Whole", "Inside", "Outside" };
		mini::dx_ptr<ID3D11ShaderResourceView> uv1TrimTexSRV;
		mini::dx_ptr<ID3D11ShaderResourceView> uv2TrimTexSRV;
		bool reupload = false;

		std::array<float, 4> color = { 0.0f, 0.0f, 1.0f, 1.0f };
		int intersectionCurveControlPoints = 10;

		bool availible = false;
		bool showUVPlanes = false;
		bool showTrimTextures = false;
		bool useCursorAsStart = false;
		gmod::vector3<double> cursorPosition;

		double gradientStep = 5 * 1e-2; 
		double gradientTolerance = 5 * 1e-5; 
		int gradientMaxIterations = 1000;

		double newtonStep = 0.5;
		double newtonTolerance = 5 * 1e-4;
		int newtonMaxIterations = 10;
		int newtonMaxRepeats = 6;

		int maxIntersectionPoints = 5000;
		double distance = 1e-2;
		double closingPointTolerance = 5 * 1e-3; 

		void UpdateMesh(const Device& device);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;
		void UpdateUVPlanes(const Device& device);
		void ReUploadUVPlanes(const Device& device);
		void RenderUVPlanes();
		std::pair<unsigned int, unsigned int> GetTrimInfo(int id) const;

		Intersection();
		void Clear();
		struct IDIG {
			int id = -1;
			const IGeometrical* s = nullptr;
		};
		unsigned int FindIntersection(std::pair<IDIG, IDIG> surfaces);
		bool IntersectionCurveAvailible() const;
		void CreateIntersectionCurve(std::vector<std::unique_ptr<Object>>& sceneObjects);
		void CreateInterpolationCurve(std::vector<std::unique_ptr<Object>>& sceneObjects);
	private:
		const int m_gridCells = 8;
		const double m_eps = 1e-12;
		Mesh m_preview;

		// UV PLANES
		const UINT m_texWidth = 256U;
		const UINT m_texHeight = 256U;
		const Texture2DDescription m_texDesc;
		std::vector<uint8_t> m_uv1Image;
		std::vector<uint8_t> m_uv2Image;
		mini::dx_ptr<ID3D11ShaderResourceView> m_uv1PrevTexSRV;
		mini::dx_ptr<ID3D11ShaderResourceView> m_uv2PrevTexSRV;

		void UpdateTrimTexture(std::vector<uint8_t>& img);
		void UpdateTrimTextureAt(std::vector<uint8_t>& img, int startX, int startY);
		void ThickenCurve(std::vector<uint8_t>& img) const;
		bool IsBlack(std::vector<uint8_t>& img, int x, int y) const;
		std::optional<std::pair<int, int>> FindStartingPixel(std::vector<uint8_t>& img) const;
		void FloodFill(std::vector<uint8_t>& img, int startX, int startY);
		void RenderClickableTexture(std::vector<uint8_t>& img, const ImTextureID& texID, const std::string& label);

		// TRIMMING
		int m_s1ID = -1;
		int m_s2ID = -1;
		TrimDisplayMode m_trimModeS1 = TrimDisplayMode::Whole;
		TrimDisplayMode m_trimModeS2 = TrimDisplayMode::Whole;

		const IGeometrical* m_s1 = nullptr;
		const IGeometrical* m_s2 = nullptr;
		struct UVs {
			double u1;
			double v1; 
			double u2; 
			double v2;
		};

		bool m_closed = false;
		struct PointOfIntersection {
			UVs uvs;
			gmod::vector3<double> pos;
		};
		std::vector<PointOfIntersection> m_pointsOfIntersection;
		Polyline* m_intersectionPolyline = nullptr;

		UVs LocalizeStart(bool selfIntersection) const;
		UVs LocalizeStartWithCursor(bool selfIntersection) const;

		static std::optional<std::pair<double, double>> ValidateUVs(double newU, double newV, const IGeometrical* s);
		std::array<double, 4> ComputeGradient(const UVs& uvs, const gmod::vector3<double>& diff) const;
		std::optional<UVs> RunGradientMethod(UVs bestUVs) const;
		
		gmod::vector3<double> Direction(const UVs& uvs) const;
		gmod::vector4<double> Function(const UVs& uvs, const gmod::vector3<double>& P0, const gmod::vector3<double>& t, double d) const;
		std::optional<gmod::matrix4<double>> JacobianInverted(const UVs& uvs, const gmod::vector3<double>& t) const;
		std::optional<UVs> ComputeNewtonStep(const UVs& uvs, const gmod::vector3<double>& P0, const gmod::vector3<double>& t, double d) const;
		std::optional<Intersection::UVs> RunNewtonMethod(const UVs& startUVs, int dir) const;

		bool FindPointsOfIntersection(UVs startUVs);
	};
}
