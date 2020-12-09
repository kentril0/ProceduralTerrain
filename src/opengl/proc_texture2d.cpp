#include "core/pch.hpp"
#include "proc_texture2d.hpp"


ProceduralTex2D::ProceduralTex2D(uint32_t res, const function<float()>& fnc)
  : m_resolution(res), 
    m_texture(true),
    noise_fnc(fnc)
{
    // Internal format variants
    //  GL_R8, base format: GL_RED
    //  GL_R8_SNORM, base: GL_RED
    //  GL_RGB8, base: GL_RGB
    //  GL_RGB8_SNORM, base: GL_RGB
    //  GL_RGBA8
    //  GL_RGBA8_SNORM
    //  GL_R32F, GL_RED
    //  GL_RGB32F, GL_RGB

    // Default: GL_RGBA8, base: GL_RGBA
    data = new uint8_t[res];

    m_texture.set_image_format(GL_RED)
    m_texture.set_internal_format(GL_R8);
    m_texture.set_clamp_to_edge();
}

void ProceduralTex2D::fill()
{
    for (uint32_t y = 0; y < m_resolution; ++y)
    {
        for (uint32_t x = 0; x < m_resolution; ++x)
        {
            m_data[(y * m_resolution) + x] = noise_fnc();
        }
    }

    m_texture.upload(m_data, m_resolution, m_resolution);
}
