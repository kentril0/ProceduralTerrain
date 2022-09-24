/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "PerlinNoise.h"
#include "FractalNoise.h"


// TODO template
class ProceduralTexture2D
{
public:
    using NoiseValue = float;

    static std::shared_ptr<ProceduralTexture2D> Create(const glm::uvec2& size);

public:
    ProceduralTexture2D(uint32_t rows, uint32_t cols);
    ProceduralTexture2D(const glm::uvec2& size);

    /** @brief Assumes the index is within bounds
     * @return Noise at index */
    NoiseValue operator[](uint32_t index) const { return m_Values[index]; }
    NoiseValue operator[](const glm::uvec2& coord) const {
        return m_Values[coord.x * m_Cols + coord.y];
    }

    NoiseValue At(size_t index) const {
        return m_Values[ std::min(index, m_Values.size()-1) ];
    }

    /** @brief Generates values based on the set size */
    void GenerateValues();

    void SetSize(const glm::uvec2& size);
    void SetScale(float scale);

    std::vector<NoiseValue>& GetValues() { return m_Values; }
    const std::vector<NoiseValue>& GetValues() const { return m_Values; }

    float GetScale() const { return m_Scale; }

private:
    uint32_t m_Rows{ 0 };
    uint32_t m_Cols{ 0 };

    std::vector<NoiseValue> m_Values;
    PerlinNoise<NoiseValue> m_PerlinNoise;
    FractalNoise<NoiseValue> m_FractalNoise;

    float m_Scale{ 1.0 };

    //static constexpr NoiseValue kOutOfBoundsValue = 0;
};
