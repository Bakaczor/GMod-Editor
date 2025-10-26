#include "../gmod/utility.h"
#include "Application.h"
#include "UI.h"
#include <utility>
#include "BSpline.h"

using namespace app;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

const float Application::selectionRadius = 0.1f;
const float Application::traSensitivity = 0.005f;
const float Application::rotSensitivity = 0.005f;
const float Application::scaSensitivity = 0.005f;
const std::wstring Application::m_appName = L"GMod Editor";
int Application::m_winWidth = 1024;
int Application::m_winHeight = 720;
const float Application::m_near = 0.05f;
const float Application::m_far = 100.0f;
const float Application::m_FOV = DirectX::XM_PIDIV2;

std::unique_ptr<AxesModel> Application::m_axesModel = std::make_unique<AxesModel>();
std::unique_ptr<CubeModel> Application::m_cubeModel = std::make_unique<CubeModel>();
std::unique_ptr<PointModel> Application::m_pointModel = std::make_unique<PointModel>();

Application::Application(HINSTANCE hInstance) : WindowApplication(hInstance, m_winWidth, m_winHeight, m_appName),
	m_device(m_window), m_camera(-2.0f), 
	m_constBuffModel(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffView(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffProj(m_device.CreateConstantBuffer<DirectX::XMFLOAT4X4>()),
	m_constBuffColor(m_device.CreateConstantBuffer<DirectX::XMFLOAT4>()),
	m_constBuffTessConst(m_device.CreateConstantBuffer<TessellationConstants>()),
	m_constBuffTrimInfo(m_device.CreateConstantBuffer<TrimmingInfo>())
{
	m_UI = std::make_unique<UI>();
	m_mouse.prevCursorPos = {
		static_cast<LONG>(m_winWidth),
		static_cast<LONG>(m_winHeight)
	};

	Initialize();

	// SHADERS
	{
		// Regular
		const auto vsBytes_r = Device::LoadByteCode(L"vs_r.cso");
		const auto psBytes_r = Device::LoadByteCode(L"ps_r.cso");
		m_shaders.insert(std::make_pair(ShaderType::Regular, Shaders{
				m_device.CreateVertexShader(vsBytes_r),
				nullptr, // domain
				nullptr, // hull
				nullptr, // geometry
				m_device.CreatePixelShader(psBytes_r),
				m_device.CreateInputLayout<Vertex_Po>(vsBytes_r)
			}
		));
		// RegularWithColors
		const auto vsBytes_rwc = Device::LoadByteCode(L"vs_rwc.cso");
		const auto psBytes_rwc = Device::LoadByteCode(L"ps_rwc.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithColors, Shaders{
				m_device.CreateVertexShader(vsBytes_rwc),
				nullptr, // domain
				nullptr, // hull
				nullptr, // geometry
				m_device.CreatePixelShader(psBytes_rwc),
				m_device.CreateInputLayout<Vertex_PoCo>(vsBytes_rwc)
			}
		));
		// RegularWithUVs
		const auto vsBytes_rwuvs = Device::LoadByteCode(L"vs_rwuvs.cso");
		const auto psBytes_rwuvs = Device::LoadByteCode(L"ps_rwuvs.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithUVs, Shaders{
				m_device.CreateVertexShader(vsBytes_rwuvs),
				nullptr, // domain
				nullptr, // hull
				nullptr, // geometry
				m_device.CreatePixelShader(psBytes_rwuvs),
				m_device.CreateInputLayout<Vertex_PoUVs>(vsBytes_rwuvs)
			}
		));
		// RegularWithTesselation
		const auto vsBytes_rwt = Device::LoadByteCode(L"vs_rwt.cso");
		const auto hsBytes_rwt = Device::LoadByteCode(L"hs_rwt.cso");
		const auto dsBytes_rwt = Device::LoadByteCode(L"ds_rwt.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithTesselation, Shaders{
				m_device.CreateVertexShader(vsBytes_rwt),
				m_device.CreateHullShader(hsBytes_rwt),
				m_device.CreateDomainShader(dsBytes_rwt),
				nullptr, // geometry
				m_device.CreatePixelShader(psBytes_r),
				m_device.CreateInputLayout<Vertex_Po>(vsBytes_rwt)
			}
		));
		// RegularWithTesselationBSpline
		const auto dsBytes_rwtbs = Device::LoadByteCode(L"ds_rwtbs.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithTesselationBSpline , Shaders{
				m_device.CreateVertexShader(vsBytes_rwt),
				m_device.CreateHullShader(hsBytes_rwt),
				m_device.CreateDomainShader(dsBytes_rwtbs),
				nullptr, // geometry
				m_device.CreatePixelShader(psBytes_r),
				m_device.CreateInputLayout<Vertex_Po>(vsBytes_rwt)
			}
		));
		// RegularWithTesselationCISpline
		const auto vsBytes_rwtcis = Device::LoadByteCode(L"vs_rwtcis.cso");
		const auto hsBytes_rwtcis = Device::LoadByteCode(L"hs_rwtcis.cso");
		const auto dsBytes_rwtcis = Device::LoadByteCode(L"ds_rwtcis.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithTesselationCISpline, Shaders{
				m_device.CreateVertexShader(vsBytes_rwtcis),
				m_device.CreateHullShader(hsBytes_rwtcis),
				m_device.CreateDomainShader(dsBytes_rwtcis),
				nullptr, // geometry
				m_device.CreatePixelShader(psBytes_r),
				m_device.CreateInputLayout<Vertex_PoCoef>(vsBytes_rwtcis)
			}
		));
		// RegularWithTesselationSurface
		const auto hsBytes_rwtsr = Device::LoadByteCode(L"hs_rwtsr.cso");
		const auto dsBytes_rwtsr = Device::LoadByteCode(L"ds_rwtsr.cso");
		const auto gsBytes_rwtsr = Device::LoadByteCode(L"gs_rwtsr.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithTesselationSurface, Shaders{
				m_device.CreateVertexShader(vsBytes_rwt),
				m_device.CreateHullShader(hsBytes_rwtsr),
				m_device.CreateDomainShader(dsBytes_rwtsr),
				m_device.CreateGeometryShader(gsBytes_rwtsr),
				m_device.CreatePixelShader(psBytes_rwuvs),
				m_device.CreateInputLayout<Vertex_Po>(vsBytes_rwt)
			}
		));
		// RegularWithTesselationBSurface
		const auto dsBytes_rwtbsr = Device::LoadByteCode(L"ds_rwtbsr.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithTesselationBSurface, Shaders{
				m_device.CreateVertexShader(vsBytes_rwt),
				m_device.CreateHullShader(hsBytes_rwtsr),
				m_device.CreateDomainShader(dsBytes_rwtbsr),
				m_device.CreateGeometryShader(gsBytes_rwtsr),
				m_device.CreatePixelShader(psBytes_rwuvs),
				m_device.CreateInputLayout<Vertex_Po>(vsBytes_rwt)
			}
		));
		// RegularWithTesselationGregory
		const auto hsBytes_rwtg = Device::LoadByteCode(L"hs_rwtg.cso");
		const auto dsBytes_rwtg = Device::LoadByteCode(L"ds_rwtg.cso");
		const auto gsBytes_rwtg = Device::LoadByteCode(L"gs_rwtg.cso");
		m_shaders.insert(std::make_pair(ShaderType::RegularWithTesselationGregory, Shaders{
				m_device.CreateVertexShader(vsBytes_rwt),
				m_device.CreateHullShader(hsBytes_rwtg),
				m_device.CreateDomainShader(dsBytes_rwtg),
				m_device.CreateGeometryShader(gsBytes_rwtg),
				m_device.CreatePixelShader(psBytes_r),
				m_device.CreateInputLayout<Vertex_Po>(vsBytes_rwt)
			}
		));
	}

	// CONSTANT BUFFERS
	{
		ID3D11Buffer* vsb[] = { m_constBuffModel.get(),  m_constBuffView.get(), m_constBuffProj.get() };
		m_device.deviceContext()->VSSetConstantBuffers(0, 3, vsb);
		ID3D11Buffer* hsb[] = { m_constBuffView.get(), m_constBuffProj.get(), m_constBuffTessConst.get() };
		m_device.deviceContext()->HSSetConstantBuffers(0, 3, hsb);
		ID3D11Buffer* dsb[] = { m_constBuffView.get(), m_constBuffProj.get() };
		m_device.deviceContext()->DSSetConstantBuffers(0, 2, dsb);
		ID3D11Buffer* psb[] = { m_constBuffColor.get(), m_constBuffTrimInfo.get() };
		m_device.deviceContext()->PSSetConstantBuffers(0, 2, psb);
	}
	// STATES
	RasterizerDescription rsdesc;
	m_rastState = m_device.CreateRasterizerState(rsdesc);
	m_device.deviceContext()->RSSetState(m_rastState.get());

	m_blendState = m_device.CreateBlendState(BlendDescription::AdditiveBlendDescription());

	DepthStencilDescription dsdesc;
	m_dssWrite = m_device.CreateDepthStencilState(dsdesc);
	dsdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	m_dssNoWrite = m_device.CreateDepthStencilState(dsdesc);

	rsdesc.FillMode = D3D11_FILL_WIREFRAME;
	m_rastStateWireframe = m_device.CreateRasterizerState(rsdesc);

	m_sampState = m_device.CreateSamplerState(SamplerDescription());
	auto s_ptr = m_sampState.get();
	m_device.deviceContext()->PSSetSamplers(0, 1, &s_ptr);

	BindTrimTextures();

	// GLOBAL
	m_axesModel->Initialize(m_device);
	m_cubeModel->Initialize(m_device);
	m_pointModel->Initialize(m_device);

	m_axes.SetModel(m_axesModel.get());
	m_UI->cursor.SetModel(m_axesModel.get());
	m_UI->cursor.transform.SetScaling(0.5, 0.5, 0.5);

	m_UI->selection.color = { 0.0f, 0.0f, 1.0f, 1.0f };
	m_UI->selection.SetModel(m_pointModel.get());
}

