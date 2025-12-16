#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "StageFour.h"

using namespace app;

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
            m_boolMap[i / m_resX][i % m_resX] = data[i * channels] > t;
        }
    } else if (channels == 3 || channels == 4) {
        for (int i = 0; i < m_resX * m_resZ; i++) {
            int idx = i * channels;
            unsigned char r = data[idx];
            unsigned char g = data[idx + 1];
            unsigned char b = data[idx + 2];
            m_boolMap[i / m_resX][i % m_resX] = (r > t || g > t || b > t);
        }
    } else {
        stbi_image_free(data);
        throw std::runtime_error("Unsupported number of channels");
    } 
    stbi_image_free(data);
}

StageFour::StageFour() {

}

std::vector<gmod::vector3<float>> app::StageFour::GeneratePath() const {
    return std::vector<gmod::vector3<float>>();
}

void StageFour::LoadImageSTB() {}
