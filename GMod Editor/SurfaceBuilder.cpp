#include "Application.h"
#include "BSurface.h"
#include "Point.h"
#include "SurfaceBuilder.h"

using namespace app;

SurfaceBuilder::SurfaceBuilder(std::vector<std::unique_ptr<Object>>& sceneObjects) : m_sceneObjects(sceneObjects),
shouldBuild(false), m_type(SurfaceType::Flat), m_isC2(false), m_a(1.0f), m_b(1.0f), m_aPatch(2), m_bPatch(2), m_divisions(Surface::minDivisions) {}

void SurfaceBuilder::Reset() {
    m_type = SurfaceType::Flat;
    m_isC2 = false;                 
    m_a = 1.0f;                      
    m_b = 1.0f;                     
    m_aPatch = 2;                   
    m_bPatch = 2;                    
    m_divisions = Surface::minDivisions;     
    shouldBuild = false;           
}

void SurfaceBuilder::SetC2(bool isC2) {
    m_isC2 = isC2;
}

Surface* SurfaceBuilder::BuildSurface() const {
    unsigned int aPoints, bPoints;
    std::vector<Object*> controlPoints;
    std::vector<Patch> patches;

    if (m_type == SurfaceType::Flat) {
        aPoints = m_aPatch * (Patch::rowSize - 1) + 1;
        bPoints = m_bPatch * (Patch::rowSize - 1) + 1;
        controlPoints.reserve(aPoints * bPoints);

        for (unsigned int i = 0; i < aPoints; ++i) {
            for (unsigned int j = 0; j < bPoints; ++j) {
                auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);
                point->deletable = false;

                float x = (m_a * i) / (aPoints - 1) - m_a / 2.0f;
                float z = (m_b * j) / (bPoints - 1) - m_b / 2.0f;
                point->SetTranslation(x, 0.0f, z);

                controlPoints.push_back(point.get());
                m_sceneObjects.push_back(std::move(point));
            }
        }

        for (unsigned int i = 0; i < m_aPatch; ++i) {
            for (unsigned int j = 0; j < m_bPatch; ++j) {
                std::array<USHORT, Patch::patchSize> indices;

                USHORT step = Patch::rowSize - 1;
                for (USHORT u = 0; u < Patch::rowSize; ++u) {
                    for (USHORT v = 0; v < Patch::rowSize; ++v) {
                        indices[u * Patch::rowSize + v] = (i * step + u) * bPoints + (j * step + v);
                    }
                }
                patches.emplace_back(indices);
            }
        }
    } else {
        aPoints = m_aPatch * (Patch::rowSize - 1);
        bPoints = m_bPatch * (Patch::rowSize - 1) + 1;
        controlPoints.reserve(aPoints * bPoints);

        for (unsigned int i = 0; i < aPoints; ++i) {
            for (unsigned int j = 0; j < bPoints; ++j) {
                auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);
                point->deletable = false;

                float angle = (2 * std::numbers::pi_v<float> *i) / aPoints;
                float x = m_a * cos(angle);
                float y = m_a * sin(angle);
                float z = (m_b * j) / (bPoints - 1) - m_b / 2.0f;
                point->SetTranslation(x, z, y);

                controlPoints.push_back(point.get());
                m_sceneObjects.push_back(std::move(point));
            }
        }

        for (unsigned int i = 0; i < m_aPatch; ++i) {
            for (unsigned int j = 0; j < m_bPatch; ++j) {
                std::array<USHORT, Patch::patchSize> indices;

                USHORT step = Patch::rowSize - 1;
                for (USHORT u = 0; u < Patch::rowSize; ++u) {
                    for (USHORT v = 0; v < Patch::rowSize; ++v) {
                        USHORT wrapped_i = (i * step + u) % aPoints;
                        indices[u * Patch::rowSize + v] = wrapped_i * bPoints + (j * step + v);
                    }
                }
                patches.emplace_back(indices);
            }
        }
    }

    return new Surface(m_type, aPoints, bPoints, m_divisions, controlPoints, patches);
}