void Application::BindTrimTextures() {
	ID3D11ShaderResourceView* psr[] = { m_UI->intersection.uv1TrimTexSRV.get(), m_UI->intersection.uv2TrimTexSRV.get() };
	m_device.deviceContext()->PSSetShaderResources(0, 2, psr);
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
	if (m_wndSizeChanged || m_UI->stereoscopicChanged || m_firstPass) {
		ResizeWnd();
		if (!m_UI->stereoscopicView) {
			DirectX::XMFLOAT4X4 projMtx = matrix4_to_XMFLOAT4X4(projMatrix());
			m_device.UpdateBuffer(m_constBuffProj, projMtx);
		}
	}

	if (m_camera.cameraChanged || m_UI->stereoscopicChanged || m_firstPass) {
		m_camera.cameraChanged = false;
		if (!m_UI->stereoscopicView) {
			DirectX::XMFLOAT4X4 viewMtx = matrix4_to_XMFLOAT4X4(m_camera.viewMatrix());
			m_device.UpdateBuffer(m_constBuffView, viewMtx);
		}
	}

	if (m_UI->stereoscopicChanged) {
		m_UI->stereoscopicChanged = false;
	}

	if (!m_UI->showCAD) {
		auto& milling = m_UI->milling;
		if (milling.resetHeightMap) {
			milling.ResetHeightMap(m_device);
			// bind textures
			// m_UI->milling.heightMapTexSRV
		}
		if (milling.updateHeightMap) {
			milling.UpdateHeightMap(m_device);
		}
		if (milling.sceneChanged) {
			milling.UpdateMesh(m_device);
		}
	}
}

