#pragma once
#include "../gmod/vector3.h"
#include "IGeometrical.h"
#include "Polyline.h"
#include <vector>

namespace app {
	class Intersection {
	public:
		std::array<float, 4> color = { 0.0f, 0.0f, 1.0f, 1.0f };
		int intersectionCurveControlPoints = 10;

		bool availible = false;
		bool showUVPlanes = false;
		bool useCursorAsStart = false;
		gmod::vector3<double> cursorPosition;

		double gradientStep = 5 * 1e-2; 
		double gradientTolerance = 5 * 1e-5; 
		int gradientMaxIterations = 100;

		double newtonStep = 0.5;
		double newtonTolerance = 5 * 1e-5;
		int newtonMaxIterations = 5;
		int newtonMaxRepeats = 8;

		int maxIntersectionPoints = 5000;
		double distance = 1e-2;
		double closingPointTolerance = 5 * 1e-3; 

		void RenderUVPlanes();
		void UpdateMesh(const Device& device);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;

		void Clear();
		unsigned int FindIntersection(std::pair<const IGeometrical*, const IGeometrical*> surfaces);
		bool IntersectionCurveAvailible() const;
		void CreateIntersectionCurve(std::vector<std::unique_ptr<Object>>& sceneObjects);
		void CreateInterpolationCurve(std::vector<std::unique_ptr<Object>>& sceneObjects);
	private:
		const int m_gridCells = 8;
		const double m_eps = 1e-12;
		Mesh m_preview;

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
		std::vector<PointOfIntersection> m_pointsOfIntersectionForward;
		std::vector<PointOfIntersection> m_pointsOfIntersectionBackward;
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
