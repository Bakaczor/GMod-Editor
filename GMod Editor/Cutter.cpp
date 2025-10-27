#include "framework.h"
#include <imgui.h>
#include "Cutter.h"

using namespace app;

const std::vector<CutterType> Cutter::m_cutterTypes = { CutterType::Spherical, CutterType::Cylindrical };
const std::vector<const char*> Cutter::m_cutterTypeNames = { "Spherical", "Cylindrical" };

Cutter::Cutter() {
	propertiesChanged = true;
}

void Cutter::RenderMesh(const mini::dx_ptr<ID3D11DeviceContext>& context, const std::unordered_map<ShaderType, Shaders>& map) const {
	map.at(ShaderType::Cutter).Set(context);
	m_mesh.Render(context);
}

void Cutter::UpdateMesh(const Device& device) {
    std::vector<Vertex_PoNo> verts;
    std::vector<USHORT> idxs;

    if (m_cutterType == CutterType::Spherical) {
        GenerateSpherical(verts, idxs);
    } else {
        GenerateCylindrical(verts, idxs);
    }

    m_mesh.Update(device, verts, idxs, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    propertiesChanged = false;
}

void Cutter::GenerateSpherical(std::vector<Vertex_PoNo>& verts, std::vector<USHORT>& idxs) {
    verts.reserve(2 * m_parts + 1 + (m_parts - 2) * m_parts + 1); // cylinder + half-sphere
    idxs.reserve(3 * ((2 + 1) * m_parts + 2 * (m_parts - 2) * m_parts + m_parts)); // cylinder + half-sphere

    const double horizontalStep = 2 * DirectX::XM_PI / m_parts;
    const double verticalStep = DirectX::XM_PIDIV2 / (m_parts - 1);
    const float radius = m_millingPartDiameter / 2.0f;

    verts.push_back(Vertex_PoNo{ DirectX::XMFLOAT3(0, m_totalCutterLength, 0), DirectX::XMFLOAT3(0, 1, 0) }); // top

    std::vector<std::pair<float, float>> cossin(m_parts);
    for (int i = 0; i < m_parts; ++i) {
        const double angle = i * horizontalStep;
        const float cosA = static_cast<float>(std::cos(angle));
        const float sinA = static_cast<float>(std::sin(angle));
        cossin[i] = std::make_pair(cosA, sinA);

        DirectX::XMFLOAT3 normal(cosA, 0, sinA);

        // top vertex
        Vertex_PoNo topVertex{
            DirectX::XMFLOAT3(
                radius * cosA,
                m_totalCutterLength,
                radius * sinA
            ), normal
        };
        verts.push_back(topVertex);
    }

    for (int i = 0; i < m_parts; ++i) {
        const float& cosA = cossin[i].first;
        const float& sinA = cossin[i].second;

        DirectX::XMFLOAT3 normal(cosA, 0, sinA);

        // bottom vertex
        Vertex_PoNo bottomVertex{
            DirectX::XMFLOAT3(
                radius * cosA,
                m_totalCutterLength - radius,
                radius * sinA
            ), normal
        };
        verts.push_back(bottomVertex);
    }

    // indices for sides and cap
    for (int i = 1; i <= m_parts; ++i) {
        int next_i = (i + 1) % (m_parts + 1);
        if (next_i == 0) { next_i++; }

        // first triangle
        idxs.push_back(static_cast<USHORT>(i));
        idxs.push_back(static_cast<USHORT>(next_i + m_parts));
        idxs.push_back(static_cast<USHORT>(i + m_parts));

        // second triangle
        idxs.push_back(static_cast<USHORT>(i));
        idxs.push_back(static_cast<USHORT>(next_i));
        idxs.push_back(static_cast<USHORT>(next_i + m_parts));

        // cap
        idxs.push_back(static_cast<USHORT>(0));
        idxs.push_back(static_cast<USHORT>(next_i));
        idxs.push_back(static_cast<USHORT>(i));
    }

    // half-sphere at the end
    for (int j = 1; j < m_parts - 1; ++j) {
        const double v = j * verticalStep + DirectX::XM_PIDIV2;
        const float cosV = static_cast<float>(std::cos(v));
        const float sinV = static_cast<float>(std::sin(v));

        for (int i = 0; i < m_parts; ++i) {
            const double u = i * horizontalStep;
            const float cosU = static_cast<float>(std::cos(u));
            const float sinU = static_cast<float>(std::sin(u));

            DirectX::XMFLOAT3 normal(cosU * sinV, cosV, sinU * sinV);

            Vertex_PoNo vertex{
                DirectX::XMFLOAT3(
                    radius * cosU * sinV,
                    radius + radius * cosV,
                    radius * sinU * sinV
                ), normal
            };
            verts.push_back(vertex);
        }
    }

    verts.push_back(Vertex_PoNo{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0, -1, 0) }); // bottom

    int startIdx = 1 + m_parts;
    for (int j = 0; j < m_parts - 2; ++j) {
        for (int i = 0; i < m_parts; ++i) {
            USHORT v1 = startIdx + j * m_parts + i;
            USHORT v2 = startIdx + j * m_parts + (i + 1) % m_parts;
            USHORT v3 = startIdx + (j + 1) * m_parts + i;
            USHORT v4 = startIdx + (j + 1) * m_parts + (i + 1) % m_parts;

            idxs.push_back(v1);
            idxs.push_back(v4);
            idxs.push_back(v3);

            idxs.push_back(v1);
            idxs.push_back(v2);
            idxs.push_back(v4);
        }
    }

    // for j = m_parts - 2
    USHORT vBottom = verts.size() - 1;
    for (int i = 0; i < m_parts; ++i) {
        USHORT v1 = startIdx + (m_parts - 2) * m_parts + i;
        USHORT v2 = startIdx + (m_parts - 2) * m_parts + (i + 1) % m_parts;

        idxs.push_back(v1);
        idxs.push_back(v2);
        idxs.push_back(vBottom);
    }
}

