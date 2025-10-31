#include "framework.h"
#include <imgui.h>
#include <unordered_set>
#include <queue>
#include "Milling.h"

using namespace app;

Milling::Milling() {
	m_heightMap = std::vector<std::vector<float>>(TextureSizeX(), std::vector<float>(TextureSizeY(), SizeZ()));
}

void Milling::ResetHeightMap(const Device& device) {
	resetHeightMap = false;
	m_heightMap = std::vector<std::vector<float>>(TextureSizeX(), std::vector<float>(TextureSizeY(), SizeZ()));

	auto texDesc = Texture2DDescription::DynamicTextureDescription(TextureSizeY(), TextureSizeX());
	m_heightMapTex = device.CreateTexture(texDesc);

	UpdateHeightMap(device);

	m_heightMapTexSRV = device.CreateShaderResourceView(m_heightMapTex);

	ID3D11ShaderResourceView* gsr[] = { m_heightMapTexSRV.get() };
	device.deviceContext()->GSSetShaderResources(0, 1, gsr);

	if (baseMeshSizeChanged) {
		UpdateMesh(device);
		baseMeshSizeChanged = false;
	}
}

void Milling::UpdateHeightMap(const Device& device) {
	updateHeightMap = false;
	const unsigned int sizeX = TextureSizeX();
	const unsigned int sizeY = TextureSizeY();

	uint8_t whole = static_cast<uint8_t>(SizeZ()); // assume max height is 255
	uint8_t frac = static_cast<uint8_t>((SizeZ() - whole) * 100); // up to 2 decimals

	D3D11_MAPPED_SUBRESOURCE mapped;
	device.deviceContext()->Map(m_heightMapTex.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	auto* data = static_cast<uint8_t*>(mapped.pData);
	for (unsigned int x = 0; x < sizeX; ++x) {
		for (unsigned int y = 0; y < sizeY; ++y) {
			float& height = m_heightMap[x][y];
			size_t idx = x * mapped.RowPitch + y * 4;
			data[idx + 0] = static_cast<uint8_t>(std::clamp(height / SizeZ(), 0.f, 1.f) * 255); // relative height
			data[idx + 1] = whole; // whole part of max height
			data[idx + 2] = frac; // 2 decimals of max height
			data[idx + 3] = 255; // unused
		}
	}
	device.deviceContext()->Unmap(m_heightMapTex.get(), 0);
}

std::optional<std::string> Milling::Mill(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos) {
	std::string positionString = "(" + std::to_string(nextPos.z()) + "," + std::to_string(nextPos.x()) + "," + std::to_string(nextPos.y()) + ")";
	if (!IsWithinMargins(nextPos)) {
		return "ERROR: next position " + positionString + " is outside milling margins";
	}
	if (!IsAboveBase(nextPos)) {
		return "ERROR: next position " + positionString + " is below milling base";
	}
	std::vector<std::vector<bool>> canvas(TextureSizeX(), std::vector<bool>(TextureSizeY(), false));

	auto [currX, currY] = Scene2Canvas(currPos);
	auto [nextX, nextY] = Scene2Canvas(nextPos);
	const float R = cutter.GetRadius();

	// calculating normals to the line on the ZX plane
	float vz = nextPos.z() - currPos.z();
	float vx = nextPos.x() - currPos.x();
	float nz1 = -vx;
	float nx1 = vz;
	float length = std::sqrt(nz1 * nz1 + nx1 * nx1);
	if (length < FZERO) {
		// there is no movement
		return std::nullopt;
	}
	nz1 = (nz1 * R) / length;
	nx1 = (nx1 * R) / length;
	float nz2 = -nz1;
	float nx2 = -nx1;

	// calculating parallel lines on the ZX plane
	gmod::vector3<float> currPos1 = { currPos.x() + nx1, 0, currPos.z() + nz1 };
	gmod::vector3<float> nextPos1 = { nextPos.x() + nx1, 0, nextPos.z() + nz1 };

	gmod::vector3<float> currPos2 = { currPos.x() + nx2, 0, currPos.z() + nz2 };
	gmod::vector3<float> nextPos2 = { nextPos.x() + nx2, 0, nextPos.z() + nz2 };

	auto [currX1, currY1] = Scene2Canvas(currPos1);
	auto [nextX1, nextY1] = Scene2Canvas(nextPos1);

	auto [currX2, currY2] = Scene2Canvas(currPos2);
	auto [nextX2, nextY2] = Scene2Canvas(nextPos2);

	// mill
	float maxHeight = 0.f;
	float canvasR = R * ((TextureSizeX() - 1) / size[0]);
	float mH;

	// circle at the start
	if (AreWithinCanvas(currX, currY)) {
		mH = DrawCircle(canvas, currX, currY, R);
		maxHeight = mH > maxHeight ? mH : maxHeight;

		mH = FloodFill(canvas, currX, currY);
		maxHeight = mH > maxHeight ? mH : maxHeight;
	}
	// circle at the end
	if (AreWithinCanvas(nextX, nextY)) {
		mH = DrawCircle(canvas, nextX, nextY, R);
		maxHeight = mH > maxHeight ? mH : maxHeight;

		mH = FloodFill(canvas, nextX, nextY);
		maxHeight = mH > maxHeight ? mH : maxHeight;
	}

	// first line
	mH = DrawLine(canvas, currX1, currY1, nextX1, nextY1);
	maxHeight = mH > maxHeight ? mH : maxHeight;

	// second line
	mH = DrawLine(canvas, currX2, currY2, nextX2, nextY2);
	maxHeight = mH > maxHeight ? mH : maxHeight;

	// fill between
	gmod::vector3<float> midPoint = (currPos + nextPos) * 0.5f;
	auto [midX, midY] = Scene2Canvas(midPoint);
	if (!AreWithinCanvas(midX, midY)) {
		if (AreWithinCanvas(currX, currY)) {
			midX = currX;
			midY = currY;
		} else if (AreWithinCanvas(nextX, nextY)) {
			midX = nextX;
			midY = nextY;
		} else {
			return "ERROR: move to " + positionString + " is outside of milling zone";
		}
	}
	mH = FloodFill(canvas, midX, midY);
	maxHeight = mH > maxHeight ? mH : maxHeight;

	float tipStartY = currPos.y();
	float tipEndY = nextPos.y();
	if (!cutter.isBaseOriented()) {
		tipStartY -= cutter.GetTipCentreDiff();
		tipEndY -= cutter.GetTipCentreDiff();
	}

	if (maxHeight >= tipStartY || maxHeight >= tipEndY) {
		if (!IsWithinAngle(currPos, nextPos)) {
			return "ERROR: the movement to " + positionString + " is too vertical";
		}
		if (!IsWithinMillingPart(currPos, nextPos, canvas)) {
			return "ERROR: the movement to " + positionString + " uses non-milling part";
		}
		UpdateHeights(currPos, nextPos, canvas);
		updateHeightMap = true;
	}
	return std::nullopt;
}

void Milling::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Milling).Set(context);
	m_planeMesh.Render(context);
}

