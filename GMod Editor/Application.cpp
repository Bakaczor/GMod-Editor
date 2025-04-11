#include "Application.h"
#include "../gmod/utility.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const float Application::selectionRadius = 0.05f;
const float Application::traSensitivity = 0.005f;
const float Application::rotSensitivity = 0.005f;
const float Application::scaSensitivity = 0.005f;
const std::wstring Application::m_appName = L"GMod Editor";
int Application::m_winWidth = 1024;
int Application::m_winHeight = 720;
const float Application::m_near = 0.01f;
const float Application::m_far = 1000.0f;
const float Application::m_FOV = DirectX::XM_PIDIV2;

Application::Application(HINSTANCE hInstance) : WindowApplication(hInstance, m_winWidth, m_winHeight, m_appName),
	m_device(m_window), m_camera(0.0f), 
	m_constBuffModel(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffView(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffProj(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffColor(m_device.CreateConstantBuffer<DirectX::XMFLOAT4>())
{
	m_UI.objects.push_back(std::make_unique<Cube>());
	m_mouse.prevCursorPos = {
		static_cast<LONG>(m_winWidth),
		static_cast<LONG>(m_winHeight)
	};

	Initialize();

	// SHADERS
	const auto vsBytes = Device::LoadByteCode(L"vs.cso");
	const auto psBytes = Device::LoadByteCode(L"ps.cso");
	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);

	// LAYOUT
	m_layout = m_device.CreateInputLayout<Vertex_Po>(vsBytes);
	SetShadersAndLayout();

	// CONSTANT BUFFERS
	ID3D11Buffer* vsb[] = { m_constBuffModel.get(),  m_constBuffView.get(), m_constBuffProj.get() };
	m_device.deviceContext()->VSSetConstantBuffers(0, 3, vsb);
	ID3D11Buffer* psb[] = { m_constBuffColor.get() };
	m_device.deviceContext()->PSSetConstantBuffers(0, 1, psb);

	// STATES
	RasterizerDescription rsdesc;
	m_rastState = m_device.CreateRasterizerState(rsdesc);
	m_device.deviceContext()->RSSetState(m_rastState.get());

	// GLOBAL
	m_axes.UpdateMesh(m_device);

	m_UI.cursor.transform.SetScaling(0.5, 0.5, 0.5);
	m_UI.cursor.UpdateMesh(m_device);

	m_UI.selection.midpoint.color = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_UI.selection.midpoint.SetScaling(0.75, 0.75, 0.75);
	m_UI.selection.midpoint.UpdateMesh(m_device);
}

void Application::Initialize() {
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
			m_device.swapChain()->Present(0, 0);

			if (m_firstPass) { m_firstPass = false; }
		}
	} while (msg.message != WM_QUIT);
	return static_cast<int>(msg.wParam);
}

void Application::SetShadersAndLayout() {
	m_device.deviceContext()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.deviceContext()->PSSetShader(m_pixelShader.get(), nullptr, 0);
	m_device.deviceContext()->IASetInputLayout(m_layout.get());
	m_device.deviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void Application::ResizeWnd() {
	m_wndSizeChanged = false;
	if (m_device.device()) {
		m_backBuffer.reset();
		m_depthBuffer.reset();
		m_device.swapChain()->ResizeBuffers(0, m_winWidth, m_winHeight, DXGI_FORMAT_UNKNOWN, 0);

		Initialize();
	}
}

void Application::Update() {
	if (m_wndSizeChanged || m_firstPass) {
		ResizeWnd();
		DirectX::XMFLOAT4X4 projMtx = matrix4_to_XMFLOAT4X4(projMatrix());
		m_device.UpdateBuffer(m_constBuffProj, projMtx);
	}

	if (m_camera.cameraChanged || m_firstPass) {
		m_camera.cameraChanged = false;
		DirectX::XMFLOAT4X4 viewMtx = matrix4_to_XMFLOAT4X4(m_camera.viewMatrix());
		m_device.UpdateBuffer(m_constBuffView, viewMtx);
	}
}

float Application::aspect() const {
	return static_cast<float>(m_winWidth) / static_cast<float>(m_winHeight);
}

gmod::matrix4<float> Application::projMatrix() const {
	const float Height = 1 / std::tan(0.5f * m_FOV);
	const float Width = Height / aspect();
	const float Range = m_far / (m_near - m_far);

	return gmod::matrix4<float>(
		Width, 0,       0,     0,
		0,	   Height,  0,     0,
		0,	   0,	    Range, Range * m_near,
		0,     0,      -1,	   0
	);
}

gmod::matrix4<float> Application::projMatrix_inv() const {
	const float Height_1 = std::tan(0.5f * m_FOV);
	const float Width_1 = Height_1 * aspect();
	const float Range_1 = (m_near - m_far) / m_far;

	return gmod::matrix4<float>(
		Width_1, 0,		   0,						0,
		0,		 Height_1, 0,						0,
		0,		 0,		   0,					   -1,
		0,		 0,		   Range_1 * (1 / m_near),  1 / m_near
	);
}

void Application::RenderUI() {
	m_device.deviceContext()->ClearRenderTargetView(m_backBuffer.get(), reinterpret_cast<float*>(&m_UI.bkgdColor));
	m_device.deviceContext()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	m_UI.Render(m_firstPass);
	ImGui::Render();
}

void Application::Render() {
	m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(m_UI.cursor.transform.modelMatrix()));
	m_UI.cursor.RenderMesh(m_device, m_constBuffColor);

	if (!m_UI.selection.empty()) {
		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(m_UI.selection.midpoint.modelMatrix()));
		m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(m_UI.selection.midpoint.color.data()));
		m_UI.selection.midpoint.RenderMesh(m_device.deviceContext());
	}

	if (m_UI.showAxes) {
		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(m_axes.modelMatrix(m_camera)));
		m_axes.RenderMesh(m_device, m_constBuffColor);
	}

	for (auto& object : m_UI.objects) {
		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(object->modelMatrix()));
		m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(object->color.data()));
		if (object->geometryChanged || m_firstPass) {
			object->geometryChanged = false;
			object->UpdateMesh(m_device);
		}
		object->RenderMesh(m_device.deviceContext());
	}
}

