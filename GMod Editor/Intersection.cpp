#include "Intersection.h"

using namespace app;

const double Intersection::m_boundMarginPercent = 0.1;
const double Intersection::m_uvChangeMultiplayer = 1.0;

unsigned int Intersection::FindIntersection(std::pair<const IGeometrical*, const IGeometrical*> surfaces) {
	const IGeometrical* s1 = surfaces.first;
	const IGeometrical* s2 = surfaces.second;

	const bool selfIntersection = (s2 == nullptr);
	if (selfIntersection) { s2 = s1; }

	const auto bounds1 = s1->ParametricBounds();
	const auto bounds2 = s2->ParametricBounds();

	const double du1 = (bounds1.uMax - bounds1.uMin) / m_gridU;
	const double dv1 = (bounds1.vMax - bounds1.vMin) / m_gridV;
	const double du2 = (bounds2.uMax - bounds2.uMin) / m_gridU;
	const double dv2 = (bounds2.vMax - bounds2.vMin) / m_gridV;

	double bestDist = std::numeric_limits<double>::max();
	std::pair<std::pair<double, double>, std::pair<double, double>> bestUVs;

	double u1 = bounds1.uMin + 0.5 * du1;
	for (int i = 0; i < m_gridU; ++i, u1 += du1) {
		double v1 = bounds1.vMin + 0.5 * dv1;
		for (int j = 0; j < m_gridV; ++j, v1 += dv1) {
			double u2 = bounds2.uMin + 0.5 * du2;
			for (int k = 0; k < m_gridU; ++k, u2 += du2) {
				double v2 = bounds2.vMin + 0.5 * dv2;
				for (int l = 0; l < m_gridV; ++l, v2 += dv2) {
					// skip same index cells for self-intersections
					if (selfIntersection && i == k && j == l) { continue; }

					auto p1 = s1->Point(u1, v1);
					auto p2 = s2->Point(u2, v2);
					double dist = (p1 - p2).length();

					// if user wants to use cursor as hint, weight distance in cursor's direction
					if (useCursorAsStart) {
						double d1 = (p1 - cursorPosition).length();
						double d2 = (p2 - cursorPosition).length();
						dist += d1 + d2;
					}

					if (dist < bestDist) {
						bestDist = dist;
						bestUVs = { { u1, v1 }, { u2, v2 } };
					}
				}
			}
		}
	}

	return RunGradientMethod(s1, s2, bestUVs.first.first, bestUVs.first.second, bestUVs.second.first, bestUVs.second.second);
}

void Intersection::Clear() {
	availible = false;
	pointsOfIntersection.clear();
	intersectionCurve = nullptr;
}

std::array<double, 4> Intersection::ComputeGradient(const IGeometrical* s1, const IGeometrical* s2, double u1, double v1, double u2, double v2, gmod::vector3<double>& diff) {
	gmod::vector3<double> dP1u, dP1v;
	s1->Tangent(u1, v1, &dP1u, &dP1v);

	gmod::vector3<double> dP2u, dP2v;
	s2->Tangent(u2, v2, &dP2u, &dP2v);

	return {
		 2.0 * gmod::dot(diff, dP1u),
		 2.0 * gmod::dot(diff, dP1v),
		-2.0 * gmod::dot(diff, dP2u),
		-2.0 * gmod::dot(diff, dP2v)
	};
}

unsigned int Intersection::RunGradientMethod(const IGeometrical* s1, const IGeometrical* s2, double u1, double v1, double u2, double v2) {
	IGeometrical::UVBounds b1 = s1->ParametricBounds();
	IGeometrical::UVBounds b2 = s2->ParametricBounds();

	bool outsideBounds = false;
	int iter;
	for (iter = 0; iter < gradientMaxIterations; ++iter) {
		auto p1 = s1->Point(u1, v1);
		auto p2 = s2->Point(u2, v2);
		auto diff = p1 - p2;

		if (diff.length() < gradientTolerance) { break; }

		std::array<double, 4> grad = ComputeGradient(s1, s2, u1, v1, u2, v2, diff);

		u1 -= gradientStep * grad[0];
		v1 -= gradientStep * grad[1];
		u2 -= gradientStep * grad[2];
		v2 -= gradientStep * grad[3];

		if (!IsInBounds(u1, v1, b1, m_boundMarginPercent) || !IsInBounds(u2, v2, b2, m_boundMarginPercent)) {
			outsideBounds = true;
			break;
		}
	}

	if (iter == gradientMaxIterations || outsideBounds) {
		return 1;  // bad gradient parameters
	}

	return FindIntersectionPoints(s1, s2, u1, v1, u2, v2);
}

bool Intersection::IsInBounds(double u, double v, const IGeometrical::UVBounds& b, double margin) {
	double umin = b.uMin - margin * (b.uMax - b.uMin);
	double umax = b.uMax + margin * (b.uMax - b.uMin);
	double vmin = b.vMin - margin * (b.vMax - b.vMin);
	double vmax = b.vMax + margin * (b.vMax - b.vMin);
	return (u >= umin && u <= umax && v >= vmin && v <= vmax);
}

gmod::vector3<double> Intersection::Direction(const gmod::vector3<double>& du1, const gmod::vector3<double>& du2, const gmod::vector3<double>& dv1, const gmod::vector3<double>& dv2) {
	const auto np = normalize(cross(du1, dv1));
	const auto nq = normalize(cross(du2, dv2));
	return normalize(cross(np, nq));
}

