#pragma once
#include <array>
#include <atomic>
#include "Milling.h"
#include "PathParser.h"
#include "Mesh.h"
#include "Shaders.h"

namespace app {
	class PathAnimator {
	public:
		// millimetres per frame
		float speed = 1.f;
		bool canRender = false;

		std::atomic<bool> isRunning = false;
		std::atomic<bool> completeAnimation = false;

		bool errorDetected = false;
		std::string errorMsg;

		bool displayPath = true;
		std::array<float, 4> pathColor = { 0.f, 1.f, 0.f, 1.f };

		PathAnimator(PathParser& path, Milling& milling);
		void RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const;
		void ClearMesh();

		inline gmod::matrix4<float> modelMatrix() { return gmod::matrix4<float>::identity(); }

		void StartAnimation();
		void StopAnimation();
		void RestartAnimation();
		void CompleteAnimation(const Device& device);
		bool MakeStep(const Device& device, float deltaTime);

		void CompleteAnimationAsync(const Device& device);
	private:
		std::vector<DirectX::XMFLOAT3> m_points;
		Mesh m_polylineMesh;
		PathParser& m_path;
		Milling& m_milling;
		bool m_firstStep = true;
		gmod::vector3<float> m_currPos;

		void UpdateMesh(const Device& device);
	};
}
