#include "Application.h"
#include "CISpline.h"
#include "Debug.h"
#include "Intersection.h"
#include <numeric>

using namespace app;

void Intersection::RenderUVPlanes() {
	// UV Planes
	if (ImGui::Button("Hide UV Planes", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		showUVPlanes = false;
	}
}

bool Intersection::IntersectionCurveAvailible() const {
	return m_intersectionPolyline != nullptr;
}

void Intersection::CreateIntersectionCurve(std::vector<std::unique_ptr<Object>>& sceneObjects) {
	std::vector<Object*> controlPoints;
	std::vector<PointOfIntersection> samples;
	if (m_closed) {
		samples = m_pointsOfIntersectionForward;
	} else {
		samples = m_pointsOfIntersectionBackward;
		std::reverse(samples.begin(), samples.end());
		samples.insert(samples.end(), m_pointsOfIntersectionForward.begin(), m_pointsOfIntersectionForward.end());
	}
	{
		auto& first = samples.front();
		auto obj = std::make_unique<Point>(Application::m_pointModel.get());
		obj->SetTranslation(first.pos.x(), first.pos.y(), first.pos.z());
		controlPoints.push_back(obj.get());
		sceneObjects.push_back(std::move(obj));
	}
	const unsigned int n_1 = intersectionCurveControlPoints - 1;
	for (unsigned int i = 1; i < n_1; ++i) {
		const unsigned int idx = std::round((static_cast<double>(i) / n_1) * (samples.size() - 1));
		auto& p = samples.at(idx);

		auto obj = std::make_unique<Point>(Application::m_pointModel.get());
		obj->SetTranslation(p.pos.x(), p.pos.y(), p.pos.z());
		controlPoints.push_back(obj.get());
		sceneObjects.push_back(std::move(obj));
	}
	{
		auto& last = samples.back();
		auto obj = std::make_unique<Point>(Application::m_pointModel.get());
		obj->SetTranslation(last.pos.x(), last.pos.y(), last.pos.z());
		controlPoints.push_back(obj.get());
		sceneObjects.push_back(std::move(obj));
	}
	auto obj = std::make_unique<Polyline>(controlPoints);
	m_intersectionPolyline = obj.get();
	sceneObjects.push_back(std::move(obj));
}

void Intersection::CreateInterpolationCurve(std::vector<std::unique_ptr<Object>>& sceneObjects) {
	if (m_intersectionPolyline != nullptr) {
		auto obj = std::make_unique<CISpline>(m_intersectionPolyline->objects);
		sceneObjects.push_back(std::move(obj));
	}
}

void Intersection::Clear() {
	availible = false;
	m_s1 = nullptr;
	m_s2 = nullptr;
	m_pointsOfIntersectionForward.clear();
	m_intersectionPolyline = nullptr;
}

void Intersection::UpdateMesh(const Device& device) {
	int n = m_pointsOfIntersectionForward.size() + m_pointsOfIntersectionBackward.size();

	std::vector<Vertex_Po> verts;
	verts.reserve(n);
	for (int i = 0; i < m_pointsOfIntersectionForward.size(); ++i) {
		const auto& pos = m_pointsOfIntersectionForward[i].pos;
		verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
	}
	if (!m_closed) {
		for (int i = m_pointsOfIntersectionBackward.size() - 1; i >= 0; --i) {
			const auto& pos = m_pointsOfIntersectionBackward[i].pos;
			verts.push_back({ DirectX::XMFLOAT3(pos.x(), pos.y(), pos.z()) });
		}
	}

	std::vector<USHORT> idxs(n);
	std::iota(idxs.begin(), idxs.end(), 0);

	m_preview.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void Intersection::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Regular).Set(context);
	m_preview.Render(context);
}

unsigned int Intersection::FindIntersection(std::pair<const IGeometrical*, const IGeometrical*> surfaces) {
	m_s1 = surfaces.first;
	m_s2 = surfaces.second;

	const bool selfIntersection = (m_s2 == nullptr);
	if (selfIntersection) { m_s2 = m_s1; }

	UVs bestUVs;
	if (useCursorAsStart) {
		bestUVs = LocalizeStartWithCursor(selfIntersection);
	} else {
		bestUVs = LocalizeStart(selfIntersection);
	}

	DebugPrint("[Starting UVs]", bestUVs.u1, bestUVs.v1, bestUVs.u2, bestUVs.v2);
	auto gradRes = RunGradientMethod(bestUVs);
	if (!gradRes.has_value()) {
		return 1; // failed at gradient
	}

	UVs startUVs = gradRes.value();
	availible = FindPointsOfIntersection(startUVs);
	if (availible) {
		return 0;
	} else {
		return 2;
	}
}

