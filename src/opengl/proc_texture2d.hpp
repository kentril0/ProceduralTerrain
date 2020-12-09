#pragma once

#include "texture2d.hpp"
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
     * @param fnc Noise function to generate values from
     */
    ProceduralTex2D(uint32_t res, const function<float()>& fnc);

    ~ProceduralTex2D() { delete[] data; }


    //------------------------------------------------------------
	// Setters
    void set_size(uint32_t res) { m_resolution = res; }

    //------------------------------------------------------------
	// Getters
    const Texture2D& texture() const { return m_texture; }

    uint32_t size() const { return m_resolution; }

	glm::uvec2 size() const { return glm::uvec2(m_resolution, m_resolution); }

private:
    void fill();

private:

	uint32_t m_resolution;          ///< A square texture

    Texture2D m_texture;
    uint8_t m_data;                 ///< CPU side data

    std::function<float()> noise_fnc;
};

