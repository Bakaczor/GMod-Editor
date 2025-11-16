#include "PathAnimator.h"
#include <numeric>
#include <thread>

using namespace app;

PathAnimator::PathAnimator(PathParser& path, Milling& milling) : m_path(path), m_milling(milling) {}

void PathAnimator::StartAnimation() {
	isRunning = true;
}

void PathAnimator::StopAnimation() {
	isRunning = false;
	completeAnimation = false;
}

void PathAnimator::ClearMesh() {
	m_currPos = gmod::vector3<float>(0, 0, 0);
	m_points.clear();
	m_polylineMesh.Release();
	canRender = false;
}

void PathAnimator::RestartAnimation() {
	m_firstStep = true;
	errorDetected = false;
	m_path.ResetStepIterator();
	m_path.ResetCommandIterator();

	StopAnimation();
}

void PathAnimator::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Regular).Set(context);
	m_polylineMesh.Render(context);
}

void PathAnimator::CompleteAnimationAsync(const Device& device) {
	if (!isRunning) {
		std::thread([this, &device]() {
			CompleteAnimation(device);
			m_milling.UpdateHeightMap(device);
		}).detach(); 
	}
}

void PathAnimator::CompleteAnimation(const Device& device) {
	if (m_path.Empty()) { 
		RestartAnimation();
		return; 
	}
	isRunning = true;

	std::optional<PathParser::MillingCommand> cmd;
	while ((cmd = m_path.GetNextCommand()).has_value()) {
		auto& coords = cmd.value().coordinates;
		if (m_firstStep) {
			m_firstStep = false;
			m_currPos = coords;
			m_points.push_back(DirectX::XMFLOAT3(m_currPos.x(), m_currPos.y(), m_currPos.z()));
			continue;
		}

		// verify and mill
		auto veridct = m_milling.Mill(m_currPos, coords, false);
		if (veridct.has_value()) {
			errorMsg = veridct.value() + " [N" + std::to_string(cmd.value().commandNumber) + "]";
			errorDetected = true;
			StopAnimation();
			return;
		}
		m_currPos = coords;
		m_points.push_back(DirectX::XMFLOAT3(m_currPos.x(), m_currPos.y(), m_currPos.z()));
	}
	auto& last = m_points.back();
	m_milling.cutter.MoveTo(gmod::vector3<float>(last.x, last.y, last.z));

	UpdateMesh(device);
	RestartAnimation();
}

bool PathAnimator::MakeStep(const Device& device, float deltaTime) {
	auto step = m_path.GetNextStep(deltaTime * speed);
	int it = 0;
	// first command
	if (m_firstStep) {
		m_firstStep = false;
		if (!step.has_value()) {
			RestartAnimation();
			return false;
		}
		auto& destinations = step.value().destinations;
		m_currPos = destinations.front();
		m_milling.cutter.MoveTo(m_currPos);
		m_points.push_back(DirectX::XMFLOAT3(m_currPos.x(), m_currPos.y(), m_currPos.z()));
		it += 1;
		if (destinations.size() == 1) {
			return true;
		}
	}

	// end of path
	if (!step.has_value()) {
		RestartAnimation();
		return false;
	}

	auto& destinations = step.value().destinations;
	for (it; it < destinations.size(); ++it) {
		auto& currDest = destinations[it];

		// verify and mill
		auto veridct = m_milling.Mill(m_currPos, currDest);
		if (veridct.has_value()) {
			errorMsg = veridct.value() + " [N" + std::to_string(step.value().associatedCommandNumbers[it]) + "]";
			errorDetected = true;
			StopAnimation();
			return false;
		}
		m_currPos = currDest;
		m_milling.cutter.MoveTo(m_currPos);
		m_points.push_back(DirectX::XMFLOAT3(m_currPos.x(), m_currPos.y(), m_currPos.z()));
	}
	UpdateMesh(device);
}

void PathAnimator::UpdateMesh(const Device& device) {
	if (m_points.size() > 0) {
		std::vector<Vertex_Po> verts;
		verts.reserve(m_points.size());
		std::transform(m_points.begin(), m_points.end(), std::back_inserter(verts), [](const auto& point) {
			return Vertex_Po{ point };
		});

		std::vector<USHORT> idxs(m_points.size());
		std::iota(idxs.begin(), idxs.end(), 0);

		m_polylineMesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
		canRender = true;
	}
}
