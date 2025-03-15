#pragma once
#include "pch.h"
#include "../mini/windowApplication.h"
#include "Device.h"
#include "../gmod/vector3.h"
#include "../gmod/matrix4.h"
#include "../gmod/quaternion.h"

class Application : public mini::WindowApplication {
public:
	explicit Application(HINSTANCE hInstance);
	~Application();
	int Run(int cmdShow = SW_SHOWNORMAL) override;

protected:
	int MainLoop() override;
	bool ProcessMessage(mini::WindowMessage& msg) override;

private:
	void RenderImGui();
	void RenderImage();
	void Render();
	void Update();
	void UpdateTexture();
	void SetTexture(int width, int height);

#pragma region APPLICATION
	const static std::wstring m_applicationName;
	const static int m_startWindowWidth;
	const static int m_startWindowHeight;
#pragma endregion

#pragma region GEOMETRY
	struct VERTEX {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 uv;
	};
	static std::vector<VERTEX> CreateQuad();
	static std::vector<USHORT> CreateQuadIndices();
#pragma endregion

#pragma region DEVICE
	Device m_device;
	mini::dx_ptr<ID3D11RenderTargetView> m_backBuffer;
	mini::dx_ptr<ID3D11DepthStencilView> m_depthBuffer;
	mini::dx_ptr<ID3D11Buffer> m_vertexBuffer;
	mini::dx_ptr<ID3D11Buffer> m_indexBuffer;
	mini::dx_ptr<ID3D11VertexShader> m_vertexShader;
	mini::dx_ptr<ID3D11PixelShader> m_pixelShader;
	mini::dx_ptr<ID3D11InputLayout> m_layout;
#pragma endregion

#pragma region IMAGE
	const gmod::vector3<float> m_cameraPosition = { 0.0f, 0.0f, -10.0f };
	gmod::matrix4<float> CalculateEquationMatrix();
	gmod::matrix4<float> m_Mr;
	gmod::quaternion<float> m_Qr;
	bool m_rotate = false;

	int m_textureWidth = m_startWindowWidth;
	int m_textureHeight = m_startWindowHeight;
	std::vector<std::vector<ImVec4>> m_image;
	mini::dx_ptr<ID3D11Texture2D> m_texture;
	mini::dx_ptr<ID3D11ShaderResourceView> m_textureSRV;
#pragma endregion

#pragma region STATE
	bool m_firstPass = true;
	bool m_stateChanged = false;
	bool m_isLeftButtonDown = false;
	bool m_isRightButtonDown = false;
	bool m_mouseMoved = false;
	bool m_interacted = false;
	float m_timeSinceInteraction = 0.0f;
	bool m_uiChanged = false;

	POINT m_prevMousePos = { 
		static_cast<LONG>(m_startWindowWidth / 2.0f),
		static_cast<LONG>(m_startWindowHeight / 2.0f)
	};

	float m_angleMult = 0.01f;
	float m_pitch = 0.0f;
	float m_yaw = 0.0f;
	float m_vecMult = 0.005f;
	struct VECTOR2 { float x, y; };
	VECTOR2 m_translation = { 0.0f, 0.0f };
#pragma endregion

#pragma region UI
	int m_rotation = 0;
	void RenderRotations();

	float m_a = 0.5f;
	float m_b = 1.0f;
	float m_c = 1.5f;
	float m_scale = 1.0f;
	void RenderEllipsoid();

	int m_stepUI = 1;
	int m_step = 1;
	int m_currStep = m_step;
	float m_shininess = 4.0f;
	void RenderRendering();

	bool m_showColors = false;
	bool m_addAmbient = false;

	ImVec4 m_backgroundColor = ImVec4(0.5f, 0.25f, 0.5f, 1.0f);
	ImVec4 m_ellipsoidColor = ImVec4(1.0f, 0.9f, 0.0f, 1.0f);
	void RenderColors();
#pragma endregion
};
