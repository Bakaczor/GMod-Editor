#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "StageFour.h"
#include <stack>

using namespace app;

StageFour::StageFour() {
    LoadImageSTB();
}

void StageFour::LoadImageSTB() {
    int channels;
    unsigned char* data = stbi_load(m_texturePath.c_str(), &m_resX, &m_resZ, &channels, 0);

    if (!data) {
        throw std::runtime_error("Failed to load image: " + m_texturePath);
    }

    m_boolMap = std::vector<std::vector<bool>>(m_resZ, std::vector<bool>(m_resX));

    int t = 127;
    if (channels == 1 || channels == 2) {
        for (int i = 0; i < m_resX * m_resZ; i++) {
            m_boolMap[i / m_resX][i % m_resX] = data[i * channels] < t;
        }
    } else if (channels == 3 || channels == 4) {
        for (int i = 0; i < m_resX * m_resZ; i++) {
            int idx = i * channels;
            unsigned char r = data[idx];
            unsigned char g = data[idx + 1];
            unsigned char b = data[idx + 2];
            m_boolMap[i / m_resX][i % m_resX] = (r < t || g < t || b < t);
        }
    } else {
        stbi_image_free(data);
        throw std::runtime_error("Unsupported number of channels");
    } 
    stbi_image_free(data);
}

std::vector<StageFour::Pixel> StageFour::PixelPath(std::vector<std::vector<bool>>& visited, int xStart, int zStart) const {
    std::vector<Pixel> path;
    if (xStart < 0 || xStart >= m_resX || zStart < 0 || zStart >= m_resZ) {
        return path;
    }

    if (visited[zStart][xStart] || !m_boolMap[zStart][xStart]) {
        return path; 
    }

    const int dx[8] = { -1, 0, 1, -1, 1, -1, 0, 1 };
    const int dz[8] = { -1, -1, -1, 0, 0, 1, 1, 1 };

    std::stack<Pixel> stack;
    stack.push({ xStart, zStart });
    visited[zStart][xStart] = true;
    path.push_back({ xStart, zStart });

    while (!stack.empty()) {
        auto [x, z] = stack.top();

        bool foundUnvisitedNeighbor = false;
        for (int i = 0; i < 8; i++) {
            int nx = x + dx[i];
            int nz = z + dz[i];

            if (nx < 0 || nx >= m_resX || nz < 0 || nz >= m_resZ) {
                continue;
            }

            if (!visited[nz][nx] && m_boolMap[nz][nx]) {
                foundUnvisitedNeighbor = true;
                visited[nz][nx] = true;
                stack.push({ nx, nz });
                path.push_back({ nx, nz });
                break;
            } 
        }

        if (!foundUnvisitedNeighbor) {
            stack.pop();
            if (!stack.empty()) {
                // moving back
                path.push_back(stack.top());
            }
        }
    }

    return path;
}

gmod::vector3<float> StageFour::Pixel2Pos(int x, int z) const {
    const float stepX = width / m_resX;
    const float stepZ = length / m_resZ;
    return gmod::vector3<float>(x * stepX + topLeftCorner.z(), baseY, z * stepZ + topLeftCorner.z());
}

std::vector<gmod::vector3<float>> StageFour::GeneratePath() const {
    auto areSimilar = [&](const gmod::vector3<float>& a, const gmod::vector3<float>& b, const gmod::vector3<float>& c) -> bool {
        double area = std::abs((b.x() - a.x()) * (c.z() - a.z()) - (c.x() - a.x()) * (b.z() - a.z()));
        return area < 1e-4f;
    };

    std::vector<gmod::vector3<float>> path;
    path.push_back(gmod::vector3<float>(0, totalHeight, 0));

    bool first = true;
    std::vector<std::vector<bool>> visited(m_resZ, std::vector<bool>(m_resX, false));
    for (int z = 0; z < m_resZ; z++) {
        for (int x = 0; x < m_resX; x++) {
            if (visited[z][x]) { continue; } // been here already
            if (!m_boolMap[z][x]) { continue; } // not a part of object

            float height = saveHeight;
            if (first) {
                first = false;
                height = totalHeight;
            }

            std::vector<gmod::vector3<float>> rawPath;

            auto pixelPath = PixelPath(visited, x, z);
            auto start = Pixel2Pos(pixelPath.front().x, pixelPath.front().z);
            rawPath.push_back(gmod::vector3<float>(start.x(), height, start.z()));
            for (const auto& p : pixelPath) {
                rawPath.push_back(Pixel2Pos(p.x, p.z));
            }
            auto end = Pixel2Pos(pixelPath.back().x, pixelPath.back().z);
            rawPath.push_back(gmod::vector3<float>(end.x(), saveHeight, end.z()));

            std::vector<gmod::vector3<float>> filtered;
            filtered.push_back(rawPath.front());
            for (size_t k = 1; k < rawPath.size() - 1; k++) {
                auto& prev = filtered.back();
                auto& curr = rawPath[k];
                auto& next = rawPath[k + 1];

                if (!areSimilar(prev, curr, next)) {
                    filtered.push_back(curr);
                }
            }
            for (const auto& p : filtered) {
                path.push_back(p);
            }
        }
    }
    path.push_back(gmod::vector3<float>(path.back().x(), totalHeight, path.back().z()));
    path.push_back(gmod::vector3<float>(0, totalHeight, 0));

    return path;
}
