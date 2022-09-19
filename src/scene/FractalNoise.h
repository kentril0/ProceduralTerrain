/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "PerlinNoise.h"


/** 
 * @brief Fractal noise generator based on *Perlin Noise*
 *  @see Sascha Willems. Vulkan. https://github.com/SaschaWillems/Vulkan.
 */
template <typename T>
class FractalNoise
{
public:
    FractalNoise(const PerlinNoise<T>& perlinNoise)
        : m_PerlinNoise(perlinNoise),
          m_Octaves(6),
          m_Persistence((T)0.5) {}

    T Noise(T x, T y, T z)
    {
    	T sum = 0;
    	T frequency = (T)1;
    	T amplitude = (T)1;
    	T max = (T)0;
    	for (uint32_t i = 0; i < m_Octaves; i++)
    	{
    		sum += m_PerlinNoise.Noise(x * frequency,
                                       y * frequency,
                                       z * frequency) * amplitude;
    		max += amplitude;
    		amplitude *= m_Persistence;
    		frequency *= (T)2;
    	}

    	sum = sum / max;
    	return (sum + (T)1.0) / (T)2.0;
    }

private:
    PerlinNoise<T> m_PerlinNoise;

    uint32_t m_Octaves;
    T m_Frequency, m_Amplitude, m_Persistence;
};