void Milling::UpdateMesh(const Device& device) {
	sceneChanged = false;
	const unsigned int size = baseMeshSize + 1;

	std::vector<Vertex_PoNo> verts(size * size);
	std::vector<USHORT> idxs(4 * baseMeshSize * baseMeshSize);

	float stepX = SizeX() / baseMeshSize;
	float stepY = SizeY() / baseMeshSize;

	for (unsigned int x = 0; x < size; ++x) {
		for (unsigned int y = 0; y < size; ++y) {
			unsigned int index = x * size + y;
			float posX = -SizeX() / 2 + x * stepX + centre[0];
			float posY = -SizeY() / 2 + y * stepY + centre[1];
			float posZ = SizeZ() + centre[2];
			verts[index] = Vertex_PoNo{ DirectX::XMFLOAT3(posY, posZ, posX), DirectX::XMFLOAT3(0, 0, 0) };
		}
	}

	for (unsigned int x = 0; x < size; ++x) {
		for (unsigned int y = 0; y < size; ++y) {
			unsigned int index = x * size + y;
			verts[index].normal = CalculateNormal(x, y, size, stepX, stepY, verts);
		}
	}

	unsigned int idx = 0;
	for (unsigned int patchX = 0; patchX < baseMeshSize; ++patchX) {
		for (unsigned int patchY = 0; patchY < baseMeshSize; ++patchY) {
			// 4 corners of the current patch
			USHORT tl = patchX * size + patchY;
			USHORT tr = patchX * size + patchY + 1;
			USHORT bl = (patchX + 1) * size + patchY;
			USHORT br = (patchX + 1) * size + patchY + 1;

			idxs[idx++] = tl;
			idxs[idx++] = tr;
			idxs[idx++] = bl;
			idxs[idx++] = br;
		}
	}
	m_planeMesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);
}

