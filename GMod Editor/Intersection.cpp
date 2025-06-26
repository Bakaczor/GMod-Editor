#include "Application.h"
#include "CISpline.h"
#include "Debug.h"
#include "Intersection.h"
#include <numeric>
#include <queue>

using namespace app;

void Intersection::RenderUVPlanes() {
	ImTextureID texID1, texID2;
	if (showTrimTextures) {
		texID1 = reinterpret_cast<ImTextureID>(uv1TrimTexSRV.get());
		texID2 = reinterpret_cast<ImTextureID>(uv2TrimTexSRV.get());
	} else {
		texID1 = reinterpret_cast<ImTextureID>(m_uv1PrevTexSRV.get());
		texID2 = reinterpret_cast<ImTextureID>(m_uv2PrevTexSRV.get());
	}

	ImGui::SeparatorText("Surface 1 UV Plane");
	ImGui::Image(texID1, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x));

	ImGui::SeparatorText("Surface 2 UV Plane");
	ImGui::Image(texID2, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().x));

	if (ImGui::Button("Switch Textures", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		showTrimTextures = !showTrimTextures;
	}

	if (ImGui::Button("Hide UV Planes", ImVec2(ImGui::GetContentRegionAvail().x, 0.f))) {
		showUVPlanes = false;
	}

	ImGui::SeparatorText("Trimming");

	int current1 = static_cast<int>(m_trimModeS1);
	ImGui::Text("Surface 1 Display");
	if (ImGui::Combo("###Surface1Display", &current1, trimDisplayModes.data(), trimDisplayModes.size())) {
		m_trimModeS1 = static_cast<TrimDisplayMode>(current1);
	}
	ImGui::Text("Surface 2 Display");
	int current2 = static_cast<int>(m_trimModeS2);
	if (ImGui::Combo("###Surface2Display", &current2, trimDisplayModes.data(), trimDisplayModes.size())) {
		m_trimModeS2 = static_cast<TrimDisplayMode>(current2);
	}
}

std::pair<unsigned int, unsigned int> Intersection::GetTrimInfo(int id) const {
	if (id == m_s1ID) {
		return std::make_pair(1, static_cast<unsigned int>(m_trimModeS1));
	} else if (id == m_s2ID) {
		return std::make_pair(2, static_cast<unsigned int>(m_trimModeS2));
	} else {
		return std::make_pair(-1, -1);
	}
}

void Intersection::UpdateUVPlanes(const Device& device) {
	m_uv1Image.clear();
	m_uv2Image.clear();
	m_uv1Image.resize(m_texWidth * m_texHeight * 4, 0);
	m_uv2Image.resize(m_texWidth * m_texHeight * 4, 0);
	std::fill(m_uv1Image.begin(), m_uv1Image.end(), 255);
	std::fill(m_uv2Image.begin(), m_uv2Image.end(), 255);

	auto mapUVToPixel = [&](double u, double v, const IGeometrical::UVBounds& bounds) {
		int x = static_cast<int>((u - bounds.uMin) / (bounds.uMax - bounds.uMin) * (m_texWidth - 1));
		int y = static_cast<int>((v - bounds.vMin) / (bounds.vMax - bounds.vMin) * (m_texHeight - 1));
		return std::pair(x, y);
	};
	for (const auto& p : m_pointsOfIntersection) {
		auto [x1, y1] = mapUVToPixel(p.uvs.u1, p.uvs.v1, m_s1->ParametricBounds());
		auto [x2, y2] = mapUVToPixel(p.uvs.u2, p.uvs.v2, m_s2->ParametricBounds());

		size_t idx1 = 4 * (y1 * m_texWidth + x1);
		size_t idx2 = 4 * (y2 * m_texWidth + x2);

		m_uv1Image[idx1 + 0] = 0;
		m_uv1Image[idx1 + 1] = 0;
		m_uv1Image[idx1 + 2] = 0;
		m_uv1Image[idx1 + 3] = 255;

		m_uv2Image[idx2 + 0] = 0;
		m_uv2Image[idx2 + 1] = 0;
		m_uv2Image[idx2 + 2] = 0;
		m_uv2Image[idx2 + 3] = 255;
	}
	{
		auto tex1 = device.CreateTexture(m_texDesc);
		auto tex2 = device.CreateTexture(m_texDesc);

		device.deviceContext()->UpdateSubresource(tex1.get(), 0, nullptr, m_uv1Image.data(), m_texWidth * 4, 0);
		device.deviceContext()->UpdateSubresource(tex2.get(), 0, nullptr, m_uv2Image.data(), m_texWidth * 4, 0);

		m_uv1PrevTexSRV = device.CreateShaderResourceView(tex1);
		m_uv2PrevTexSRV = device.CreateShaderResourceView(tex2);
	}
	{
		UpdateTrimTexture(m_uv1Image);
		UpdateTrimTexture(m_uv2Image);

		auto tex1 = device.CreateTexture(m_texDesc);
		auto tex2 = device.CreateTexture(m_texDesc);

		device.deviceContext()->UpdateSubresource(tex1.get(), 0, nullptr, m_uv1Image.data(), m_texWidth * 4, 0);
		device.deviceContext()->UpdateSubresource(tex2.get(), 0, nullptr, m_uv2Image.data(), m_texWidth * 4, 0);

		uv1TrimTexSRV = device.CreateShaderResourceView(tex1);
		uv2TrimTexSRV = device.CreateShaderResourceView(tex2);
	}
}

