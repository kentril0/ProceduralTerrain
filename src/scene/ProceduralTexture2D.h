/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <SGL/opengl/Texture2D.h>

#include "PerlinNoise.h"
#include "FractalNoise.h"


// TODO template
class ProceduralTexture2D
{
public:
    using NoiseValue = float;

    static std::shared_ptr<ProceduralTexture2D> Create(uint32_t width,
                                                       uint32_t height);
public:
    ProceduralTexture2D(uint32_t width,
                        uint32_t height);

    /** 
     * @brief Assumes the index is within bounds
     * @return Noise value at index
     */
    NoiseValue operator[](uint32_t index) const { return m_Values[index]; }
    NoiseValue operator[](const glm::uvec2& coord) const {
        return m_Values[coord.x * m_Width + coord.y];
    }

    NoiseValue At(size_t index) const {
        return m_Values[ std::min(index, m_Values.size()-1) ];
    }

    /** @brief Generates values based on the set size */
    void GenerateValues();

    /** @brief Updates the texture with the generated values */
    void UpdateTexture();

    /** @param size x: Width, y: height */
    void SetSize(const glm::uvec2& size);
    glm::uvec2 GetSize() const { return glm::uvec2(m_Width, m_Height); }

    std::vector<NoiseValue>& GetValues() { return m_Values; }
    const std::vector<NoiseValue>& GetValues() const { return m_Values; }
    float GetMinValue() const { return m_MinValue; }
    float GetMaxValue() const { return m_MaxValue; }

    const std::shared_ptr<sgl::Texture2D>& GetTexture() const { return m_Texture; }

    // Noise function settings

    void SetSeed(int32_t seed) {
        m_FractalNoise.perlinNoise.SetSeed(seed);
        m_FractalNoise.perlinNoise.GeneratePermutations();
    }
    void SetOctaves(int octaves) { m_FractalNoise.octaveCount = octaves; }
    void SetScale(float scale);
    void SetOffset(float offset) { m_FractalNoise.offset = offset; }
    void SetGain(float gain) { m_FractalNoise.gain = gain; }
    void SetLacunarity(float lacunarity) { m_FractalNoise.lacunarity = lacunarity; }

    int32_t GetSeed() const { return m_FractalNoise.perlinNoise.GetSeed(); }
    int GetOctaves() const { return m_FractalNoise.octaveCount; }
    float GetScale() const { return m_FractalNoise.scale; }
    float GetOffset() const { return m_FractalNoise.offset; }
    float GetGain() const { return m_FractalNoise.gain; }
    float GetLacunarity() const { return m_FractalNoise.lacunarity; }

private:
    uint32_t m_Width{ 0 };
    uint32_t m_Height{ 0 };

    FractalNoise<NoiseValue> m_FractalNoise;
    std::vector<NoiseValue> m_Values;

    float m_MinValue{ 0.0 };
    float m_MaxValue{ 0.0 };

    std::shared_ptr<sgl::Texture2D> m_Texture;
};
