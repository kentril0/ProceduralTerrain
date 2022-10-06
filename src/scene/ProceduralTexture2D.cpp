/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "ProceduralTexture2D.h"

#include <memory>
#include <vector>
#include <limits>

#include <SGL/SGL.h>


std::shared_ptr<ProceduralTexture2D> ProceduralTexture2D::Create(
    uint32_t width, uint32_t height)
{
    return std::make_shared<ProceduralTexture2D>(width, height);
}

// =============================================================================

ProceduralTexture2D::ProceduralTexture2D(uint32_t width, uint32_t height)
    : m_Width(width),
      m_Height(height),
      m_FractalNoise(PerlinNoise<NoiseValue>()),
      m_Texture(sgl::Texture2D::Create())
{
    //GenerateValues();
    //UpdateTexture();
}

void ProceduralTexture2D::GenerateValues()
{
    // TODO time

    m_Values.resize(m_Width * m_Height);

    m_MinValue = std::numeric_limits<float>::max();
    m_MaxValue = std::numeric_limits<float>::min();

    for (uint32_t y = 0; y < m_Height; ++y)
        for (uint32_t x = 0; x < m_Width; ++x)
        {
            const uint32_t kIndex = y*m_Width + x;
            const float kValue = m_FractalNoise.Noise(x, y, 0);

            m_Values[kIndex] = kValue;

            m_MinValue = glm::min(m_MinValue, kValue);
            m_MaxValue = glm::max(m_MaxValue, kValue);
        }
}

void ProceduralTexture2D::UpdateTexture()
{
    std::vector<glm::vec3> grayscaleValues;
    grayscaleValues.reserve(m_Values.size());
    std::transform(m_Values.begin(), m_Values.end(),
                   std::back_inserter(grayscaleValues),
                   [](float v) { return glm::vec3(v); });

    // TODO better
    m_Texture->SetData({
        static_cast<int>(m_Width),
        static_cast<int>(m_Height),
        static_cast<const void*>(grayscaleValues.data()),
        GL_RGB8,
        GL_RGB,
        GL_FLOAT,
        false
    });
}

void ProceduralTexture2D::SetSize(const glm::uvec2& size)
{
    m_Width = size.x;
    m_Height = size.y;
}

void ProceduralTexture2D::SetScale(float scale)
{
    scale = glm::max(0.001f, scale);
    m_FractalNoise.scale = scale;
}