Intersection::UVs Intersection::LocalizeStart(bool selfIntersection) const {
	const auto bounds1 = m_s1->ParametricBounds();
	const auto bounds2 = m_s2->ParametricBounds();

	const double du1 = (bounds1.uMax - bounds1.uMin) / m_gridCells;
	const double dv1 = (bounds1.vMax - bounds1.vMin) / m_gridCells;
	const double du2 = (bounds2.uMax - bounds2.uMin) / m_gridCells;
	const double dv2 = (bounds2.vMax - bounds2.vMin) / m_gridCells;

	double bestDist = std::numeric_limits<double>::max();
	UVs bestUVs{};

	double u1 = bounds1.uMin + 0.5 * du1;
	for (int i = 0; i < m_gridCells; ++i, u1 += du1) {
		double v1 = bounds1.vMin + 0.5 * dv1;
		for (int j = 0; j < m_gridCells; ++j, v1 += dv1) {
			double u2 = bounds2.uMin + 0.5 * du2;
			for (int k = 0; k < m_gridCells; ++k, u2 += du2) {
				double v2 = bounds2.vMin + 0.5 * dv2;
				for (int l = 0; l < m_gridCells; ++l, v2 += dv2) {
					// skip same index cells for self-intersections
					if (selfIntersection && i == k && j == l) { continue; }

					auto p1 = m_s1->Point(u1, v1);
					auto p2 = m_s2->Point(u2, v2);
					double dist = (p1 - p2).length();

					// if user wants to use cursor as hint, weight distance in cursor's direction - replaced
					//if (useCursorAsStart) {
					//	double d1 = (p1 - cursorPosition).length();
					//	double d2 = (p2 - cursorPosition).length();
					//	dist += d1 + d2;
					//}

					if (dist < bestDist) {
						bestDist = dist;
						bestUVs = { u1, v1, u2, v2 };
					}
				}
			}
		}
	}

	return bestUVs;
}

Intersection::UVs Intersection::LocalizeStartWithCursor(bool selfIntersection) const {
	int gridCells = m_gridCells * m_gridCells / 2;

	const auto bounds1 = m_s1->ParametricBounds();
	const auto bounds2 = m_s2->ParametricBounds();

	const double du1 = (bounds1.uMax - bounds1.uMin) / gridCells;
	const double dv1 = (bounds1.vMax - bounds1.vMin) / gridCells;
	const double du2 = (bounds2.uMax - bounds2.uMin) / gridCells;
	const double dv2 = (bounds2.vMax - bounds2.vMin) / gridCells;

	UVs bestUVs{};
	int bestI = 0, bestJ = 0;

	// first surface
	double bestDist = std::numeric_limits<double>::max();
	double u1 = bounds1.uMin + 0.5 * du1;
	for (int i = 0; i < gridCells; ++i, u1 += du1) {
		double v1 = bounds1.vMin + 0.5 * dv1;
		for (int j = 0; j < gridCells; ++j, v1 += dv1) {
			auto p1 = m_s1->Point(u1, v1);
			double d1 = (p1 - cursorPosition).length();

			if (d1 < bestDist) {
				bestDist = d1;
				bestUVs.u1 = u1;
				bestUVs.v1 = v1;
				// save best cell for self-intersections
				bestI = i;
				bestJ = j;
			}
		}
	}

	// second surface
	bestDist = std::numeric_limits<double>::max();
	double u2 = bounds2.uMin + 0.5 * du2;
	for (int k = 0; k < gridCells; ++k, u2 += du2) {
		double v2 = bounds2.vMin + 0.5 * dv2;
		for (int l = 0; l < gridCells; ++l, v2 += dv2) {
			// skip same index cells for self-intersections
			if (selfIntersection && bestI == k && bestJ == l) { continue; }

			auto p2 = m_s2->Point(u2, v2);
			double d2 = (p2 - cursorPosition).length();

			if (d2 < bestDist) {
				bestDist = d2;
				bestUVs.u2 = u2;
				bestUVs.v2 = v2;
			}
		}
	}

	return bestUVs;
}

std::optional<std::pair<double, double>> Intersection::ValidateUVs(double newU, double newV, const IGeometrical* s) {
	const auto bounds = s->ParametricBounds();
	const bool wrapU = s->IsUClosed();
	const bool wrapV = s->IsVClosed();

	const double uMin = bounds.uMin;
	const double uMax = bounds.uMax;
	const double vMin = bounds.vMin;
	const double vMax = bounds.vMax;

	std::pair<double, double> validUVs = { newU, newV };

	if (newU < uMin) {
		if (wrapU) {
			validUVs.first = uMax;
		} else {
			return std::nullopt;
		}
	} else if (newU > uMax) {
		if (wrapU) {
			validUVs.first = uMin;
		} else {
			return std::nullopt;
		}
	}

	if (newV < vMin) {
		if (wrapV) {
			validUVs.second = vMax;
		} else {
			return std::nullopt;
		}
	} else if (newV > vMax) {
		if (wrapV) {
			validUVs.second = vMin;
		} else {
			return std::nullopt;
		}
	}

	return validUVs;
}