void Application::HandleTransformsOnMouseMove(LPARAM lParam) {
	if (!m_mouse.isLMBDown_flag || m_UI.currentMode == UI::Mode::Neutral) { return; }
	float dx = static_cast<float>(Mouse::GetXPos(lParam) - m_mouse.prevCursorPos.x);
	float dy = static_cast<float>(Mouse::GetYPos(lParam) - m_mouse.prevCursorPos.y);
	gmod::vector3<float> trans;
	if (m_UI.currentMode != UI::Mode::Rotate) {
		std::swap(dx, dy);
		dx = -dx;
	}
	switch (m_UI.currentAxis) {
		case UI::Axis::X: {
			trans = { dy, 0.0f, 0.0f };
			break;
		}
		case UI::Axis::Y: {
			trans = { 0.0f, dx, 0.0f };
			break;
		}
		case UI::Axis::Z: {
			trans = { 0.0f, 0.0f, -dy };
			break;
		}
		case UI::Axis::All: {
			trans = { dx, dx, dx };
			break;
		}
		default: {
			trans = { 0.0f, 0.0f, 0.0f };
			break;
		}
	}
	switch (m_UI.currentMode) {
		case UI::Mode::Translate: {
			trans = trans * traSensitivity;
			if (m_UI.noObjectSelected()) {
				m_UI.cursor.transform.UpdateTranslation(trans.x(), trans.y(), trans.z());
			} else {
				auto& selectedObj = m_UI.objects.at(m_UI.objects_selectedRowIdx);
				selectedObj->UpdateTranslation(trans.x(), trans.y(), trans.z());
				selectedObj->InformParents();
			}
			break;
		}
		case UI::Mode::Rotate: {
			if (m_UI.noObjectSelected()) { return; }
			trans = trans * rotSensitivity;
			switch (m_UI.currentOrientation) {
				case UI::Orientation::World: {
					m_UI.objects.at(m_UI.objects_selectedRowIdx)->UpdateRotation_Quaternion(trans.x(), trans.y(), trans.z());
					break;
				}
				case UI::Orientation::Cursor: {
					m_UI.objects.at(m_UI.objects_selectedRowIdx)->UpdateRotationAroundPoint_Quaternion(trans.x(), trans.y(), trans.z(),
						m_UI.cursor.transform.position());
					break;
				}
				case UI::Orientation::Selection:{
					m_UI.objects.at(m_UI.objects_selectedRowIdx)->UpdateRotationAroundPoint_Quaternion(trans.x(), trans.y(), trans.z(),
						m_UI.selection.UpdateMidpoint());
					break;
				}
			}
			break;
		}
		case UI::Mode::Scale: {
			if (m_UI.noObjectSelected()) { return; }
			trans = trans * scaSensitivity;
			switch (m_UI.currentOrientation) {
				case UI::Orientation::World: {
					m_UI.objects.at(m_UI.objects_selectedRowIdx)->UpdateScaling(trans.x(), trans.y(), trans.z());
					break;
				}
				case UI::Orientation::Cursor: {
					m_UI.objects.at(m_UI.objects_selectedRowIdx)->UpdateScalingAroundPoint(trans.x(), trans.y(), trans.z(),
						m_UI.cursor.transform.position());
					break;
				}
				case UI::Orientation::Selection: {
					m_UI.objects.at(m_UI.objects_selectedRowIdx)->UpdateScalingAroundPoint(trans.x(), trans.y(), trans.z(),
						m_UI.selection.UpdateMidpoint());
					break;
				}
			}
			break;
		}
		default: return;
	}
	m_UI.selection.UpdateMidpoint();
}

