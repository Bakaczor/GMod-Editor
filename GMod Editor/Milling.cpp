#include "framework.h"
#include <imgui.h>
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
			size_t idx = (x * sizeY + y) * 4;
			data[idx + 0] = static_cast<uint8_t>(std::clamp(height / SizeZ(), 0.f, 1.f) * 255); // relative height
			data[idx + 1] = whole; // whole part of max height
			data[idx + 2] = frac; // 2 decimals of max height
			data[idx + 3] = 255; // unused
		}
	}
	device.deviceContext()->Unmap(m_heightMapTex.get(), 0);
}

std::optional<std::string> Milling::Mill(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos) {
	// TODO
	// use bresenham to get list of pixels to check, max height, isMilling
	// check margins
	// check base
	// if milling
	// millingHeight >= maxHeight
	// check angle
	// for spherical, distance from tip to elements should >= 0 (consider vector as y = mx + b and calculate distances)
	// if everything is okay, mill
	updateHeightMap = true;
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


