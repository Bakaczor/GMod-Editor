#include "pch.h"
#include "Application.h"
#include "Solver.h"
#include <algorithm>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const std::wstring Application::m_applicationName = L"Ellipsoid";
const int Application::m_startWindowWidth = 970;
const int Application::m_startWindowHeight = 720;

Application::Application(HINSTANCE hInstance) : WindowApplication(hInstance, m_startWindowWidth, m_startWindowHeight, m_applicationName),
m_device(m_window), m_image(m_textureHeight, std::vector<ImVec4>(m_textureWidth)) {
	m_Mr = gmod::matrix4<float>::identity();
	m_Qr = gmod::quaternion<float>();

	// RENDER TARGET
	ID3D11Texture2D* temp = nullptr;
	mini::dx_ptr<ID3D11Texture2D> backTexture;
	m_device.swapChain()->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&temp));
	backTexture.reset(temp);
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

	// TEXTURE
	SetTexture(m_textureWidth, m_textureHeight);

	// SHADERS
	const auto vsBytes = Device::LoadByteCode(L"vs.cso");
	const auto psBytes = Device::LoadByteCode(L"ps.cso");
	m_vertexShader = m_device.CreateVertexShader(vsBytes);
	m_pixelShader = m_device.CreatePixelShader(psBytes);

	m_device.deviceContext()->VSSetShader(m_vertexShader.get(), nullptr, 0);
	m_device.deviceContext()->PSSetShader(m_pixelShader.get(), nullptr, 0);

	// QUAD
	const auto vertices = CreateQuad();
	m_vertexBuffer = m_device.CreateVertexBuffer(vertices);
	const auto indices = CreateQuadIndices();
	m_indexBuffer = m_device.CreateIndexBuffer(indices);

	// LAYOUT
	std::vector<D3D11_INPUT_ELEMENT_DESC> elements {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	m_layout = m_device.CreateInputLayout(elements, vsBytes);

	m_device.deviceContext()->IASetInputLayout(m_layout.get());
	m_device.deviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// BUFFERS
	ID3D11Buffer* vbs[] = { m_vertexBuffer.get() };
	UINT strides[] = { sizeof(VERTEX) };
	UINT offsets[] = { 0 };
	m_device.deviceContext()->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	m_device.deviceContext()->IASetIndexBuffer(m_indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
}

Application::~Application() {
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Application::SetTexture(int width, int height) {
	auto desc = Texture2DDescription::DynamicTextureDescription(width, height);
	m_texture = m_device.CreateTexture(desc);
	m_textureSRV = m_device.CreateShaderResourceView(m_texture);
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
			Render();
		}
	} while (msg.message != WM_QUIT);
	return static_cast<int>(msg.wParam);
}

void Application::Update() {
	if (!m_uiChanged && m_mouseMoved && (m_isLeftButtonDown || m_isRightButtonDown)) {
		m_interacted = true;
		m_timeSinceInteraction = 0.0f;
	} else {
		if (m_timeSinceInteraction > 0.0f) {
			if (m_timeSinceInteraction < 0.1f) {
				m_timeSinceInteraction += ImGui::GetIO().DeltaTime;
			} else {
				m_timeSinceInteraction = -2.0f;
				m_interacted = false;
				m_stateChanged = true;
			}
		} else if (m_timeSinceInteraction > -1.0f) {
			m_timeSinceInteraction += ImGui::GetIO().DeltaTime;
		}
	}

	if (!m_uiChanged && m_interacted) {
		m_currStep = m_step;
	} else {
		m_currStep = 1;
	}
	if (m_stateChanged || m_firstPass) {
		m_firstPass = false;
		m_stateChanged = false;
		RenderImage();
		UpdateTexture();
	}
	m_uiChanged = false;
}

