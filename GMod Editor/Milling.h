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

		bool resolutionChanged = true;
		unsigned int resolutionX = 500;
		unsigned int resolutionY = 500;

		// all in millimetres, milling space
		std::array<float, 3> centre = { 0.f, 0.f, 0.f };
		inline float CentreX() const { return centre[0]; }
		inline float CentreY() const { return centre[1]; }
		inline float CentreZ() const { return centre[2]; }
		std::array<float, 3> size = { 150.f, 150.f, 50.f };
		inline float SizeX() const { return size[0]; }
		inline float SizeY() const { return size[1]; }
		inline float SizeZ() const { return size[2]; }
		float baseThickness = 15.f;
		float margin = 50.f;
		bool sceneChanged = false;

		Milling();
		void Initialize(const Device& device);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;
		void UpdateMesh(const Device& device);
		void RenderProperties();

		void ResetHeightMap(Device& device);
		void UpdateHeightMap(Device& device);

		std::optional<std::string> Mill(Device& device, const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, bool updateTexture = true);
		void ResetErrorBuffer(const Device& device);
		UINT ReadComputeErrors(const Device& device);
	private:
		// value to consider as 0 for floats
		const float FZERO = 100.f * std::numeric_limits<float>::epsilon();

		Mesh m_planeMesh;
		mini::dx_ptr<ID3D11Texture2D> m_heightMapTex_dynamic;		
		mini::dx_ptr<ID3D11ShaderResourceView> m_heightMapTexSRV;
		mini::dx_ptr<ID3D11Texture2D> m_heightMapTex_rw;
		mini::dx_ptr<ID3D11UnorderedAccessView> m_heightMapTexUAV;

		bool IsWithinMargins(const gmod::vector3<float>& nextPos) const;
		bool IsAboveBase(const gmod::vector3<float>& nextPos) const;

		struct MillingParams {
			// Group 1: 16 bytes
			DirectX::XMFLOAT3 currPos;
			unsigned int cutterType; // 0 = cylindrical, 1 = spherical

			// Group 2: 16 bytes  
			DirectX::XMFLOAT3 nextPos;
			unsigned int checkBothDirections;  // 0 = false, 1 = true

			// Group 3: 16 bytes
			float tipCentreOffset;
			float cutterRadius;
			float cuttingHeight;
			float maxAngle; // in radians
	
			// Group 4: 16 bytes
			DirectX::XMFLOAT3 centre;
			unsigned int textureSizeX;

			// Group 5: 16 bytes
			DirectX::XMFLOAT3 size;
			unsigned int textureSizeY;
		};

		static constexpr uint32_t NUM_THREADS = 16;

		mini::dx_ptr<ID3D11Buffer> m_errorBuffer_raw;
		mini::dx_ptr<ID3D11Buffer> m_errorBuffer_staging;
		mini::dx_ptr<ID3D11UnorderedAccessView> m_errorUAV;

		mini::dx_ptr<ID3D11Buffer> m_constBuffMillingParams;
		mini::dx_ptr<ID3D11ComputeShader> m_millingComputeShader;

		//// for any position
		//std::pair<int, int> Scene2Canvas(const gmod::vector3<float>& coord) const;
		//// only for positions within canvas
		//gmod::vector3<float> Canvas2Scene(unsigned int x, unsigned int y) const;

		//bool AreWithinCanvas(int x, int y) const;

		//float DrawCircle(std::vector<std::vector<bool>>& canvas, int xCentre, int yCentre, int R) const;
		//float DrawLine(std::vector<std::vector<bool>>& canvas, int xStart, int yStart, int xEnd, int yEnd) const;
		//float FloodFill(std::vector<std::vector<bool>>& canvas, int xStart, int yStart) const;

		//void DrawInAllOctants(std::vector<std::vector<bool>>& canvas, int cx, int cy, int x, int y, float& maxHeight) const;

		//bool IsWithinAngle(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos) const;
		//bool IsWithinMillingPart(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, const std::vector<std::vector<bool>>& canvas) const;
		
		//void UpdateHeights(const gmod::vector3<float>& currPos, const gmod::vector3<float>& nextPos, const std::vector<std::vector<bool>>& canvas);
	};
}