std::array<double, 4> Intersection::ComputeGradient(const UVs& uvs, const gmod::vector3<double>& diff) const {
	gmod::vector3<double> dP1u, dP1v;
	m_s1->Tangent(uvs.u1, uvs.v1, &dP1u, &dP1v);

	gmod::vector3<double> dP2u, dP2v;
	m_s2->Tangent(uvs.u2, uvs.v2, &dP2u, &dP2v);

	return {
		 2.0 * dot(diff, dP1u),
		 2.0 * dot(diff, dP1v),
		-2.0 * dot(diff, dP2u),
		-2.0 * dot(diff, dP2v)
	};
}

std::optional<Intersection::UVs> Intersection::RunGradientMethod(UVs bestUVs) const {
	IGeometrical::UVBounds b1 = m_s1->ParametricBounds();
	IGeometrical::UVBounds b2 = m_s2->ParametricBounds();

	bool outsideBounds = false;
	int iter;
	for (iter = 0; iter < gradientMaxIterations; ++iter) {
		auto p1 = m_s1->Point(bestUVs.u1, bestUVs.v1);
		auto p2 = m_s2->Point(bestUVs.u2, bestUVs.v2);
		auto diff = p1 - p2;

		if (diff.length() < gradientTolerance) { break; }

		std::array<double, 4> grad = ComputeGradient(bestUVs, diff);

		UVs newUVs = {
			bestUVs.u1 - gradientStep * grad[0],
			bestUVs.v1 - gradientStep * grad[1],
			bestUVs.u2 - gradientStep * grad[2],
			bestUVs.v2 - gradientStep * grad[3],
		};
		{
			auto validationRes = ValidateUVs(newUVs.u1, newUVs.v1, m_s1);
			if (!validationRes.has_value()) {
				outsideBounds = true;
				break;
			}
			bestUVs.u1 = validationRes.value().first;
			bestUVs.v1 = validationRes.value().second;
		}
		{
			auto validationRes = ValidateUVs(newUVs.u2, newUVs.v2, m_s2);
			if (!validationRes.has_value()) {
				outsideBounds = true;
				break;
			}
			bestUVs.u2 = validationRes.value().first;
			bestUVs.v2 = validationRes.value().second;
		}
	}

	DebugPrint("[Gradient UVs]", bestUVs.u1, bestUVs.v1, bestUVs.u2, bestUVs.v2);
	DebugPrint("[Gradient iterations]", iter);
	if (iter == gradientMaxIterations || outsideBounds) {
		return std::nullopt;
	}

	return bestUVs;
}

gmod::vector3<double> Intersection::Direction(const UVs& uvs) const {
	gmod::vector3<double> du1, dv1, du2, dv2;
	gmod::vector3<double> t1 = m_s1->Tangent(uvs.u1, uvs.v1, &du1, &dv1);
	gmod::vector3<double> t2 = m_s2->Tangent(uvs.u2, uvs.v2, &du2, &dv2);

	const auto np = normalize(cross(du1, dv1));
	const auto nq = normalize(cross(du2, dv2));
	gmod::vector3<double> t = cross(np, nq);

	if (t.length() < m_eps) {
		t = t1 - t2;
		if (t.length() < m_eps) {
			t = t1;
		}
	}
	return normalize(t);
}

gmod::vector4<double> Intersection::Function(const UVs& uvs, const gmod::vector3<double>& P0, const gmod::vector3<double>& t, double d) const {
	const auto P1 = m_s1->Point(uvs.u1, uvs.v1);
	const auto Q1 = m_s2->Point(uvs.u2, uvs.v2);

	return {
		P1.x() - Q1.x(),
		P1.y() - Q1.y(),
		P1.z() - Q1.z(),
		dot(P1 - P0, t) - d
	};
}

std::optional<gmod::matrix4<double>> Intersection::JacobianInverted(const UVs& uvs, const gmod::vector3<double>& t) const {
	gmod::vector3<double> du1, dv1, du2, dv2;
	m_s1->Tangent(uvs.u1, uvs.v1, &du1, &dv1);
	m_s2->Tangent(uvs.u2, uvs.v2, &du2, &dv2);
	du2 = du2 * -1;
	dv2 = dv2 * -1;

	gmod::matrix4<double> J(
		du1.x(), dv1.x(), du2.x(), dv2.x(),
		du1.y(), dv1.y(), du2.y(), dv2.y(),
		du1.z(), dv1.z(), du2.z(), dv2.z(),
		dot(du1, t), dot(dv1, t), 0.0, 0.0
	);

	if (invert(J)) {
		return J;
	} 
	return std::nullopt;
}