float Application::aspect() const {
	return static_cast<float>(m_winWidth) / static_cast<float>(m_winHeight);
}

gmod::matrix4<float> Application::projMatrix() const {
	const float& n = m_near;
	const float& f = m_far;

	const float h = 1 / std::tan(0.5f * m_FOV);
	const float l = h / aspect();
	const float d = f - n;

	return gmod::matrix4<float>(
		l, 0,  0,           0,
		0, h,  0,           0,
		0, 0, (f + n) / d, (2 * f * n) / d,
		0, 0, -1,	        0
	);
}

gmod::matrix4<float> Application::projMatrix_inv() const {
	const float& n = m_near;
	const float& f = m_far;

	const float h_1 = std::tan(0.5f * m_FOV);
	const float w_1 = h_1 * aspect();
	const float d = 2 * f * n;

	return gmod::matrix4<float>(
		w_1, 0,	  0,		   0,
		0,   h_1, 0,		   0,
		0,   0,	  0,		  -1,
		0,   0,  (f - n) / d, (f + n) / d
	);
}

gmod::matrix4<float> Application::stereoProjMatrix(int sign) const {
	const float& n = m_near;
	const float& f = m_far;

	const float d = sign * m_UI->stereoD / 2;
	const float shift = (d * n) / m_UI->stereoF;

	const float t = n * std::tan(0.5f * m_FOV);
	const float b = -t;

	const float width_2 = t * aspect();

	const float l = -width_2 + shift;
	const float r = width_2 + shift;
	

	const float A = (2 * n) / (r - l);
	const float F = (2 * n) / (t - b);
	const float C = (r + l) / (r - l);
	const float G = (t + b) / (t - b);
	const float K = (f + n) / (f - n);
	const float L = (2 * f * n) / (f - n);

	return gmod::matrix4<float>(
		A, 0,  C, 0,
		0, F,  G, 0,
		0, 0,  K, L,
		0, 0, -1, 0
	);
}

