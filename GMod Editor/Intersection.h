#pragma once
#include <vector>
#include "../gmod/vector3.h"
#include "Polyline.h"
#include "IGeometrical.h"

namespace app {
	class Intersection {
	public:
		bool closed = false;
		std::vector<gmod::vector3<double>> pointsOfIntersection;

		int intersectionCurveSamples = 10;
		Polyline* intersectionCurve = nullptr;

		bool availible = false;
		bool showUVPlanes = false;
		bool useCursorAsStart = false;
		gmod::vector3<double> cursorPosition;

		double gradientStep = 1.0;
		int gradientMaxIterations = 1000;
		double gradientTolerance = 1e-3;

		double newtonStep = 1e-3;
		int newtonMaxIterations = 4;
		double newtonTolerance = 1e-3;

		unsigned int FindIntersection(std::pair<const IGeometrical*, const IGeometrical*> surfaces);
		void Clear();
	private:
		const static int m_gridU = 10;
		const static int m_gridV = 10;
		const static double m_boundMarginPercent;
		const static double m_uvChangeMultiplayer;
		const static unsigned int m_maxIntersections = 2000;

		static std::array<double, 4> ComputeGradient(const IGeometrical* s1, const IGeometrical* s2, double u1, double v1, double u2, double v2, gmod::vector3<double>& diff);
		static bool IsInBounds(double u, double v, const IGeometrical::UVBounds& b, double margin = 0.0);
		
		static gmod::vector3<double> Direction(const gmod::vector3<double>& du1, const gmod::vector3<double>& du2, const gmod::vector3<double>& dv1, const gmod::vector3<double>& dv2);
		struct NewtonData {
			const IGeometrical* s;
			double u;
			double v;
			double uNew;
			double vNew;
		};
		static gmod::vector4<double> Function(NewtonData& data1, NewtonData& data2, double dist);
		static std::optional<gmod::matrix4<double>> JacobianInverted(NewtonData& data1, NewtonData& data2);
		static std::optional<gmod::vector4<double>> ComputeNewtonStep(NewtonData& data1, NewtonData& data2, double dist);

		unsigned int RunGradientMethod(const IGeometrical* s1, const IGeometrical* s2, double u1, double v1, double u2, double v2);
		bool ComputeNewUV(NewtonData& data, double uChange, double vChange, bool backtracked);

		bool RunNewtonMethod(NewtonData& d1, NewtonData& d2);
		unsigned int FindIntersectionPoints(const IGeometrical* s1, const IGeometrical* s2, double u1, double v1, double u2, double v2);
	};
}