void Application::UpdateTexture() {
	D3D11_MAPPED_SUBRESOURCE mapped;
	m_device.deviceContext()->Map(m_texture.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	auto* data = static_cast<uint8_t*>(mapped.pData);
	for (int y = 0; y < m_textureHeight; y++) {
		for (int x = 0; x < m_textureWidth; x++) {
			data[y * mapped.RowPitch + x * 4 + 0] = static_cast<uint8_t>(std::clamp(m_image[y][x].x * 255, 0.0f, 255.0f));
			data[y * mapped.RowPitch + x * 4 + 1] = static_cast<uint8_t>(std::clamp(m_image[y][x].y * 255, 0.0f, 255.0f));
			data[y * mapped.RowPitch + x * 4 + 2] = static_cast<uint8_t>(std::clamp(m_image[y][x].z * 255, 0.0f, 255.0f));
			data[y * mapped.RowPitch + x * 4 + 3] = static_cast<uint8_t>(255);
		}
	}
	m_device.deviceContext()->Unmap(m_texture.get(), 0);
}

gmod::matrix4<float> Application::CalculateEquationMatrix() {
	const gmod::matrix4<float> D(gmod::vector4<float>(1 / m_a, 1 / m_b, 1 / m_c, -1));
	const gmod::matrix4<float> Ms = gmod::matrix4<float>::inv_scaling(m_scale, m_scale, m_scale);
#if ROTATION_COMBINATION
	if (m_rotate) {
		m_rotate = false;
		// === equivalent using matrix ===
		// m_Mr = m_Mr * gmod::matrix4<float>::inv_rotationYX(m_yaw, m_pitch);
		// m_Mr.orthonormalize();
		auto qy = gmod::quaternion<float>::from_angle_axis(-m_yaw, 0, 1, 0);
		auto qx = gmod::quaternion<float>::from_angle_axis(-m_pitch, 1, 0, 0);
		m_Qr = m_Qr * qy * qx;
		m_Qr.normalize();
		m_Mr = m_Qr.unit_to_rotation();
	}
#else
	// === equivalent using rodrigues ===
	// auto Ry_inv = gmod::matrix4<float>::inv_rotationY(m_yaw);
	// gmod::vector4<float> OX(1, 0, 0, 1);
	// m_Mr = gmod::matrix4<float>::rotation(Ry_inv * OX, -m_pitch) * Ry_inv;
	// === equivalent using quaternions ===
	// auto q = gmod::quaternion<float>::from_euler(-m_pitch, -m_yaw, 0).normalized();
	// m_Mr = q.unit_to_rotation();
	m_Mr = gmod::matrix4<float>::inv_rotationYX(m_yaw, m_pitch);
#endif
	const gmod::matrix4<float> Mt = gmod::matrix4<float>::inv_translation(m_translation.x, m_translation.y, 0);
	const gmod::matrix4<float> M = Ms * (m_Mr * Mt);
	const gmod::matrix4<float> M_T = Mt.transposed() * (m_Mr.transposed() * Ms);
	const gmod::matrix4<float> Dm = M_T * D * M;
	return Dm;
}

void Application::RenderImage() {
	gmod::matrix4<float> A = CalculateEquationMatrix();
	const float height2 = m_textureHeight / 2.0f;
	const float width2 = m_textureWidth / 2.0f;
	const float ratio = m_textureWidth / 5.0f;
	const float ambient = 0.1f;
	const int n = (m_step - 1) / 2;

	for (int y = 0; y < m_textureHeight; y += m_currStep) {
		for (int x = 0; x < m_textureWidth; x += m_currStep) {
#if OPTIMIZED_ADAPTATION
			if (!m_uiChanged && m_step != 1 && m_currStep == 1) {
				if (y % m_step == n && x % m_step == n) {
					continue;
				}
			}
#endif
			const float accuracy2 = m_currStep / 2.0f;
			const float ypos = (y + accuracy2 - height2) / ratio;
			const float xpos = (x + accuracy2 - width2) / ratio;
			std::pair<bool, float> result = Solver::CalculateDepth(xpos, ypos, 1.0f, A);

			auto fill_kernel = [&](const ImVec4& color) -> void {
				for (int dy = 0; dy < m_currStep && y + dy < m_textureHeight; dy++) {
					for (int dx = 0; dx < m_currStep && x + dx < m_textureWidth; dx++) {
						m_image[y + dy][x + dx] = color;
					}
				}
			};

			if (result.first) {
				const float zpos = result.second;
				const gmod::vector4<float> pos(xpos, ypos, zpos, 1);
				const gmod::vector3<float> N = Solver::CalculateGradient(pos, A).normalized();
				const gmod::vector3<float> V = gmod::normalize(m_cameraPosition - gmod::vector3<float>(pos.x(), pos.y(), pos.z()));
				float cosVNm = std::pow(std::max(gmod::dot(V, N), 0.0f), m_shininess);

				if (m_addAmbient) {
					cosVNm += ambient;
				}
				float r = std::clamp(cosVNm * m_ellipsoidColor.x, 0.0f, 1.0f);
				float g = std::clamp(cosVNm * m_ellipsoidColor.y, 0.0f, 1.0f);
				float b = std::clamp(cosVNm * m_ellipsoidColor.z, 0.0f, 1.0f);

				fill_kernel(ImVec4(r, g, b, 1));
			} else {
				fill_kernel(m_backgroundColor);
			}
		}
	}
}

#pragma region GEOMETRY
std::vector<Application::VERTEX> Application::CreateQuad() {
	return {
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } }, // 0: Bottom-left
		{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }, // 1: Bottom-right
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } }, // 2: Top-right
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } }, // 3: Top-left
	};
}

std::vector<USHORT> Application::CreateQuadIndices() {
	return { 0, 2, 1, 0, 3, 2 };
}
#pragma endregion