void Application::RenderUI() {
	m_device.deviceContext()->ClearRenderTargetView(m_backBuffer.get(), reinterpret_cast<float*>(&m_UI->bkgdColor));
	m_device.deviceContext()->ClearDepthStencilView(m_depthBuffer.get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	m_UI->Render(m_firstPass, m_camera);
	ImGui::Render();
}

void Application::Render() {
	if (m_UI->showCAD) {
		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(m_UI->cursor.transform.modelMatrix()));
		m_UI->cursor.RenderMesh(m_device.deviceContext(), m_shaders);

		if (!m_UI->selection.Empty()) {
			m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(m_UI->selection.modelMatrix()));
			m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(m_UI->selection.color.data()));
			m_UI->selection.RenderMesh(m_device.deviceContext(), m_shaders);
		}
	}

	if (m_UI->showAxes) {
		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(m_axes.modelMatrix(m_camera, m_far, m_FOV)));
		m_axes.RenderMesh(m_device.deviceContext(), m_shaders);
	}

	if (m_UI->stereoscopicView) {
		m_device.deviceContext()->OMSetBlendState(m_blendState.get(), nullptr, UINT_MAX);
		m_device.deviceContext()->OMSetDepthStencilState(m_dssNoWrite.get(), 0);
		RenderStereoscopic(-1, m_UI->stereoRed);
		RenderStereoscopic(+1, m_UI->stereoCyan);
		m_device.deviceContext()->OMSetDepthStencilState(nullptr, 0);
		m_device.deviceContext()->OMSetBlendState(nullptr, nullptr, UINT_MAX);
		return;
	}

	for (auto& obj : m_UI->sceneObjects) {
		if (m_UI->hideControlPoints && typeid(Point) == typeid(*obj.get())) { continue; }

		auto opt = obj->GetSubObjects();
		if (opt.has_value()) {
			for (auto& subObj : *opt.value()) {
				if (m_UI->hideControlPoints && typeid(Point) == typeid(*subObj.get())) { continue; }
				m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(subObj->modelMatrix()));
				m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(subObj->color.data()));
				subObj->RenderMesh(m_device.deviceContext(), m_shaders);
			}
		}

		auto trimInfo = m_UI->intersection.GetTrimInfo(obj->id);
		TrimmingInfo ti;
		if (trimInfo.first == -1) {
			ti = { 0, 0, 0, 0 };
		} else {
			ti = { 1, trimInfo.first, trimInfo.second, 0 };
		}
		m_device.UpdateBuffer(m_constBuffTrimInfo, ti);

		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(obj->modelMatrix()));
		// TODO : to improve performance, change internals of Contains
		if (m_UI->selection.Contains(obj->id)) {
			m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(reinterpret_cast<float*>(&m_UI->slctdColor)));
		} else {
			m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(obj->color.data()));
		}
		if (obj->geometryChanged || m_firstPass) {
			obj->UpdateMesh(m_device);
		}
		auto div = dynamic_cast<Divisable*>(obj.get());
		if (nullptr != div) {
			unsigned int divisions = div->GetDivisions();
			auto [uPatches, vPatches] = div->GetUVPatches();
			m_device.UpdateBuffer(m_constBuffTessConst, TessellationConstants{ divisions, uPatches, vPatches, 0 });
		}
		obj->RenderMesh(m_device.deviceContext(), m_shaders);
	}

	if (m_UI->intersection.availible) {
		if (m_UI->updatePreview) {
			m_UI->updatePreview = false;
			m_UI->intersection.UpdateMesh(m_device);
			m_UI->intersection.UpdateUVPlanes(m_device);
			BindTrimTextures();
		}
		if (m_UI->intersection.reupload) {
			m_UI->intersection.ReUploadUVPlanes(m_device);
			BindTrimTextures();
		}

		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(gmod::matrix4<float>::identity()));
		m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(m_UI->intersection.color.data()));
		m_UI->intersection.RenderMesh(m_device.deviceContext(), m_shaders);
	}

	if (!m_UI->showCAD) {
		auto& milling = m_UI->milling;
		if (milling.cutter.propertiesChanged) {
			milling.cutter.UpdateMesh(m_device);
		}
		if (milling.cutter.showCutter) {
			m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(milling.cutter.modelMatrix()));
			m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(milling.cutter.color.data()));
			milling.cutter.RenderMesh(m_device.deviceContext(), m_shaders);
		}

		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(milling.modelMatrix()));
		m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(milling.color.data()));
		milling.RenderMesh(m_device.deviceContext(), m_shaders);

		auto& animator = m_UI->animator;
		// complete animation button
		if (m_UI->completeAnimation) {
			m_UI->completeAnimation = false;
			animator.CompleteAnimation(m_device);
		}
		// make animation step
		if (animator.isRunning) {
			animator.MakeStep(m_device);
		}
		// display path
		if (animator.displayPath && animator.canRender) {
			m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(animator.modelMatrix()));
			m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(animator.pathColor.data()));
			animator.RenderMesh(m_device.deviceContext(), m_shaders);
		}
	}
}

