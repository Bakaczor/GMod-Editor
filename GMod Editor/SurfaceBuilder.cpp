#include "SurfaceBuilder.h"

using namespace app;

SurfaceBuilder::SurfaceBuilder() : shouldBuild(false), m_type(SurfaceType::Flat), m_isC2(false),
m_a(1.0f), m_b(1.0f), m_aPatch(2), m_bPatch(2), m_divisions(m_minDivisions) {}

void SurfaceBuilder::Reset() {
    m_type = SurfaceType::Flat;
    m_isC2 = false;                 
    m_a = 1.0f;                      
    m_b = 1.0f;                     
    m_aPatch = 2;                   
    m_bPatch = 2;                    
    m_divisions = m_minDivisions;     
    shouldBuild = false;           
}

bool SurfaceBuilder::RenderProperties() {
    bool shouldClose = false;
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 0), ImGuiCond_Appearing);

    if (ImGui::BeginPopupModal("Surface Builder", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize)) {
        ImGui::Text("Surface type:");
        ImGui::BeginGroup();
        ImGui::RadioButton("Flat", reinterpret_cast<int*>(&m_type), static_cast<int>(SurfaceType::Flat));
        ImGui::SameLine();
        ImGui::RadioButton("Cylindric", reinterpret_cast<int*>(&m_type), static_cast<int>(SurfaceType::Cylindric));
        ImGui::EndGroup();

        ImGui::Checkbox("C2 continuity", &m_isC2);
        ImGui::Separator();
        ImGui::Text("Size:");

        std::string firstDim;
        switch (m_type) {
            case SurfaceType::Flat: {
                firstDim = "Width";
                break;
            }
            case SurfaceType::Cylindric: {
                firstDim = "Radius";
                break;
            }
        }
        float a = m_a;
        ImGui::InputFloat(firstDim.c_str(), &a, 0.01f, 0.1f, "%.2f");
        m_a = std::max(0.0f, a);

        float b = m_b;
        ImGui::InputFloat("Length", &b, 0.01f, 0.1f, "%.2f");
        m_b = std::max(0.0f, b);

        ImGui::Separator();
        ImGui::Text("Number of patches:");

        int aPatch = m_aPatch;
        ImGui::InputInt("First count", &aPatch, 1, 2);
        m_aPatch = std::max(1, aPatch);

        int bPatch = m_bPatch;
        ImGui::InputInt("Second count", &bPatch, 1, 2);
        m_bPatch = std::max(1, bPatch);

        int divisions = m_divisions;
        ImGui::InputInt("Divisions", &divisions, 1, static_cast<int>(m_minDivisions));
        m_divisions = std::max(static_cast<int>(m_minDivisions), divisions);

        ImGui::Separator();
        ImGuiStyle& style = ImGui::GetStyle();
        float width = (300 - style.ItemSpacing.x - 2 * style.WindowPadding.x) / 2;
        if (ImGui::Button("Create", ImVec2(width, 0))) {
            shouldBuild = true;
            shouldClose = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(width, 0))) {
            shouldBuild = false;
            shouldClose = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return shouldClose;
}

Surface* SurfaceBuilder::Build() const {
	if (m_isC2) {
        // CHANGE TO BSURFACE
		return new Surface(m_type, m_a, m_b, m_aPatch, m_bPatch, m_divisions);
	} else {
		return new Surface(m_type, m_a, m_b, m_aPatch, m_bPatch, m_divisions);
	}
}
