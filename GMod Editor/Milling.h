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
		inline gmod::matrix4<float> modelMatrix() const { return gmod::matrix4<float>::identity(); }

		Cutter cutter;

		bool baseMeshSizeChanged = true;
		unsigned int baseMeshSize = 7;
		unsigned int resolutionX = 10;
		inline unsigned int TextureSizeX() const { return baseMeshSize * resolutionX + 1; }
		unsigned int resolutionY = 10;
		inline unsigned int TextureSizeY() const { return baseMeshSize * resolutionY + 1; }

		// all in millimetres, milling space
		std::array<float, 3> centre = { 0.f, 0.f, 0.f };
		std::array<float, 3> size = { 200.f, 200.f, 50.f };
		inline float SizeX() const { return size[0]; }
		inline float SizeY() const { return size[1]; }
		inline float SizeZ() const { return size[2]; }
		float baseThickness = 15.f;
		float margin = 75.f;
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
		// value to consider as 0 for floats
		const float FZERO = 1000.f * std::numeric_limits<float>::epsilon();

		Mesh m_planeMesh;
		std::vector<std::vector<float>> m_heightMap;
		mini::dx_ptr<ID3D11Texture2D> m_heightMapTex;		
		mini::dx_ptr<ID3D11ShaderResourceView> m_heightMapTexSRV;

		DirectX::XMFLOAT3 CalculateNormal(unsigned int x, unsigned int y, unsigned int size, float stepX, float stepY, const std::vector<Vertex_PoNo>& verts);

		bool IsWithinMargins(const gmod::vector3<float>& nextPos) const;
		bool IsAboveBase(const gmod::vector3<float>& nextPos) const;

		// for any position
		std::pair<int, int> Scene2Canvas(const gmod::vector3<float>& coord) const;
		// only for positions within canvas
		gmod::vector3<float> Canvas2Scene(unsigned int x, unsigned int y) const;

		bool AreWithinCanvas(int x, int y) const;

		float DrawCircle(std::vector<std::vector<bool>>& canvas, int xCentre, int yCentre, int R) const;
		float DrawLine(std::vector<std::vector<bool>>& canvas, int xStart, int yStart, int xEnd, int yEnd) const;
		float FloodFill(std::vector<std::vector<bool>>& canvas, int xStart, int yStart) const;

		void DrawInAllOctants(std::vector<std::vector<bool>>& canvas, int cx, int cy, int x, int y, float& maxHeight) const;

		bool IsWithinAngle(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos) const;
		bool IsWithinMillingPart(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, const std::vector<std::vector<bool>>& canvas) const;
		
		void UpdateHeights(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, const std::vector<std::vector<bool>>& canvas);
	};
}