void app::Application::RenderStereoscopic(int sign, ImVec4& color) {
	m_device.UpdateBuffer(m_constBuffColor, DirectX::XMFLOAT4(reinterpret_cast<float*>(&color)));

	DirectX::XMFLOAT4X4 viewMtx = matrix4_to_XMFLOAT4X4(m_camera.stereoViewMatrix(sign, m_UI->stereoD));
	m_device.UpdateBuffer(m_constBuffView, viewMtx);

	DirectX::XMFLOAT4X4 projMtx = matrix4_to_XMFLOAT4X4(stereoProjMatrix(sign));
	m_device.UpdateBuffer(m_constBuffProj, projMtx);

	for (auto& obj : m_UI->sceneObjects) {
		if (m_UI->hideControlPoints && typeid(Point) == typeid(*obj.get())) { continue; }

		auto opt = obj->GetSubObjects();
		if (opt.has_value()) {
			for (auto& subObj : *opt.value()) {
				if (m_UI->hideControlPoints && typeid(Point) == typeid(*subObj.get())) { continue; }
				m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(subObj->modelMatrix()));
				subObj->RenderMesh(m_device.deviceContext(), m_shaders);
			}
		}

		m_device.UpdateBuffer(m_constBuffModel, matrix4_to_XMFLOAT4X4(obj->modelMatrix()));
		if (obj->geometryChanged || m_firstPass) {
			obj->UpdateMesh(m_device);
		}
		auto div = dynamic_cast<Divisable*>(obj.get());
		if (nullptr != div) {
			unsigned int divisions = div->GetDivisions();
			auto [uPatches, vPatches] = div->GetUVPatches();
			m_device.UpdateBuffer(m_constBuffTessConst, TessellationConstants{ divisions, uPatches, vPatches, 0 });
		}
		obj->RenderMesh(m_device.deviceContext(), m_shaders);

	}
}

