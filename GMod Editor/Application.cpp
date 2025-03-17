#include "pch.h"
#include "Application.h"
#include "../mini/exceptions.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const float Application::traSensitivity = 0.005f;
const float Application::rotSensitivity = 0.01f;
const float Application::scaSensitivity = 0.01f;
const std::wstring Application::m_appName = L"GMod Editor";
int Application::m_winWidth = 970;
int Application::m_winHeight = 720;
const float Application::m_near = 10.0f;
const float Application::m_far = 100.0f;
const float Application::m_FOV = DirectX::XM_PIDIV4;

Application::Application(HINSTANCE hInstance) : WindowApplication(hInstance, m_winWidth, m_winHeight, m_appName), m_device(m_window), m_torus(2.0f, 1.0f, 8, 8),
	m_constBuffModel(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffView(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffProj(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>())
{
	m_mouse.prevCursorPos = {
		static_cast<LONG>(m_winWidth),
		static_cast<LONG>(m_winHeight)
	};

	// RENDER TARGET
	ID3D11Texture2D* temp = nullptr;
	auto hr = m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	const mini::dx_ptr<ID3D11Texture2D> backTexture(temp);
	m_backBuffer = m_device.CreateRenderTargetView(backTexture);

	// DEPTH STENCIL
	SIZE size = m_window.getClientSize();
	m_depthBuffer = m_device.CreateDepthStencilView(size);

	// OUTPUT MERGER
	auto backBuffer = m_backBuffer.get();
	m_device.deviceContext()->OMSetRenderTargets(1, &backBuffer, m_depthBuffer.get());

	// VIEWPORT
	Viewport viewport{ size };
	m_device.deviceContext()->RSSetViewports(1, &viewport);

	// SHADERS
	const auto vsBytes = Device::LoadByteCode(L"vs.cso");
	const auto psBytes = Device::LoadByteCode(L"ps.cso");
	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);

	// LAYOUT
	std::vector<D3D11_INPUT_ELEMENT_DESC> elements {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout = m_device.CreateInputLayout(elements, vsBytes);

	SetShaders();

	// BUFFERS
	ID3D11Buffer* vsb[] = { m_constBuffModel.get(),  m_constBuffView.get(), m_constBuffProj.get() };
	m_device.deviceContext()->VSSetConstantBuffers(0, 3, vsb);
	// ID3D11Buffer* psb[] = { nullptr };
	// m_device.deviceContext()->PSSetConstantBuffers(0, 0, psb);
}

Application::~Application() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

int Application::Run(int cmdShow) {
	m_window.Show(cmdShow);
	UpdateWindow(m_window.getHandle());

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); 
	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(m_window.getHandle());
	ImGui_ImplDX11_Init(m_device.device().get(), m_device.deviceContext().get());

	return MainLoop();
}

int Application::MainLoop() {
	MSG msg;
	ZeroMemory(&msg, sizeof msg);
	do {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
			RenderUI();
			Update();
			Render();

			ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
			m_device.swapChain()->Present(1, 0);
		}
	} while (msg.message != WM_QUIT);
	return static_cast<int>(msg.wParam);
}

void Application::SetShaders() {
	m_device.deviceContext()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.deviceContext()->PSSetShader(m_pixelShader.get(), nullptr, 0);
	m_device.deviceContext()->IASetInputLayout(m_layout.get());
	m_device.deviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void Application::Update() {
	if (m_wndSizeChanged || m_firstPass) {
		DirectX::XMFLOAT4X4 projMtx = matrix4_to_XMFLOAT4X4(projMatrix());
		UpdateBuffer(m_constBuffProj, projMtx);
	}

	if (m_camera.cameraChanged || m_firstPass) {
		cameraChanged = false;
		//XMFLOAT4X4 cameraMtx;
		//DirectX::XMStoreFloat4x4(&cameraMtx, m_camera.getViewMatrix());
		//XMMATRIX mtx = XMLoadFloat4x4(&cameraMtx);
		/*XMVECTOR det;
		auto invvmtx = XMMatrixInverse(&det, mtx);
		XMFLOAT4X4 view[2] = { cameraMtx };
		XMStoreFloat4x4(view + 1, invvmtx);
		UpdateBuffer(m_cbView, view);*/
	}

	if (m_firstPass) {
		m_firstPass = false;
	}
	m_UI.uiChanged = false;
}

float Application::aspect() const {
	return static_cast<float>(m_winWidth) / static_cast<float>(m_winHeight);
}

gmod::matrix4<float> Application::projMatrix() const {
	const float CTG_FOV_2 = 1 / std::tan(m_FOV / 2);
	const float asp = aspect();
	const float f_n = m_far - m_near;

	return gmod::matrix4<float>(
		CTG_FOV_2 / asp, 0,		    0,						0,
		0,				 CTG_FOV_2, 0,						0,
		0,				 0,		    (m_far + m_near) / f_n, (-2 * m_far * m_near) / f_n,
		0,				 0,		    1,						0
	);
}

void Application::RenderUI() {
	const float clearColor[] = { 0.5f, 0.5f, 1.0f, 1.0f };
	m_device.deviceContext()->ClearRenderTargetView(m_backBuffer.get(), clearColor);
	m_device.deviceContext()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	const float width = 250.0f;
	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - width, 0.0f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, 315.0f), ImGuiCond_Always);
	ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	m_UI.RenderRotations(m_firstPass);
	m_UI.RenderEllipsoid(m_firstPass);
	m_UI.RenderRendering(m_firstPass);
	ImGui::End();

	const float height = 80.0f;
	ImGui::SetNextWindowPos(ImVec2(0.0f, viewportSize.y - height), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(320.0f, height), ImGuiCond_Always);

	if (m_UI.m_showColors) {
		ImGui::Begin("Colors", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		m_UI.RenderColors(m_firstPass);
		ImGui::End();
	}
	ImGui::Render();
}

