/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file proc_texture2d.cpp
 * @brief Procedural 2D texture abstraction
 *********************************************************/

#include "core/pch.hpp"
#include "proc_texture2d.hpp"


ProceduralTex2D::ProceduralTex2D(uint32_t res, Noise::Type t)
  : m_resolution(res), 
    m_scale(27.6),
    m_noiseType(t),
    m_texture(true),
    m_changed(false)
{
    m_pixelSize = CHANNELS_RGB;
    m_texture.set_internal_format(GL_RGB8);
    m_texture.set_image_format(GL_RGB);
    m_texture.set_clamp_to_edge();

    fill();
}

void ProceduralTex2D::fill()
{
    m_noiseMap.resize(m_resolution * m_resolution);

    switch (m_noiseType)
    {
        case Noise::Random:
            fillRandom();
            break;
        case Noise::Perlin2D:
            fillPerlin2D();
            break;
        case Noise::OctavesPerlin2D:
            fillOctavesPerlin2D();
            break;
    }

    applyTexture();
    m_changed = true;
}

void ProceduralTex2D::fillRandom()
{
    for (uint32_t y = 0; y < m_resolution; ++y)
        for (uint32_t x = 0; x < m_resolution; ++x)
            m_noiseMap[(y * m_resolution) + x] = m_noise.random();
}

void ProceduralTex2D::fillPerlin2D()
{
    for (uint32_t y = 0; y < m_resolution; ++y)
        for (uint32_t x = 0; x < m_resolution; ++x)
            m_noiseMap[(y * m_resolution) + x] = m_noise.perlin2DNorm(x / m_scale, 
                                                                      y / m_scale);
}

void ProceduralTex2D::fillOctavesPerlin2D()
{
    float maxNoise = -9999, minNoise = 9999;
    for (uint32_t y = 0; y < m_resolution; ++y)
        for (uint32_t x = 0; x < m_resolution; ++x)
        {
            Noise::value_t noise = m_noise.octavesPerlin2D(x / m_scale, y / m_scale);
            m_noiseMap[y * m_resolution + x] = noise;

            if (noise > maxNoise)
                maxNoise = noise;
            else if (noise < minNoise)
                minNoise = noise;
        }

    // Normalize to <0., 1.>
    for (uint32_t y = 0; y < m_resolution; ++y)
        for (uint32_t x = 0; x < m_resolution; ++x)
        {
            uint32_t index = y * m_resolution + x;
            m_noiseMap[index] = Noise::inverseLerp(minNoise, maxNoise, m_noiseMap[index]);
        }
}

void ProceduralTex2D::applyTexture()
{
    m_colorMap.resize(m_resolution * m_resolution * m_pixelSize);

    for (uint32_t y = 0; y < m_resolution; ++y)
        for (uint32_t x = 0; x < m_resolution; ++x)
        {
            uint32_t index = (y * m_resolution) + x;
            Noise::value_t v = m_noiseMap[index];

            m_colorMap[index*m_pixelSize + 0] = v;
            m_colorMap[index*m_pixelSize + 1] = v;
            m_colorMap[index*m_pixelSize + 2] = v;
        } 

    m_texture.upload(&m_colorMap[0], m_resolution, m_resolution);
}

void ProceduralTex2D::setSize(uint32_t res)
{ 
    if (res != m_resolution)
    {
        m_resolution = res;
        fill();
    }
}