bool Intersection::IntersectionCurveAvailible() const {
	return m_intersectionPolyline != nullptr;
}

void Intersection::CreateIntersectionCurve(std::vector<std::unique_ptr<Object>>& sceneObjects) {
	const size_t totalPoints = m_pointsOfIntersection.size();
	if (totalPoints < 2) return;

	std::vector<double> cumulativeLength(totalPoints, 0.0);
	for (size_t i = 1; i < totalPoints; ++i) {
		cumulativeLength[i] = cumulativeLength[i - 1] + (m_pointsOfIntersection[i].pos - m_pointsOfIntersection[i - 1].pos).length();
	}

	const double totalLength = cumulativeLength.back();
	if (totalLength < m_eps) return;

	const unsigned int numCtrl = intersectionCurveControlPoints;
	std::vector<size_t> selectedIndices;

	selectedIndices.push_back(0); // always include first point
	for (unsigned int i = 1; i < numCtrl - 1; ++i) {
		const double targetLength = (static_cast<double>(i) / (numCtrl - 1)) * totalLength;

		auto it = std::lower_bound(cumulativeLength.begin(), cumulativeLength.end(), targetLength);
		size_t idx = std::distance(cumulativeLength.begin(), it);

		if (idx <= selectedIndices.back()) {
			idx = selectedIndices.back() + 1;
			if (idx >= totalPoints - 1) break;
		}
		selectedIndices.push_back(idx);
	}
	selectedIndices.push_back(totalPoints - 1); // always include last point

	std::vector<Object*> controlPoints;
	for (size_t& idx : selectedIndices) {
		const auto& pos = m_pointsOfIntersection[idx].pos;
		auto obj = std::make_unique<Point>(Application::m_pointModel.get());
		obj->SetTranslation(pos.x(), pos.y(), pos.z());
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

void Intersection::UpdateTrimTexture(std::vector<uint8_t>& img) {
	ThickenCurve(img);
	auto start = FindStartingPixel(img);
	if (!start.has_value()) { return; }
	auto [sx, sy] = start.value();
	FloodFill(img, sx, sy);
}

void Intersection::ThickenCurve(std::vector<uint8_t>& img) const {
	std::vector<uint8_t> copy = img;

	auto isBlack = [](const uint8_t* pixel) {
		return pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0;
	};

	auto setBlack = [](uint8_t* pixel) {
		pixel[0] = pixel[1] = pixel[2] = 0;
		pixel[3] = 255;
	};

	for (int y = 1; y < m_texHeight - 1; ++y) {
		for (int x = 1; x < m_texWidth - 1; ++x) {
			size_t idx = 4 * (y * m_texWidth + x);
			if (isBlack(&copy[idx])) {
				setBlack(&img[4 * ((y - 1) * m_texWidth + x)]);     // Up
				setBlack(&img[4 * ((y + 1) * m_texWidth + x)]);     // Down
				setBlack(&img[4 * (y * m_texWidth + (x - 1))]);     // Left
				setBlack(&img[4 * (y * m_texWidth + (x + 1))]);     // Right
			}
		}
	}
}

bool Intersection::IsBlack(std::vector<uint8_t>& img, int x, int y) const {
	if (x < 0 || x >= m_texWidth || y < 0 || y >= m_texHeight) { return true; }
	size_t idx = 4 * (y * m_texWidth + x);
	return img[idx + 0] == 0 && img[idx + 1] == 0 && img[idx + 2] == 0;
}

std::optional<std::pair<int, int>> Intersection::FindStartingPixel(std::vector<uint8_t>& img) const {
	for (int y = 0; y < m_texHeight; ++y) {
		for (int x = 0; x < m_texWidth; ++x) {
			size_t idx = 4 * (y * m_texWidth + x);
			if (img[idx + 0] == 255 && img[idx + 1] == 255 && img[idx + 2] == 255) {
				return std::make_pair(x, y);
			}
		}
	}
	return std::nullopt;
}

void Intersection::FloodFill(std::vector<uint8_t>& img, int startX, int startY) {
	auto fill = [&](int x, int y) -> void {
		size_t idx = 4 * (y * m_texWidth + x);
		img[idx + 0] = 0;
		img[idx + 1] = 0;
		img[idx + 2] = 0;
		img[idx + 3] = 255;
	};

	std::queue<std::pair<int, int>> q;
	std::vector<bool> visited(m_texWidth * m_texHeight, false);

	q.push({ startX, startY });
	visited[startY * m_texWidth + startX] = true;
	fill(startX, startY);

	const std::array<std::pair<int, int>, 4> neighbors = { {
		{ 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
	} };

	while (!q.empty()) {
		auto [x, y] = q.front();
		q.pop();

		for (auto [dx, dy] : neighbors) {
			int nx = x + dx, ny = y + dy;
			if (nx < 0 || ny < 0 || nx >= m_texWidth || ny >= m_texHeight) { continue; }

			size_t idx = ny * m_texWidth + nx;
			if (visited[idx]) { continue; }

			if (!IsBlack(img, nx, ny)) {
				visited[idx] = true;
				fill(nx, ny);
				q.push({ nx, ny });
			}
		}
	}
}

Intersection::Intersection() : m_texDesc(m_texWidth, m_texHeight) {}

void Intersection::Clear() {
	availible = false;
	m_s1 = nullptr;
	m_s2 = nullptr;
	m_pointsOfIntersection.clear();
	m_intersectionPolyline = nullptr;
	m_uv1Image.clear();
	m_uv2Image.clear();
	m_s1ID = -1;
	m_s2ID = -1;
}

void Intersection::UpdateMesh(const Device& device) {
	std::vector<Vertex_Po> verts;
	verts.reserve(m_pointsOfIntersection.size());
	for (auto& p : m_pointsOfIntersection) {
		verts.push_back({ DirectX::XMFLOAT3(p.pos.x(), p.pos.y(), p.pos.z()) });
	}

	std::vector<USHORT> idxs(m_pointsOfIntersection.size());
	std::iota(idxs.begin(), idxs.end(), 0);

	m_preview.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
}

void Intersection::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Regular).Set(context);
	m_preview.Render(context);
}

unsigned int Intersection::FindIntersection(std::pair<Intersection::IDIG, Intersection::IDIG> surfaces) {
	m_s1ID = surfaces.first.id;
	m_s1 = surfaces.first.s;
	m_s2ID = surfaces.second.id;
	m_s2 = surfaces.second.s;

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
			validUVs.first = uMax - std::numeric_limits<double>::epsilon();
		} else {
			return std::nullopt;
		}
	} else if (newU > uMax) {
		if (wrapU) {
			validUVs.first = uMin + std::numeric_limits<double>::epsilon();
		} else {
			return std::nullopt;
		}
	}

	if (newV < vMin) {
		if (wrapV) {
			validUVs.second = vMax - std::numeric_limits<double>::epsilon();
		} else {
			return std::nullopt;
		}
	} else if (newV > vMax) {
		if (wrapV) {
			validUVs.second = vMin + std::numeric_limits<double>::epsilon();
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
	std::vector<PointOfIntersection> pointsOfIntersectionForward;
	std::vector<PointOfIntersection> pointsOfIntersectionBackward;

	m_closed = false;
	pointsOfIntersectionForward.push_back({ startUVs, m_s1->Point(startUVs.u1, startUVs.v1) });
	auto start = pointsOfIntersectionForward[0].pos;

	int dir = 1;
	UVs nextUVs = startUVs;
	auto& pointList = pointsOfIntersectionForward;
	for (unsigned int p = 0; p < maxIntersectionPoints; ++p) {
		auto result = RunNewtonMethod(nextUVs, dir);
		// reached the end of UV plane
		if (!result.has_value()) {
			// start searching the other direction
			if (dir == 1) {
				DebugPrint("[Switch Iteration]", p);
				dir = -1;
				pointList = pointsOfIntersectionBackward;
				continue;
			} else {
				DebugPrint("[End Iteration]", p);
				break; // finish search
			}
		} else {
			nextUVs = result.value();
			pointList.push_back({ nextUVs, m_s1->Point(nextUVs.u1, nextUVs.v1) });

			// let algorithm find some points before checking for loop
			if (dir == 1 && p > 10 && (pointsOfIntersectionForward.back().pos - start).length() < closingPointTolerance) {
				DebugPrint("[Closed Iteration]", p);
				m_closed = true;
				break;
			}
		}
	}

	if (m_closed) {
		m_pointsOfIntersection = pointsOfIntersectionForward;
	} else {
		m_pointsOfIntersection = pointsOfIntersectionBackward;
		std::reverse(m_pointsOfIntersection.begin(), m_pointsOfIntersection.end());
		m_pointsOfIntersection.insert(m_pointsOfIntersection.end(), pointsOfIntersectionForward.begin(), pointsOfIntersectionForward.end());
	}

	return pointsOfIntersectionForward.size() > 1;
}