void Application::HandleCameraOnMouseMove(LPARAM lParam) {
	if (!m_mouse.isMMBDown_flag && !m_mouse.isRMBDown_flag) { return; }

	float dx = static_cast<float>(Mouse::GetXPos(lParam) - m_mouse.prevCursorPos.x);
	float dy = static_cast<float>(Mouse::GetYPos(lParam) - m_mouse.prevCursorPos.y);

	bool MMB = m_UI.useMMB && m_mouse.isMMBDown_flag;
	bool RMB = !m_UI.useMMB && m_mouse.isRMBDown_flag;
	if (MMB) {
		if (m_mouse.isShiftDown_flag) {
			m_camera.Move(dx, dy);
		} else {
			m_camera.Rotate(dx, dy);
		}
		m_camera.cameraChanged = true;
	} else if (RMB) {
		if (m_mouse.isShiftDown_flag) {
			if (m_mouse.isCtrlDown_flag) {
				m_camera.Zoom(-dy);
			} else {
				m_camera.Move(dx, dy);
			}
		} else {
			m_camera.Rotate(dx, dy);
		}
		m_camera.cameraChanged = true;
	}
}

void Application::HandleCameraOnMouseWheel(WPARAM wParam) {
	if (!m_UI.useMMB) { return; }
	float dd = static_cast<float>(m_mouse.GetWheelDelta(wParam));
	m_camera.Zoom(dd);
	m_camera.cameraChanged = true;
}

bool Application::ProcessMessage(mini::WindowMessage& msg) {
	msg.result = 0;
	if (ImGui_ImplWin32_WndProcHandler(m_window.getHandle(), msg.message, msg.wParam, msg.lParam)) {
		return true;
	}

	switch (msg.message) {
		case WM_LBUTTONDOWN: {
			m_mouse.isLMBDown_flag = true;
			Object* clicked = nullptr;
			if (clicked = HandleSelectionOnMouseClick(msg.lParam)) {
				m_UI.SelectObjectOnMouseClick(clicked);
			}
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_LBUTTONUP: {
			m_mouse.isLMBDown_flag = false;
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_RBUTTONDOWN: {
			m_mouse.isRMBDown_flag = true;
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_RBUTTONUP: {
			m_mouse.isRMBDown_flag = false;
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_MBUTTONDOWN: {
			m_mouse.isMMBDown_flag = true;
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_MBUTTONUP: {
			m_mouse.isMMBDown_flag = false;
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_MOUSEMOVE: {
			m_mouse.UpdateFlags(msg.wParam);
			HandleCameraOnMouseMove(msg.lParam);
			HandleTransformsOnMouseMove(msg.lParam);
			m_mouse.UpdatePos(msg.lParam);
			break;
		}
		case WM_MOUSEWHEEL: {
			m_mouse.UpdateFlags(msg.wParam);
			HandleCameraOnMouseWheel(msg.wParam);
			m_mouse.UpdateDist(msg.wParam);
			break;
		}
		case WM_SIZE: {
			if (msg.wParam != SIZE_MINIMIZED) {
				m_winWidth = LOWORD(msg.lParam);
				m_winHeight = HIWORD(msg.lParam);
				m_wndSizeChanged = true;
			}
			break;
		}
		case WM_ERASEBKGND: {
			msg.result = 0;
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

Object* Application::HandleSelectionOnMouseClick(LPARAM lParam) {
	if (m_UI.numOfPointObjects == 0) { return nullptr; }
	const float x = static_cast<float>(Mouse::GetXPos(lParam));
	const float y = static_cast<float>(Mouse::GetYPos(lParam));
	const float nx = (2.0f * x) / m_winWidth - 1.0f;
	const float ny = 1.0f - (2.0f * y) / m_winHeight;

	const gmod::matrix4<float> invProj = this->projMatrix_inv();
	const gmod::vector4<float> nearPoint = gmod::transform_coord(gmod::vector4<float>(nx, ny, 0.0f, 1.0f), invProj);
	const gmod::vector4<float> farPoint = gmod::transform_coord(gmod::vector4<float>(nx, ny, 1.0f, 1.0f), invProj);

	const gmod::matrix4<float> invView = m_camera.viewMatrix_inv();
	const gmod::vector4<float> rayOrigin = gmod::transform_coord(nearPoint, invView);
	const gmod::vector4<float> rayEnd = gmod::transform_coord(farPoint, invView);
	const gmod::vector4<float> rayDir = gmod::normalize(rayEnd - rayOrigin);

	const gmod::vector3<double> origin(rayOrigin.x(), rayOrigin.y(), rayOrigin.z());
	const gmod::vector3<double> direction(rayDir.x(), rayDir.y(), rayDir.z());

	Object* closestPoint = nullptr;
	for (auto& obj : m_UI.objects) {
		if (nullptr == dynamic_cast<Point*>(obj.get())) { continue; }
		const gmod::vector3<double> vecToPoint = obj->position() - origin;
		const gmod::vector3<double> crossProd = gmod::cross(vecToPoint, direction);

		if (crossProd.length() < selectionRadius) {
			closestPoint = obj.get();
			break;
		}
	}
	return closestPoint;
}