void Application::Render() {
	if (false) {
		DirectX::XMFLOAT4X4 modelMtx = matrix4_to_XMFLOAT4X4(m_torus.transform.modelMatrix());
		UpdateBuffer(m_constBuffModel, modelMtx);
		m_torus.RenderMesh(m_device.deviceContext());
	}
}

void Application::UpdateBuffer(const mini::dx_ptr<ID3D11Buffer>& buffer, const void* data, std::size_t count) {
	D3D11_MAPPED_SUBRESOURCE res;
	auto hr = m_device.deviceContext()->Map(buffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	if (FAILED(hr)) {
		THROW_DX(hr);
	}
	memcpy(res.pData, data, count);
	m_device.deviceContext()->Unmap(buffer.get(), 0);
}

bool Application::ProcessMessage(mini::WindowMessage& msg) {
	msg.result = 0;
	if (ImGui_ImplWin32_WndProcHandler(m_window.getHandle(), msg.message, msg.wParam, msg.lParam)) {
		return true;
	}

	switch (msg.message) {
		case WM_LBUTTONDOWN: {
			//m_isLeftButtonDown = true;
			//m_prevMousePos.x = LOWORD(msg.lParam);
			//m_prevMousePos.y = HIWORD(msg.lParam);
			break;
		}
		case WM_LBUTTONUP: {
			//m_isLeftButtonDown = false;
			break;
		}
		case WM_RBUTTONDOWN: {
			//m_isRightButtonDown = true;
			//m_prevMousePos.x = LOWORD(msg.lParam);
			//m_prevMousePos.y = HIWORD(msg.lParam);
			break;
		}
		case WM_RBUTTONUP: {
			//m_isRightButtonDown = false;
			break;
		}
		case WM_MOUSEMOVE: {
			//if (m_mouse.positionChanged) {
			//	//m_camera.Rotate(d.y * ROTATION_SPEED, d.x * ROTATION_SPEED);
			//} else if (m_mouse.distanceChanged) {
			//	m_camera.Zoom(d.y * ZOOM_SPEED);
			//} else {
			//	return false;
			//}
			//return true;
			int currentX = LOWORD(msg.lParam);
			int currentY = HIWORD(msg.lParam);
			//int deltaX = currentX - m_prevMousePos.x;
			//int deltaY = currentY - m_prevMousePos.y;

			/*if (!m_uiChanged && m_isLeftButtonDown) {
#if ROTATION_COMBINATION	
			m_rotate = true;
	#if ROTATION_SWITCH
			if (m_rotation == 0) {
				m_pitch = deltaY * m_angleMult;
				m_yaw = 0.0f;
			} else if (m_rotation == 1) {
				m_pitch = 0.0f;
				m_yaw = -deltaX * m_angleMult;
			}
	#else
			m_pitch = deltaY * m_angleMult;
			m_yaw = -deltaX * m_angleMult;
	#endif		
#else
	#if ROTATION_SWITCH
			if (m_rotation == 0) {
				m_pitch += deltaY * m_angleMult;
			} else if (m_rotation == 1) {
				m_yaw += -deltaX * m_angleMult;
			}
	#else
			m_pitch += deltaY * m_angleMult;
			m_yaw += -deltaX * m_angleMult;
	#endif
#endif
				m_stateChanged = true;
			}
			if (!m_uiChanged && m_isRightButtonDown) {
				m_translation.x += deltaX * m_vecMult;
				m_translation.y += deltaY * m_vecMult;
				m_stateChanged = true;
			}

			m_prevMousePos.x = currentX;
			m_prevMousePos.y = currentY;
			m_mouseMoved = true;*/
			break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		case WM_CLOSE: {
			DestroyWindow(m_window.getHandle());
			break;
		}
		default: {
			msg.result = DefWindowProc(m_window.getHandle(), msg.message, msg.wParam, msg.lParam);
			break;
		}
	}
	return (msg.result == 0);
}
