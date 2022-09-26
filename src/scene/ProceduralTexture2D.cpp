/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#include "ProceduralTexture2D.h"

#include <memory>
#include <vector>
#include <limits>


std::shared_ptr<ProceduralTexture2D> ProceduralTexture2D::Create(
    const glm::uvec2& size)
{
    return std::make_shared<ProceduralTexture2D>(size);
}

// =============================================================================

ProceduralTexture2D::ProceduralTexture2D(uint32_t rows, uint32_t cols)
    : m_Rows(rows),
      m_Cols(cols),
      m_PerlinNoise(),
      m_FractalNoise(m_PerlinNoise, m_Scale)
{
    GenerateValues();
}

ProceduralTexture2D::ProceduralTexture2D(const glm::uvec2& size)
    : ProceduralTexture2D(size.x, size.y)
{

}

void ProceduralTexture2D::GenerateValues()
{
    // TODO time

    m_Values.resize(m_Rows * m_Cols);

    m_MinValue = std::numeric_limits<float>::max();
    m_MaxValue = std::numeric_limits<float>::min();

    for (uint32_t x = 0; x < m_Rows; ++x)
        for (uint32_t y = 0; y < m_Cols; ++y)
        {
            const uint32_t kIndex = x*m_Cols + y;
            const float kValue = m_FractalNoise.Noise(x, y, 0);

            m_Values[kIndex] = kValue;

            m_MinValue = glm::min(m_MinValue, kValue);
            m_MaxValue = glm::max(m_MaxValue, kValue);
        }
}

void ProceduralTexture2D::SetSize(const glm::uvec2& size)
{
    m_Rows = size.x;
    m_Cols = size.y;
}

void ProceduralTexture2D::SetScale(float scale)
{
    scale = glm::max(0.001f, scale);
    m_Scale = scale;
    m_FractalNoise.SetScale(scale);
}