void Cutter::GenerateCylindrical(std::vector<Vertex_PoNo>& verts, std::vector<USHORT>& idxs) const {
    verts.reserve(2 * m_parts + 2); // there are 2 caps + 2 center points 
    idxs.reserve(3 * (2 * m_parts + 2 * m_parts)); // there are 2 caps + 2 triangles for each side

    const double horizontalStep = 2 * DirectX::XM_PI / m_parts;
    const float radius = m_millingPartDiameter / 2.f;

    for (int i = 0; i < m_parts; ++i) {
        const double angle = i * horizontalStep;
        const float cosA = static_cast<float>(std::cos(angle));
        const float sinA = static_cast<float>(std::sin(angle));

        DirectX::XMFLOAT3 normal(cosA, 0, sinA);

        // bottom vertex
        Vertex_PoNo bottomVertex{
            DirectX::XMFLOAT3(
                radius * cosA,
                0, // base level
                radius * sinA 
            ), normal
        };
        verts.push_back(bottomVertex);

        // top vertex
        Vertex_PoNo topVertex{
            DirectX::XMFLOAT3(
                radius * cosA,
                m_totalCutterLength,
                radius * sinA
            ), normal
        };
        verts.push_back(topVertex);
    }

    // indices for sides
    const int parts2 = 2 * m_parts;
    for (int i = 0; i < m_parts; ++i) {
        int base = 2 * i;
        int base1 = base + 1;
        int base2 = (base + 2) % parts2;
        int base3 = (base + 3) % parts2;

        // first triangle
        idxs.push_back(static_cast<USHORT>(base));
        idxs.push_back(static_cast<USHORT>(base1));
        idxs.push_back(static_cast<USHORT>(base2));

        // second triangle
        idxs.push_back(static_cast<USHORT>(base1));
        idxs.push_back(static_cast<USHORT>(base3));
        idxs.push_back(static_cast<USHORT>(base2));
    }

    // center points
    verts.push_back(Vertex_PoNo{ DirectX::XMFLOAT3(0, 0, 0), DirectX::XMFLOAT3(0, -1, 0) }); // bottom
    verts.push_back(Vertex_PoNo{ DirectX::XMFLOAT3(0, m_totalCutterLength, 0), DirectX::XMFLOAT3(0, 1, 0) }); // top

    int bottomCenterIndex = static_cast<int>(verts.size()) - 2;
    int topCenterIndex = static_cast<int>(verts.size()) - 1;

    // indices for caps
    for (int i = 0; i < m_parts; ++i) {
        int next_i = (i + 1) % m_parts;

        // bottom cap
        idxs.push_back(static_cast<USHORT>(bottomCenterIndex));
        idxs.push_back(static_cast<USHORT>(2 * i));
        idxs.push_back(static_cast<USHORT>(2 * next_i));

        // top cap
        idxs.push_back(static_cast<USHORT>(topCenterIndex));
        idxs.push_back(static_cast<USHORT>(2 * next_i + 1));
        idxs.push_back(static_cast<USHORT>(2 * i + 1));
    }
}

gmod::matrix4<float> Cutter::modelMatrix() {
    auto MM = m_transform.modelMatrix();
    if (!m_useCutterBase) {
        auto oldPos = m_transform.position();
        auto newPos = oldPos + gmod::vector3<float>(0, -m_millingPartHeight / 2.f, 0);
        m_transform.SetTranslation(newPos.x(), newPos.y(), newPos.z());
        MM = m_transform.modelMatrix();
        m_transform.SetTranslation(oldPos.x(), oldPos.y(), oldPos.z());
    }
	return MM;
}

