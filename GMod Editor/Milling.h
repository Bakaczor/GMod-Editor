#pragma once
#include "../gmod/vector3.h"
#include <array>
#include <optional>
#include <string>
#include "Cutter.h"
#include "Mesh.h"

namespace app {
	class Milling {
	public:
		const std::array<float, 4> color = { 0.8f, 0.8f, 1.f, 1.f }; // replace with texture
		inline gmod::matrix4<float> modelMatrix() { return gmod::matrix4<float>::identity(); }

		Cutter cutter;

		bool baseMeshSizeChanged = true;
		unsigned int baseMeshSize = 8;
		unsigned int resolutionX = 24;
		inline unsigned int TextureSizeX() const { return baseMeshSize * resolutionX + 1; }
		unsigned int resolutionY = 24;
		inline unsigned int TextureSizeY() const { return baseMeshSize * resolutionY + 1; }

		// all in millimetres
		std::array<float, 3> centre = { 0.f, 0.f, 0.f };
		std::array<float, 3> size = { 150.f, 150.f, 50.f };
		inline float SizeX() { return size[0]; }
		inline float SizeY() { return size[1]; }
		inline float SizeZ() { return size[2]; }
		float baseThickness = 1.5f;
		float margin = 0.5f;
		bool sceneChanged = false;



		Milling();
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;
		void UpdateMesh(const Device& device);
		void RenderProperties();

		bool resetHeightMap = true;
		void ResetHeightMap(const Device& device);
		bool updateHeightMap = false;
		void UpdateHeightMap(const Device& device);

		std::optional<std::string> Mill(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos);
	private:
		Mesh m_planeMesh;
		std::vector<std::vector<float>> m_heightMap;
		mini::dx_ptr<ID3D11Texture2D> m_heightMapTex;		
		mini::dx_ptr<ID3D11ShaderResourceView> m_heightMapTexSRV;

		DirectX::XMFLOAT3 CalculateNormal(unsigned int x, unsigned int y, unsigned int size, float stepX, float stepY, const std::vector<Vertex_PoNo>& verts);
	};
}