void Application::HandleTransformsOnMouseMove(LPARAM lParam) {
	if (!m_mouse.isLMBDown_flag || m_UI->currentMode == UI::Mode::Neutral) { return; }
	float dx = static_cast<float>(Mouse::GetXPos(lParam) - m_mouse.prevCursorPos.x);
	float dy = static_cast<float>(Mouse::GetYPos(lParam) - m_mouse.prevCursorPos.y);
	gmod::vector3<float> trans;
	if (m_UI->currentMode == UI::Mode::Rotate) {
		std::swap(dx, dy);
		dx = -dx;
	}
	switch (m_UI->currentAxis) {
		case UI::Axis::X: {
			trans = { dx, 0.0f, 0.0f };
			break;
		}
		case UI::Axis::Y: {
			trans = { 0.0f, dx, 0.0f };
			break;
		}
		case UI::Axis::Z: {
			trans = { 0.0f, 0.0f, dx };
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

	auto applyTransformation = [&](auto transformFunc) -> void {
		std::optional<Object*> selectedObj = m_UI->selection.Single();
		if (selectedObj.has_value()) {
			transformFunc(*selectedObj.value());
			selectedObj.value()->InformParents();
		} else {
			transformFunc(m_UI->selection);
		}
	};

	switch (m_UI->currentMode) {
		case UI::Mode::Translate: {
			trans = trans * traSensitivity;
			if (m_UI->selection.Empty()) {
				m_UI->cursor.transform.UpdateTranslation(trans.x(), trans.y(), trans.z());
			} else {
				applyTransformation([&](auto& target) {
					target.UpdateTranslation(trans.x(), trans.y(), trans.z());
				});
			}
			break;
		}
		case UI::Mode::Rotate: {
			if (m_UI->selection.Empty()) { return; }
			trans = trans * rotSensitivity;
			switch (m_UI->currentOrientation) {
				case UI::Orientation::World: {
					applyTransformation([&](auto& target) {
						target.UpdateRotation_Quaternion(trans.x(), trans.y(), trans.z());
					});
					break;
				}
				case UI::Orientation::Cursor: {
					applyTransformation([&](auto& target) {
						target.UpdateRotationAroundPoint_Quaternion(trans.x(), trans.y(), trans.z(), m_UI->cursor.transform.position());
					});
					break;
				}
			}
			break;
		}
		case UI::Mode::Scale: {
			if (m_UI->selection.Empty()) { return; }
			trans = trans * scaSensitivity;
			switch (m_UI->currentOrientation) {
				case UI::Orientation::World: {
					applyTransformation([&](auto& target) {
						target.UpdateScaling(trans.x(), trans.y(), trans.z());
					});
					break;
				}
				case UI::Orientation::Cursor: {
					applyTransformation([&](auto& target) {
						target.UpdateScalingAroundPoint(trans.x(), trans.y(), trans.z(), m_UI->cursor.transform.position());
					});
					break;
				}
			}
			break;
		}
		default: return;
	}
	m_UI->selection.UpdateMidpoint();
}

void Application::HandleCameraOnMouseMove(LPARAM lParam) {
	if (!m_mouse.isMMBDown_flag && !m_mouse.isRMBDown_flag) { return; }

	float dx = static_cast<float>(Mouse::GetXPos(lParam) - m_mouse.prevCursorPos.x);
	float dy = static_cast<float>(Mouse::GetYPos(lParam) - m_mouse.prevCursorPos.y);

	bool MMB = m_UI->useMMB && m_mouse.isMMBDown_flag;
	bool RMB = !m_UI->useMMB && m_mouse.isRMBDown_flag;
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
				m_camera.Zoom(-dy * 10.0f);
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
	if (!m_UI->useMMB) { return; }
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
			m_UI->SelectObjectOnMouseClick(HandleSelectionOnMouseClick(msg.lParam));
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
	auto getClosest = [&origin, &direction, &closestPoint](std::unique_ptr<Object>& obj) -> bool {
		const gmod::vector3<double> vecToPoint = obj->position() - origin;
		const gmod::vector3<double> crossProd = gmod::cross(vecToPoint, direction);

		if (crossProd.length() < selectionRadius) {
			closestPoint = obj.get();
			return true;
		}
		return false;
	};

	for (auto& obj : m_UI->sceneObjects) {
		auto opt = obj->GetSubObjects();
		if (opt.has_value()) {
			for (auto& subObj : *opt.value()) {
				if (getClosest(subObj)) {
					break;
				}
			}
		}
		if (nullptr != closestPoint) { break; }
		if (typeid(Point) != typeid(*obj.get())) { continue; }
		if (getClosest(obj)) {
			break;
		}
	}

	return closestPoint;
}

