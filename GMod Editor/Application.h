#pragma once
#include "framework.h"
#include <Windows.h>
#include "../imgui/backends/imgui_impl_dx11.h"
#include "../imgui/backends/imgui_impl_win32.h"
#include "../gmod/matrix4.h"
#include "../gmod/quaternion.h"
#include "../gmod/vector3.h"
#include "../mini/windowApplication.h"
#include "../mini/exceptions.h"
#include "Camera.h"
#include "Cube.h"
#include "Device.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Torus.h"
#include "Axes.h"
#include "CubeModel.h"
#include "AxesModel.h"
#include "PointModel.h"
#include "Shaders.h"
#include <unordered_map>

namespace app {
	class UI;

	class Application : public mini::WindowApplication {
		friend class UI;
	public:
		explicit Application(HINSTANCE hInstance);
		~Application();
		int Run(int cmdShow = SW_SHOWNORMAL) override;

	protected:
		int MainLoop() override;
		bool ProcessMessage(mini::WindowMessage& msg) override;

	private:
		std::unique_ptr<UI> m_UI;

		Mouse m_mouse;
		Keyboard m_keyboard;
		Camera m_camera;
		Axes m_axes;

		void Initialize();
		void SetShaders(ShaderType shaderType);

		void RenderUI();
		void Update();
		void Render();

		void HandleTransformsOnMouseMove(LPARAM lParam);
		void HandleCameraOnMouseMove(LPARAM lParam);
		void HandleCameraOnMouseWheel(WPARAM wParam);
		Object* HandleSelectionOnMouseClick(LPARAM lParam);
		void ResizeWnd();
#pragma region APPLICATION
		const static float selectionRadius;
		const static float traSensitivity;
		const static float rotSensitivity;
		const static float scaSensitivity;
		const static std::wstring m_appName;
		static int m_winWidth;
		static int m_winHeight;
#pragma endregion

#pragma region STATE
		bool m_firstPass = true;
		bool m_wndSizeChanged = false;
#pragma endregion

#pragma region PROJECTION
		const static float m_near;
		const static float m_far;
		const static float m_FOV;
		float aspect() const;
		gmod::matrix4<float> projMatrix() const;
		gmod::matrix4<float> projMatrix_inv() const;
#pragma endregion

#pragma region STATIC_MODELS
		static std::unique_ptr<AxesModel> m_axesModel;
		static std::unique_ptr<CubeModel> m_cubeModel;
		static std::unique_ptr<PointModel> m_pointModel;
#pragma endregion

#pragma region DEVICE
		Device m_device;
		mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
		mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
		mini::dx_ptr<ID3D11RasterizerState> m_rastState;
		std::unordered_map<ShaderType, Shaders> m_shaders;

		mini::dx_ptr<ID3D11Buffer> m_constBuffModel;
		mini::dx_ptr<ID3D11Buffer> m_constBuffView;
		mini::dx_ptr<ID3D11Buffer> m_constBuffProj;
		mini::dx_ptr<ID3D11Buffer> m_constBuffColor;
#pragma endregion

		template<typename T>
		static DirectX::XMFLOAT4X4 matrix4_to_XMFLOAT4X4(const gmod::matrix4<T>& M) {
			DirectX::XMFLOAT4X4 result;
			for (int i = 0; i < 4; ++i) {
				for (int j = 0; j < 4; ++j) {
					result(i, j) = static_cast<float>(M(j, i));
				}
			}
			return result;
		}
	};
}