float Cutter::GetRadius() const {
	return m_millingPartDiameter / 2.f;
}

float Cutter::GetCuttingHeight() const {
	return m_millingPartHeight;
}

float Cutter::GetTotalLength() const {
	return m_totalCutterLength;
}

float Cutter::GetMaxAngle() const {
	return m_maxHorizontalDeviationAngle;
}

bool Cutter::isBaseOriented() const {
	return m_useCutterBase;
}

gmod::vector3<float> Cutter::GetPosition() const {
	return m_transform.position();
}

void Cutter::SetCutterDiameter(int diameter) {
	m_millingPartDiameter = static_cast<float>(diameter);
	propertiesChanged = true;
    if (m_cutterType == CutterType::Spherical) {
        m_millingPartHeight = m_millingPartDiameter;
    }
    if (m_millingPartDiameter > m_totalCutterLength) {
        m_totalCutterLength = 2 * m_millingPartDiameter;
    }
}

void Cutter::SetCutterType(CutterType type) {
	m_cutterType = type;
	m_selectedCutterType = m_cutterType == CutterType::Spherical ? 0 : 1;
	propertiesChanged = true;
}

void Cutter::MoveTo(gmod::vector3<float> pos) {
    m_transform.SetTranslation(pos.x(), pos.y(), pos.z());
}

void Cutter::RenderProperties() {
	ImGui::Spacing();
	ImGui::SeparatorText("Cutter settings");
	ImGui::Spacing();

    auto checkChange = [this](float& oldValue, float& currentValue)-> void {
        if (std::abs(oldValue - currentValue) > 100 * std::numeric_limits<float>::epsilon()) {
            propertiesChanged = true;
        }
    };
    float oldValue;

	ImGui::Text("Cutter type: ");
    int oldType = m_selectedCutterType;
	if (ImGui::Combo("##cutter_type", &m_selectedCutterType, m_cutterTypeNames.data(), m_cutterTypeNames.size())) {
		m_cutterType = m_cutterTypes[m_selectedCutterType];
	}
    if (oldType != m_selectedCutterType) {
        propertiesChanged = true;
    }

	ImGui::Spacing();

	float inputWidth = 100.f;
	ImGui::Columns(2, "cutter_settings", false);
	ImGui::SetColumnWidth(0, 150.f);

	ImGui::Text("diameter [mm]:"); ImGui::NextColumn();
	ImGui::SetNextItemWidth(inputWidth);
    oldValue = m_millingPartDiameter;
	ImGui::InputFloat("##diameter", &m_millingPartDiameter, 0.01f, 1.f);
    checkChange(oldValue, m_millingPartDiameter);
    if (m_millingPartDiameter > m_totalCutterLength) {
        m_totalCutterLength = m_millingPartDiameter;
    }
	ImGui::NextColumn();

	if (m_cutterType == CutterType::Cylindrical) {
		ImGui::Text("height [mm]:");  ImGui::NextColumn();
		ImGui::SetNextItemWidth(inputWidth);
        oldValue = m_millingPartHeight;
		ImGui::InputFloat("##height", &m_millingPartHeight, 0.01f, 1.f);
        checkChange(oldValue, m_millingPartHeight);
		ImGui::NextColumn();
    } else {
        m_millingPartHeight = m_millingPartDiameter;
    }

	ImGui::Text("total length [mm]:");  ImGui::NextColumn();
	ImGui::SetNextItemWidth(inputWidth);
    oldValue = m_totalCutterLength;
	ImGui::InputFloat("##total_length", &m_totalCutterLength, 0.01f, 1.f);
    checkChange(oldValue, m_totalCutterLength);
	ImGui::NextColumn();

	ImGui::Text("max angle [deg]:");  ImGui::NextColumn();
	ImGui::SetNextItemWidth(inputWidth);
	ImGui::InputFloat("##max_angle", &m_maxHorizontalDeviationAngle, 0.1f, 1.f, "%.2f");
	ImGui::NextColumn();

	ImGui::Columns(1);
}

void Cutter::RenderCutterOrientation() {
	ImGui::BeginGroup();
	ImGui::Text("Use cutter:");
	ImGui::SameLine();
	if (ImGui::RadioButton("base", m_useCutterBase)) {
		m_useCutterBase = true;
	}
	ImGui::SameLine();
	if (ImGui::RadioButton("centre", !m_useCutterBase)) {
		m_useCutterBase = false;
	}
	ImGui::EndGroup();
    ImGui::Checkbox("Show cutter", &showCutter);
}
