#pragma once
#include "pch.h"
#include "../mini/windowApplication.h"
#include "Device.h"
#include "../gmod/vector3.h"
#include "../gmod/matrix4.h"
#include "../gmod/quaternion.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "UI.h"
#include "Torus.h"
#include "Camera.h"

class Application : public mini::WindowApplication {
public:
	explicit Application(HINSTANCE hInstance);
	~Application();
	int Run(int cmdShow = SW_SHOWNORMAL) override;

protected:
	int MainLoop() override;
	bool ProcessMessage(mini::WindowMessage& msg) override;

private:
	Mouse m_mouse;
	Keyboard m_keyboard;
	Camera m_camera;
	UI m_UI;
	Torus m_torus;

	void SetShaders();
	void RenderUI();
	void Update();
	void Render();
	void UpdateBuffer(const mini::dx_ptr<ID3D11Buffer>& buffer, const void* data, std::size_t count);

	template<typename T>
	void UpdateBuffer(const mini::dx_ptr<ID3D11Buffer>& buffer, const T& data) {
		UpdateBuffer(buffer, &data, sizeof(T));
	}

#pragma region APPLICATION
	const static float traSensitivity;
	const static float rotSensitivity;
	const static float scaSensitivity;
	const static std::wstring m_appName;
	static int m_winWidth;
	static int m_winHeight;
	bool m_firstPass = true;
	bool m_wndSizeChanged = false;
#pragma endregion

#pragma region PROJECTION
	const static float m_near;
	const static float m_far;
	const static float m_FOV;
	float aspect() const;
	gmod::matrix4<float> projMatrix() const;
#pragma endregion

#pragma region DEVICE
	Device m_device;
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;

	mini::dx_ptr<ID3D11Buffer> m_constBuffModel;
	mini::dx_ptr<ID3D11Buffer> m_constBuffView;
	mini::dx_ptr<ID3D11Buffer> m_constBuffProj;
#pragma endregion

	template<typename T>
	static DirectX::XMFLOAT4X4 matrix4_to_XMFLOAT4X4(const gmod::matrix4<T>& M) {
		DirectX::XMFLOAT4X4 result;
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				result(i, j) = static_cast<float>(M(i, j));
			}
		}
		return result;
	}
};