DirectX::XMFLOAT3 Milling::CalculateNormal(unsigned int x, unsigned int y, unsigned int size, float stepX, float stepY, const std::vector<Vertex_PoNo>& verts) {
	float z_center = verts[x * size + y].position.y;
	float dz_dx, dz_dy;

	if (x == 0) {
		// top edge
		float z_below = verts[(x + 1) * size + y].position.y;
		dz_dx = (z_below - z_center) / stepX;
	} else if (x == size - 1) {
		// bottom edge
		float z_above = verts[(x - 1) * size + y].position.y;
		dz_dx = (z_center - z_above) / stepX;
	} else {
		// interior
		float z_above = verts[(x - 1) * size + y].position.y;
		float z_below = verts[(x + 1) * size + y].position.y;
		dz_dx = (z_below - z_above) / (2 * stepX);
	}

	if (y == 0) {
		// left edge
		float z_toRight = verts[x * size + (y + 1)].position.y;
		dz_dy = (z_toRight - z_center) / stepY;
	} else if (y == size - 1) {
		// right edge  
		float z_toLeft = verts[x * size + (y - 1)].position.y;
		dz_dy = (z_center - z_toLeft) / stepY;
	} else {
		// interior
		float z_toLeft = verts[x * size + (y - 1)].position.y;
		float z_toRight = verts[x * size + (y + 1)].position.y;
		dz_dy = (z_toRight - z_toLeft) / (2 * stepY);
	}

	DirectX::XMFLOAT3 normal(-dz_dy, 1.0f, -dz_dx);

	float length = sqrt(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
	if (length > 0.0001f) {
		normal.x /= length;
		normal.y /= length;
		normal.z /= length;
	}

	return normal;
}

bool Milling::IsWithinMargins(const gmod::vector3<float>& nextPos) const {
	const float R = cutter.GetRadius();
	const float halfSizeX = size[0] / 2;
	const float halfSizeY = size[1] / 2;

	const float leftX = centre[1] - halfSizeY + R - margin; 
	const float rightX = centre[1] + halfSizeY - R + margin;
	const float topZ = centre[0] - halfSizeX + R - margin;
	const float bottomZ = centre[0] + halfSizeX - R + margin;

	return nextPos.x() >= leftX && nextPos.x() <= rightX && nextPos.z() >= topZ && nextPos.z() <= bottomZ;
}

bool Milling::IsAboveBase(const gmod::vector3<float>& nextPos) const {
	const float tip = cutter.isBaseOriented() ? nextPos.y() : nextPos.y() - cutter.GetTipCentreDiff();
	return tip >= baseThickness + centre[2];
}

std::pair<int, int> Milling::Scene2Canvas(const gmod::vector3<float>& coord) const {
	const float xScaled = std::floor((coord.z() + size[0] / 2 - centre[0]) * ((TextureSizeX() - 1) / size[0]));
	const float yScaled = std::floor((coord.x() + size[1] / 2 - centre[1]) * ((TextureSizeY() - 1) / size[1]));
	//const int maxX = TextureSizeX() - 1;
	//const int maxY = TextureSizeY() - 1;
	//unsigned int x = std::max(0, std::min(maxX, static_cast<int>(std::floor(xScaled))));
	//unsigned int y = std::max(0, std::min(maxY, static_cast<int>(std::floor(yScaled))));

	return std::make_pair(xScaled, yScaled);
}

gmod::vector3<float> Milling::Canvas2Scene(unsigned int x, unsigned int y) const {
	float coordZ = static_cast<float>(x) * (size[0] / (TextureSizeX() - 1)) - size[0] / 2 + centre[0];
	float coordY = m_heightMap[x][y] + centre[2];
	float coordX = static_cast<float>(y) * (size[1] / (TextureSizeY() - 1)) - size[1] / 2 + centre[1];

	return gmod::vector3<float>(coordX, coordY, coordZ);
}

bool Milling::AreWithinCanvas(int x, int y) const {
	return 0 <= x && x < TextureSizeX() && 0 <= y && y < TextureSizeY();
}

void Milling::DrawInAllOctants(std::vector<std::vector<bool>>& canvas, int cx, int cy, int x, int y, float& maxHeight) const {
	int points[8][2] = {
		{cx + y, cy + x},  // octant 1: (y, x)
		{cx + x, cy + y},  // octant 2: (x, y) 
		{cx - x, cy + y},  // octant 3: (-x, y)
		{cx - y, cy + x},  // octant 4: (-y, x)
		{cx - y, cy - x},  // octant 5: (-y, -x)
		{cx - x, cy - y},  // octant 6: (-x, -y)
		{cx + x, cy - y},  // octant 7: (x, -y)
		{cx + y, cy - x}   // octant 8: (y, -x)
	};

	for (auto& point : points) {
		int px = point[0];
		int py = point[1];

		if (AreWithinCanvas(px, py)) {
			canvas[px][py] = true;
			if (m_heightMap[px][py] > maxHeight) {
				maxHeight = m_heightMap[px][py];
			}
		}
	}
}

// starting point is within canvas
float Milling::DrawCircle(std::vector<std::vector<bool>>& canvas, int xCentre, int yCentre, int R) const {
	float maxHeight = 0.f;

	int de = 3;
	int dse = 5 - 2 * R;
	int d = 1 - R;
	int x = 0;
	int y = R;
	
	DrawInAllOctants(canvas, xCentre, yCentre, x, y, maxHeight);

	while (y > x) {
		if (d < 0) {
			d += de;
			de += 2;
			dse += 2;
		} else {
			d += dse;
			de += 2;
			dse += 4;
			y -= 1;
		}
		x += 1;
		DrawInAllOctants(canvas, xCentre, yCentre, x, y, maxHeight);
	}
	
	return maxHeight;
}

// Bresenham's midpoint algorithm
// starting or ending point can be outside canvas
float Milling::DrawLine(std::vector<std::vector<bool>>& canvas, int xStart, int yStart, int xEnd, int yEnd) const {
	float maxHeight = 0.f;
	
	if (AreWithinCanvas(xStart, yStart)) {
		maxHeight = m_heightMap[xStart][yStart];
		canvas[xStart][yStart] = true;
	}

	const int x1 = static_cast<int>(xStart);
	const int y1 = static_cast<int>(yStart);
	const int x2 = static_cast<int>(xEnd);
	const int y2 = static_cast<int>(yEnd);

	int x = x1, y = y1;

	int dx, xi;
	if (x1 < x2) {
		xi = 1;
		dx = x2 - x1;
	} else {
		xi = -1;
		dx = x1 - x2;
	}

	int dy, yi;
	if (y1 < y2) {
		yi = 1;
		dy = y2 - y1;
	} else {
		yi = -1;
		dy = y1 - y2;
	}

	if (dx > dy) {
		int dne = (dy - dx) << 1;
		int de = dy << 1;
		int d = de - dx;
		while (x != x2) {
			x += xi;
			if (d >= 0) {
				y += yi;
				d += dne;
			} else {
				d += de;
			}
			if (AreWithinCanvas(x, y)) {
				canvas[x][y] = true;
				if (m_heightMap[x][y] > maxHeight) {
					maxHeight = m_heightMap[x][y];
				}
			}
		}
	} else {
		int dne = (dx - dy) << 1;
		int dn = dx << 1;
		int d = dn - dy;
		while (y != y2) {
			y += yi;
			if (d >= 0) {
				x += xi;
				d += dne;
			} else {
				d += dn;
			}
			if (AreWithinCanvas(x, y)) {
				canvas[x][y] = true;
				if (m_heightMap[x][y] > maxHeight) {
					maxHeight = m_heightMap[x][y];
				}
			}
		}
	}

	return maxHeight;
}

// starting point is within canvas
float Milling::FloodFill(std::vector<std::vector<bool>>& canvas, int xStart, int yStart) const {
	float maxHeight = m_heightMap[xStart][yStart];
	canvas[xStart][yStart] = true;

	const float sizeX = TextureSizeX();
	const float sizeY = TextureSizeY();

	std::queue<std::pair<int, int>> q;
	std::vector<bool> visited(sizeX * sizeY, false);

	q.push({ xStart, yStart });
	visited[xStart * sizeY + yStart] = true;

	const std::array<std::pair<int, int>, 4> neighbors = { {
		{ 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 }
	} };

	while (!q.empty()) {
		auto [x, y] = q.front();
		q.pop();

		for (auto [dx, dy] : neighbors) {
			int nx = x + dx, ny = y + dy;
			if (nx < 0 || ny < 0 || nx >= sizeX || ny >= sizeY) { continue; }

			size_t index = nx * sizeY + ny;
			if (visited[index]) { continue; }

			if (!canvas[nx][ny]) {
				visited[index] = true;
				canvas[nx][ny] = true;
				if (m_heightMap[nx][ny] > maxHeight) {
					maxHeight = m_heightMap[nx][ny];
				}
				q.push({ nx, ny });
			}
		}
	}

	return maxHeight;
}

bool Milling::IsWithinAngle(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos) const {
	if (std::abs(currPos.y() - nextPos.y()) < FZERO) {
		return true;
	}
	const gmod::vector3<float> moveDir = (nextPos - currPos).normalized();
	gmod::vector3<float> vertical;
	if (nextPos.y() > currPos.y()) {
		// cutter moves up
		vertical = { 0, 1, 0 };
		if (!cutter.shouldApplyToBothDirections()) {
			return true;
		}
	} else {
		// cutter moves down
		vertical = { 0, -1, 0 };
	}
	// cos(90deg - alpha) = sin(alpha)
	const float sinAlpha = gmod::dot<float>(moveDir, vertical);
	// alpha <= maxAlpha <=> sin(alpha) <= sin(maxAlpha)

	return sinAlpha <= std::sin(cutter.GetMaxAngle() * DirectX::XM_PI / 180.f);
}

bool Milling::IsWithinMillingPart(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, const std::vector<std::vector<bool>>& canvas) const {
	const gmod::vector3<float> dirMove = nextPos - currPos;
	const float dirMoveNorm2 = gmod::dot<float>(dirMove, dirMove);

	auto toolPos = [this, &currPos, &nextPos, &dirMove, &dirMoveNorm2](const gmod::vector3<float>& pos) -> gmod::vector3<float> {
		auto diff = pos - currPos;
		float t = gmod::dot<float>(diff, dirMove) / dirMoveNorm2;
		t = std::max(0.f, std::min(1.f, t)); // clamp
		if (t <= FZERO) {
			return currPos;
		}
		if (std::abs(t - 1) <= FZERO) {
			return nextPos;
		}
		return currPos + t * dirMove;
	};

	for (unsigned int x = 0; x < TextureSizeX(); ++x) {
		for (unsigned int y = 0; y < TextureSizeY(); ++y) {
			if (!canvas[x][y]) { continue; }
			auto pos = Canvas2Scene(x, y);
			pos.y() += centre[2];

			auto tipPos = toolPos(pos);
			if (!cutter.isBaseOriented()) {
				tipPos.y() -= cutter.GetTipCentreDiff();
			}

			float currMillHeightMax = tipPos.y() + cutter.GetCuttingHeight();
			if (pos.y() > currMillHeightMax) {
				return false;
			}
		}
	}
	return true;
}

void Milling::UpdateHeights(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, const std::vector<std::vector<bool>>& canvas) {
	const gmod::vector3<float> dirMove = nextPos - currPos;
	const float dirMoveNorm2 = gmod::dot<float>(dirMove, dirMove);

	auto toolPos = [this, &currPos, &nextPos, &dirMove, &dirMoveNorm2](const gmod::vector3<float>& pos) -> gmod::vector3<float> {
		auto diff = pos - currPos;
		float t = gmod::dot<float>(diff, dirMove) / dirMoveNorm2;
		t = std::max(0.f, std::min(1.f, t)); // clamp
		if (t <= FZERO) {
			return currPos;
		}
		if (std::abs(t - 1) <= FZERO) {
			return nextPos;
		}
		return currPos + t * dirMove;
	};

	for (unsigned int x = 0; x < TextureSizeX(); ++x) {
		for (unsigned int y = 0; y < TextureSizeY(); ++y) {
			if (!canvas[x][y]) { continue; }
			auto pos = Canvas2Scene(x, y);
			pos.y() += centre[2];

			auto tipPos = toolPos(pos);
			if (!cutter.isBaseOriented()) {
				tipPos.y() -= cutter.GetTipCentreDiff();
			}

			if (cutter.GetCutterType() == CutterType::Cylindrical) {
				if (m_heightMap[x][y] > tipPos.y()) {
					m_heightMap[x][y] = tipPos.y();
				}
			} else {
				const float R = cutter.GetRadius();
				auto sphereCentre = gmod::vector3<float>(tipPos.x(), tipPos.y() + R, tipPos.z());
				const float xDiff = (pos.x() - sphereCentre.x());
				const float zDiff = (pos.z() - sphereCentre.z());
				const float underTheRoot = R * R - xDiff * xDiff - zDiff * zDiff;
				if (underTheRoot < 0) {
					continue; // the point turned out to be outside of milling zone
				}
				float height = sphereCentre.y() - std::sqrt(underTheRoot);
				if (m_heightMap[x][y] > height) {
					m_heightMap[x][y] = height;
				}
			}
		}
	}
}

void Milling::RenderProperties() {
	float inputWidth = 100.f;
	ImGui::Columns(2, "milling_settings", false);
	ImGui::SetColumnWidth(0, 150.f);

	ImGui::Text("Base thickness [cm]:"); ImGui::NextColumn();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputFloat("##base_thickness", &baseThickness, 0.01f, 1.f); ImGui::NextColumn();

	ImGui::Text("Margin [cm]:"); ImGui::NextColumn();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputFloat("##margin", &margin, 0.01f, 1.f); ImGui::NextColumn();

	ImGui::Text("Base mesh size:"); ImGui::NextColumn();
	int bms = baseMeshSize;
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputInt("##bms", &bms, 1, 10); ImGui::NextColumn();
	if (bms != baseMeshSize && bms > 0) {
		baseMeshSize = bms;
		resetHeightMap = true;
		baseMeshSizeChanged = true;
	}
	ImGui::Columns(1);

	ImGui::Text("Resolution:");
	ImGui::Text("X:"); ImGui::SameLine();
	int resX = resolutionX;
	ImGui::SetNextItemWidth(90.f);
	ImGui::InputInt("##resolution_X", &resX, 1, 10);
	if (resX != resolutionX && resX > 0 && resX <= 64) {
		resolutionX = resX;
		resetHeightMap = true;
	}
	ImGui::SameLine();
	ImGui::Text("Y:"); ImGui::SameLine();
	int resY = resolutionY;
	ImGui::SetNextItemWidth(90.f);
	ImGui::InputInt("##resolution_Y", &resY, 1, 10);
	if (resY != resolutionY && resY > 0 && resY <= 64) {
		resolutionY = resY;
		resetHeightMap = true;
	}

	ImGui::Spacing();
	if (ImGui::Button("Reset scene", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
		resetHeightMap = true;
	}
}


