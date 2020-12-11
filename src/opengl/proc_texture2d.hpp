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

    Noise::value_t operator[](uint32_t idx) const { return m_noiseMap[idx]; }

    //------------------------------------------------------------
	// Setters

    /** @brief Sets a new size and refills and reapplies the texture */
    void setSize(uint32_t res);

    /** 
     * @brief Sets a new noise type and refills and reapplies the texture
     * @return True if changed else false
     */
    bool setType(Noise::Type t)
    {
        if (t != m_noiseType) 
        {
            m_noiseType = t; 
            fill();
            return true;
        }
        return false;
    }

    void setScale(float scale)
    {
        m_scale = std::max(0.0001f, scale);
        if (m_noiseType != Noise::Random) { fill(); }
    }

    // From Noise class
    void setOctaves(uint32_t octaves)
    {
        m_noise.m_octaves = octaves;
        if (m_noiseType == Noise::OctavesPerlin2D) { fill(); }
    }

    void setPersistence(float persistence)
    {
        glm::clamp(persistence, 0.0001f, 1.0f);
        m_noise.m_persistence = persistence;
        if (m_noiseType == Noise::OctavesPerlin2D) { fill(); }
    }

    void reseed(uint32_t seed)
    {
        m_noise.m_gen.seed(seed);
        m_noise.m_seed = seed;
        fill();
    }

    //------------------------------------------------------------
	// Getters
	glm::uvec2 size() const { return glm::uvec2(m_resolution, m_resolution); }

    float scale() const { return m_scale; }

    const Texture2D& texture() const { return m_texture; }

    uint32_t ID() const { return m_texture.ID(); }

    // From Noise class
    uint32_t seed() const { return m_noise.m_seed; }

    Noise::Type type() const { return m_noiseType; }

    uint32_t octaves() const { return m_noise.m_octaves; }

    float persistence() const { return m_noise.m_persistence; }

private:
    void fill();

    void fillRandom();

    void fillPerlin2D();

    void fillOctavesPerlin2D();

    void applyTexture();

private:
	uint32_t m_resolution;          ///< A square texture
    float m_scale;                  ///< For noise scaling

    Noise m_noise;
    Noise::Type m_noiseType;        ///< Type of noiseMap generator
    std::vector<Noise::value_t> m_noiseMap;

    Texture2D m_texture;            ///< To visualize the noiseMap
    uint32_t m_pixelSize;
    std::vector<float> m_colorMap;
};