std::optional<Intersection::UVs> Intersection::ComputeNewtonStep(const UVs& uvs, const gmod::vector3<double>& P0, const gmod::vector3<double>& t, double d) const {
	auto J = JacobianInverted(uvs, t);
	if (!J.has_value()) { 
		return std::nullopt;
	}
	const gmod::vector4<double> F = Function(uvs, P0, t, d);
	gmod::vector4<double> change =  J.value() * F;
	//DebugPrint("[Change]", change.x(), change.y(), change.z(), change.w());

	UVs newUVs = {
			uvs.u1 - newtonStep * change.x(),
			uvs.v1 - newtonStep * change.y(),
			uvs.u2 - newtonStep * change.z(),
			uvs.v2 - newtonStep * change.w(),
	};
	{
		auto validationRes = ValidateUVs(newUVs.u1, newUVs.v1, m_s1);
		if (!validationRes.has_value()) {
			return std::nullopt;
		}
		newUVs.u1 = validationRes.value().first;
		newUVs.v1 = validationRes.value().second;
	}
	{
		auto validationRes = ValidateUVs(newUVs.u2, newUVs.v2, m_s2);
		if (!validationRes.has_value()) {
			return std::nullopt;
		}
		newUVs.u2 = validationRes.value().first;
		newUVs.v2 = validationRes.value().second;
	}

	return newUVs;
}

std::optional<Intersection::UVs> Intersection::RunNewtonMethod(const UVs& startUVs, int dir) const {
	const gmod::vector3<double> P0 = m_s1->Point(startUVs.u1, startUVs.v1);
	const gmod::vector3<double> t = dir * Direction(startUVs);
	double d = distance;
	int repeats = 0;
	//DebugPrint("[P0]", P0.x(), P0.y(), P0.z());
	//DebugPrint("[t]", t.x(), t.y(), t.z());
	//DebugPrint("[d]", d);

	bool found = false;
	while (true) {
		UVs newUVs = startUVs;
		for (unsigned int i = 0; i < newtonMaxIterations; ++i) {
			auto result = ComputeNewtonStep(newUVs, P0, t, d);
			if (!result.has_value()) { break; }
			newUVs = result.value();

			gmod::vector3<double> P1 = m_s1->Point(newUVs.u1, newUVs.v1);
			gmod::vector3<double> Q1 = m_s2->Point(newUVs.u2, newUVs.v2);
			double error = (Q1 - P1).length();

			if (error < newtonTolerance /* && std::abs(dot(P1 - P0, t) - d) < m_eps */) {
				DebugPrint("[Newton Iteration]", i);
				DebugPrint("[Error]", error);
				found = true;
				break;
			}
		}

		if (found) {
			return newUVs;
		} else {
			d *= 0.5;
			repeats++;
		}

		if (repeats > newtonMaxRepeats) {
			return std::nullopt;
		}
	}
}

bool Intersection::FindPointsOfIntersection(UVs startUVs) {
	m_closed = false;
	m_pointsOfIntersectionForward.push_back({ startUVs, m_s1->Point(startUVs.u1, startUVs.v1) });
	auto& start = m_pointsOfIntersectionForward.front();

	int dir = 1;
	UVs nextUVs = startUVs;
	auto& pointList = m_pointsOfIntersectionForward;
	for (unsigned int p = 0; p < maxIntersectionPoints; ++p) {
		auto result = RunNewtonMethod(nextUVs, dir);
		// reached the end of UV plane
		if (!result.has_value()) {
			// start searching the other direction
			if (dir == 1) {
				DebugPrint("[Switch Iteration]", p);
				dir = -1;
				pointList = m_pointsOfIntersectionBackward;
				continue;
			} else {
				DebugPrint("[End Iteration]", p);
				break; // finish search
			}
		} else {
			nextUVs = result.value();
			pointList.push_back({ nextUVs, m_s1->Point(nextUVs.u1, nextUVs.v1) });

			// let algorithm find some points before checking for loop
			if (dir == 1 && p > 10 && (m_pointsOfIntersectionForward.back().pos - start.pos).length() < closingPointTolerance) {
				DebugPrint("[Closed Iteration]", p);
				m_closed = true;
				break;
			}
		}
	}
	return m_pointsOfIntersectionForward.size() > 1;
}