gmod::vector4<double> Intersection::Function(NewtonData& data1, NewtonData& data2, double dist) {
	const auto P0 = data1.s->Point(data1.u, data1.v);
	const auto P1 = data1.s->Point(data1.uNew, data1.vNew);
	const auto Q1 = data2.s->Point(data2.uNew, data2.vNew);

	gmod::vector3<double> du1, dv1, du2, dv2;
	data1.s->Tangent(data1.u, data1.v, &du1, &dv1);
	data2.s->Tangent(data2.u, data2.v, &du2, &dv2);

	const auto t = Direction(du1, du2, dv1, dv2);

	return {
		P1.x() - Q1.x(),
		P1.y() - Q1.y(),
		P1.z() - Q1.z(),
		dot(P1 - P0, t) - dist
	};
}

std::optional<gmod::matrix4<double>> Intersection::JacobianInverted(NewtonData& data1, NewtonData& data2) {
	gmod::vector3<double> du1, dv1, du2, dv2;
	data1.s->Tangent(data1.u, data1.v, &du1, &dv1);
	data2.s->Tangent(data2.u, data2.v, &du2, &dv2);

	const auto t = Direction(du1, du2, dv1, dv2);

	data1.s->Tangent(data1.uNew, data1.vNew, &du1, &dv1);
	data2.s->Tangent(data2.uNew, data2.vNew, &du2, &dv2);
	du2 = du2 * -1;
	dv2 = dv2 * -1;

	const double dot1 = dot(du1, t);
	const double dot2 = dot(dv1, t);

	gmod::matrix4<double> J(
		du1.x(), dv1.x(), du2.x(), dv2.x(),
		du1.y(), dv1.y(), du2.y(), dv2.y(),
		du1.z(), dv1.z(), du2.z(), dv2.z(),
		dot1, dot2, 0.0, 0.0
	);

	if (invert(J)) {
		return J;
	} 
	return std::nullopt;
}

std::optional<gmod::vector4<double>> Intersection::ComputeNewtonStep(NewtonData& data1, NewtonData& data2, double dist) {
	auto J = JacobianInverted(data1, data2);
	if (J.has_value()) {
		const gmod::vector4<double> F = Function(data1, data2, dist);
		return J.value() * F;
	}
	return std::nullopt;
}

bool Intersection::ComputeNewUV(NewtonData& data, double uChange, double vChange, bool backtracked) {
	const auto bounds = data.s->ParametricBounds();
	const bool wrapU = data.s->IsUClosed();
	const bool wrapV = data.s->IsVClosed();

	const double uMin = bounds.uMin;
	const double uMax = bounds.uMax;
	const double vMin = bounds.vMin;
	const double vMax = bounds.vMax;

	double uNew = data.u - uChange * m_uvChangeMultiplayer;
	double vNew = data.v - vChange * m_uvChangeMultiplayer;

	bool outOfBounds = false;

	if (uNew < uMin) {
		if (wrapU) {
			uNew = uMax;
		} else {
			uNew = uMin;
			if (backtracked) {
				return true; // give up
			}
			outOfBounds = true;
		}
	} else if (uNew > uMax) {
		if (wrapU) {
			uNew = uMin;
		} else {
			uNew = uMax;
			if (backtracked) {
				return true; // give up
			}
			outOfBounds = true;
		}
	}

	if (vNew < vMin) {
		if (wrapV) {
			vNew = vMax;
		} else {
			vNew = vMin;
			if (backtracked) {
				return true; // give up
			}
			outOfBounds = true;
		}
	} else if (vNew > vMax) {
		if (wrapV) {
			vNew = vMin;
		} else {
			vNew = vMax;
			if (backtracked) {
				return true; // give up
			}
			outOfBounds = true;
		}
	}

	data.uNew = uNew;
	data.vNew = vNew;

	return outOfBounds;
}

bool Intersection::RunNewtonMethod(NewtonData& d1, NewtonData& d2) {
	double dist = newtonStep;
	bool backtrack = false;
	bool failed = false;

	while (true) {
		for (unsigned int i = 0; i < newtonMaxIterations; ++i) {
			auto change = ComputeNewtonStep(d1, d2, dist);
			// Jacobian not invertible
			if (!change.has_value()) {
				if (backtrack) {
					failed = true;
				}
				backtrack = true;
				break;
			}

			const gmod::vector4<double> delta = change.value();

			// update UVs
			bool outOfBounds1 = ComputeNewUV(d1, delta[0], delta[1], backtrack);
			bool outOfBounds2 = ComputeNewUV(d2, delta[2], delta[3], backtrack);

			if (outOfBounds1 || outOfBounds2) {
				if (backtrack) {
					failed = true;
				}
				backtrack = true;
				break;
			}
		}

		if (failed) {
			return !failed;
		}

		gmod::vector3<double> p1 = d1.s->Point(d1.u, d1.v);
		gmod::vector3<double> p2 = d2.s->Point(d2.u, d2.v);
		double error = (p2 - p1).length();

		if (error < newtonTolerance) {
			return true;
		} else {
			dist *= 0.5;
			backtrack = true;
		}

		if (backtrack) {
			// do something
		}
	}
}

unsigned int Intersection::FindIntersectionPoints(const IGeometrical* s1, const IGeometrical* s2, double u1, double v1, double u2, double v2) {
	NewtonData d1{ s1, u1, v1, u1, v1 };
	NewtonData d2{ s2, u2, v2, u2, v2 };

	bool finished = false;
	for (unsigned int loops = 0; loops < m_maxIntersections; ++loops) {
		if (finished) { 
			return 0;
		}
		bool result = RunNewtonMethod(d1, d2);

		if (!result) {
			return 2; // bad newton parameters
		} 

		// check stopping conditions (distance to start, iteration count, etc.)
		// if met: finished = true;
	}
}