Surface* SurfaceBuilder::BuildBSurface() const {
    unsigned int aPoints, bPoints;
    std::vector<Object*> controlPoints;
    std::vector<Patch> patches;

    if (m_type == SurfaceType::Flat) {
        aPoints = m_aPatch + 3;
        bPoints = m_bPatch + 3;
        controlPoints.reserve(aPoints * bPoints);

        for (unsigned int i = 0; i < aPoints; ++i) {
            for (unsigned int j = 0; j < bPoints; ++j) {
                auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);
                point->deletable = false;

                float x = (m_a * i) / (aPoints - 1) - m_a / 2.0f;
                float z = (m_b * j) / (bPoints - 1) - m_b / 2.0f;
                point->SetTranslation(x, 0.0f, z);

                controlPoints.push_back(point.get());
                m_sceneObjects.push_back(std::move(point));
            }
        }

        for (unsigned int i = 0; i < m_aPatch; ++i) {
            for (unsigned int j = 0; j < m_bPatch; ++j) {
                std::array<USHORT, Patch::patchSize> indices;

                for (USHORT u = 0; u < 4; ++u) {
                    for (USHORT v = 0; v < 4; ++v) {
                        indices[u * 4 + v] = (i + u) * bPoints + (j + v);
                    }
                }
                patches.emplace_back(indices);
            }
        }
    } else {
        aPoints = m_aPatch;
        bPoints = m_bPatch + 3;
        controlPoints.reserve(aPoints * bPoints);

        for (unsigned int i = 0; i < aPoints; ++i) {
            for (unsigned int j = 0; j < bPoints; ++j) {
                auto point = std::make_unique<Point>(Application::m_pointModel.get(), 0.5f);
                point->deletable = false;

                float angle = (2 * std::numbers::pi_v<float> *i) / aPoints;
                float x = m_a * std::cos(angle);
                float y = m_a * std::sin(angle);
                float z = (m_b * j) / (bPoints - 1) - m_b / 2.0f;
                point->SetTranslation(x, z, y);

                controlPoints.push_back(point.get());
                m_sceneObjects.push_back(std::move(point));
            }
        }

        for (unsigned int i = 0; i < m_aPatch; ++i) {
            for (unsigned int j = 0; j < m_bPatch; ++j) {
                std::array<USHORT, Patch::patchSize> indices;

                for (USHORT u = 0; u < Patch::rowSize; ++u) {
                    for (USHORT v = 0; v < Patch::rowSize; ++v) {
                        USHORT wrapped_i = (i + u) % aPoints;
                        indices[u * Patch::rowSize + v] = wrapped_i * bPoints + (j + v);
                    }
                }
                patches.emplace_back(indices);
            }
        }
    }

    return new BSurface(m_type, aPoints, bPoints, m_divisions, controlPoints, patches);
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

        int min_aPatch = 1;
        if (m_isC2 && m_type == SurfaceType::Cylindric) {
            // special case, where for C2 cylinder minimum number of patches in A dimension is 3
            min_aPatch = 3;
        }

        int aPatch = m_aPatch;
        ImGui::InputInt("First count", &aPatch, 1, 2);
        m_aPatch = std::max(min_aPatch, aPatch);

        int bPatch = m_bPatch;
        ImGui::InputInt("Second count", &bPatch, 1, 2);
        m_bPatch = std::max(1, bPatch);

        int divisions = m_divisions;
        ImGui::InputInt("Divisions", &divisions, 1, static_cast<int>(Surface::minDivisions));
        m_divisions = std::min(std::max(static_cast<int>(Surface::minDivisions), divisions), static_cast<int>(Surface::maxDivisions));

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
		return BuildBSurface();
	} else {
        return BuildSurface();
	}
}
