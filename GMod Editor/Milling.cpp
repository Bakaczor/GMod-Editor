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
	m_heighMapTex = device.CreateTexture(texDesc);

	UpdateHeightMap(device);

	heightMapTexSRV = device.CreateShaderResourceView(m_heighMapTex);

	if (baseMeshSizeChanged) {
		UpdateMesh(device);
		baseMeshSizeChanged = false;
	}

	// bind in Application, with other textures, if any
}

void Milling::UpdateHeightMap(const Device& device) {
	updateHeightMap = false;
	const unsigned int sizeX = TextureSizeX();
	const unsigned int sizeY = TextureSizeY();

	std::vector<uint8_t> image(sizeX * sizeY * 4, 0);
	for (unsigned int x = 0; x < sizeX; ++x) {
		for (unsigned int y = 0; y < sizeY; ++y) {
			float& height = m_heightMap[x][y];
			size_t idx = 4 * (x * sizeY + y);

			uint8_t whole = static_cast<uint8_t>(SizeZ()); // assume max height is 255
			uint8_t frac = static_cast<uint8_t>((SizeZ() - whole) * 100); // up to 2 decimals
			image[idx + 0] = static_cast<uint8_t>(height / SizeZ()) * 255; // relative height
			image[idx + 1] = whole; // whole part of max height
			image[idx + 2] = frac; // 2 decimals of max height
			image[idx + 3] = 255; // unused
		}
	}
	device.deviceContext()->UpdateSubresource(m_heighMapTex.get(), 0, nullptr, image.data(), sizeY * 4, 0);
}

std::optional<std::string> Milling::Mill(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos) {
	// TODO
	//updateHeightMap = true;
	return std::nullopt;
}

void Milling::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	// later change to phong shaders
	map.at(ShaderType::Regular).Set(context);
	m_planeMesh.Render(context);
}

void Milling::UpdateMesh(const Device& device) {
	sceneChanged = false;
	const unsigned int size = baseMeshSize + 1;

	std::vector<Vertex_Po> verts(size * size);
	std::vector<USHORT> idxs(4 * baseMeshSize * baseMeshSize);

	float stepX = SizeX() / baseMeshSize;
	float stepY = SizeY() / baseMeshSize;

	for (unsigned int x = 0; x < size; ++x) {
		for (unsigned int y = 0; y < size; ++y) {
			unsigned int index = x * size + y;
			float posX = -SizeX() / 2 + x * stepX + centre[0];
			float posY = -SizeY() / 2 + y * stepY + centre[1];
			float posZ = SizeZ() + centre[2];
			verts[index] = Vertex_Po{ { posY, posZ, posX } };
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

	// D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST
	m_planeMesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
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


