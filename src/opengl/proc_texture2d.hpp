#pragma once

#include "texture2d.hpp"
#include "noise.hpp"
#include <functional>


/**
 * @brief Class for generating procedural 2D textures
 *  TODO only square textures
 */
class ProceduralTex2D
{
public:
    /**
     * @brief Creates a procedural texture, 8-bit
     * @param res Resolution, both width and height are same
     * @param t Type of noise function
     */
    ProceduralTex2D(uint32_t res, Noise::Type t = Noise::Random);

    //~ProceduralTex2D() { delete[] data; }

    Noise::value_t operator[](uint32_t idx) const { return m_noiseMap[idx]; }

    //------------------------------------------------------------
	// Setters

    /** @brief Sets a new size and refills and reapplies the texture */
    void setSize(uint32_t res);

    /** @brief Sets a new noise type and refills and reapplies the texture */
    void setNoiseType(Noise::Type t)
    {
        if (t != m_noiseType) 
        {
            m_noiseType = t; 
            fill();
        }
    }


    //------------------------------------------------------------
	// Getters
    const Texture2D& texture() const { return m_texture; }

	glm::uvec2 size() const { return glm::uvec2(m_resolution, m_resolution); }

    uint32_t ID() const { return m_texture.ID(); }

private:
    void fill();

    void fillRandom();

    void fillPerlin2D();

    void fillOctavesPerlin2D();

    void applyTexture();

private:
	uint32_t m_resolution;          ///< A square texture
    Noise::Type m_noiseType;        ///< Type of noise function
    std::vector<Noise::value_t> m_noiseMap;

    Texture2D m_texture;
    uint32_t m_pixelSize;
    std::vector<float> m_colorMap;
};