#pragma region UI
void Application::RenderImGui() {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	const float width = 250.0f;
	ImGui::SetNextWindowPos(ImVec2(viewportSize.x - width, 0.0f), ImGuiCond_Always);
#if ROTATION_SWITCH 
	ImGui::SetNextWindowSize(ImVec2(width, 315.0f), ImGuiCond_Always);
#else
	ImGui::SetNextWindowSize(ImVec2(width, 265.0f), ImGuiCond_Always);
#endif
	ImGui::Begin("Parameters", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
#if ROTATION_SWITCH 
	RenderRotations();
#endif
	RenderEllipsoid();
	RenderRendering();
	ImGui::End();

	const float height = 80.0f;
	ImGui::SetNextWindowPos(ImVec2(0.0f, viewportSize.y - height), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(320.0f, height), ImGuiCond_Always);

	if (m_showColors) {
		ImGui::Begin("Colors", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		RenderColors();
		ImGui::End();
	}
	ImGui::Render();
}

void Application::RenderRotations() {
	if (m_firstPass) {
		ImGui::SetNextItemOpen(true);
	}
	if (ImGui::CollapsingHeader("Rotations")) {
		if (ImGui::RadioButton("X", &m_rotation, 0)) {
			m_uiChanged = true;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("Y", &m_rotation, 1)) {
			m_uiChanged = true;
		}
	}
}

void Application::RenderEllipsoid() {
	if (m_firstPass) {
		ImGui::SetNextItemOpen(true);
	}
	if (ImGui::CollapsingHeader("Ellipsoid")) {
		if (ImGui::InputFloat("a", &m_a, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			m_stateChanged = true;
			m_uiChanged = true;
		}
		if (ImGui::InputFloat("b", &m_b, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			m_stateChanged = true;
			m_uiChanged = true;
		}
		if (ImGui::InputFloat("c", &m_c, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			m_stateChanged = true;
			m_uiChanged = true;
		}
		if (ImGui::SliderFloat("scale", &m_scale, 0.0f, 2.0f, "%.2f")) {
			m_scale = std::max(0.001f, m_scale);
			m_stateChanged = true;
			m_uiChanged = true;
		}
	}
}

void Application::RenderRendering() {
	if (m_firstPass) {
		ImGui::SetNextItemOpen(true);
	}
	if (ImGui::CollapsingHeader("Rendering")) {
		if (ImGui::InputInt("step", &m_stepUI, 1, 10)) {
			m_stepUI = std::max(1, m_stepUI);
			m_step = m_stepUI;
			m_stateChanged = true;
			m_uiChanged = true;
#if OPTIMIZED_ADAPTATION
			m_step = 2 * m_step - 1;
#endif
		}
		if (ImGui::InputFloat("shininess", &m_shininess, 0.1f, 1.0f, "%.1f", ImGuiInputTextFlags_CharsDecimal)) {
			m_shininess = std::max(0.0f, m_shininess);
			m_stateChanged = true;
			m_uiChanged = true;
		}
		ImGui::Separator();
		ImGui::Checkbox("show colors", &m_showColors);
		ImGui::Checkbox("add ambient", &m_addAmbient);
	}
}

void Application::RenderColors() {
	if (ImGui::ColorEdit3("background", (float*)&m_backgroundColor)) {
		m_stateChanged = true;
		m_uiChanged = true;
	}
	if (ImGui::ColorEdit3("ellipsoid", (float*)&m_ellipsoidColor)) {
		m_stateChanged = true;
		m_uiChanged = true;
	}
}
#pragma endregion

void Application::Render() {
	// const float backgroundColor[4] = { m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w };
	// m_device.deviceContext()->ClearRenderTargetView(m_backBuffer.get(), backgroundColor);
	m_device.deviceContext()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	RenderImGui();
	Update();

	ID3D11ShaderResourceView* srvs[] = { m_textureSRV.get() };
	m_device.deviceContext()->PSSetShaderResources(0, 1, srvs);

	m_device.deviceContext()->DrawIndexed(6, 0, 0);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	m_device.swapChain()->Present(1, 0);
}

bool Application::ProcessMessage(mini::WindowMessage& msg) {
	msg.result = 0;
	if (ImGui_ImplWin32_WndProcHandler(m_window.getHandle(), msg.message, msg.wParam, msg.lParam)) {
		return true;
	}

	switch (msg.message) {
		case WM_LBUTTONDOWN: {
			m_isLeftButtonDown = true;
			m_prevMousePos.x = LOWORD(msg.lParam);
			m_prevMousePos.y = HIWORD(msg.lParam);
			break;
		}
		case WM_LBUTTONUP: {
			m_isLeftButtonDown = false;
			break;
		}
		case WM_RBUTTONDOWN: {
			m_isRightButtonDown = true;
			m_prevMousePos.x = LOWORD(msg.lParam);
			m_prevMousePos.y = HIWORD(msg.lParam);
			break;
		}
		case WM_RBUTTONUP: {
			m_isRightButtonDown = false;
			break;
		}
		case WM_MOUSEMOVE: {
			int currentX = LOWORD(msg.lParam);
			int currentY = HIWORD(msg.lParam);
			int deltaX = currentX - m_prevMousePos.x;
			int deltaY = currentY - m_prevMousePos.y;

			if (!m_uiChanged && m_isLeftButtonDown) {
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
			m_mouseMoved = true;
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
