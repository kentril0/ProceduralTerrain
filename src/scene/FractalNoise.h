/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include "PerlinNoise.h"


/** 
 * @brief Fractal noise generator based on *Perlin Noise*
 */
template <typename T>
class FractalNoise
{
public:
    /**
     * @param scale Scales the sample
     * @param offset Offsets the sample
     * @param octaves Number of octaves
     * @param gain Scales amplitude, influence of a successive octave
     * @param lacunarity Scales frequency with each successive octave
     */
    FractalNoise(const PerlinNoise<T>& perlinNoise,
                 T scale = (T)1,
                 T offset = (T)0,
                 uint32_t octaves = 6,
                 T gain = (T)0.5,
                 T lacunarity = (T)2)
        : m_PerlinNoise(perlinNoise),
          m_Scale(scale),
          m_Offset(offset),
          m_Octaves(octaves),
          m_Gain(gain),
          m_Lacunarity(lacunarity) {}

    /** @return 3D Fractal noise value in [0,1] */
    T Noise(T x, T y, T z)
    {
        T sum = 0;
        T max = (T)0;
        T frequency = (T)1;
        T amplitude = (T)1;

        for (uint32_t i = 0; i < m_Octaves; ++i)
        {
            T noiseVal = m_PerlinNoise.Noise(
                (x + m_Offset) / m_Scale * frequency,
                (y + m_Offset) / m_Scale * frequency,
                (z + m_Offset) / m_Scale * frequency); // * (T)2 - (T)1 ;

            sum += noiseVal * amplitude;
            max += amplitude;

            amplitude *= m_Gain;
            frequency *= m_Lacunarity;
        }

        sum = sum / max;
        return (sum + (T)1.0) / (T)2.0;
    }

    void SetScale(T scale) { m_Scale = scale; }
    void SetOffset(T offset) { m_Offset = offset; }
    void SetOctaves(uint32_t octaves) { m_Octaves = octaves; }
    void SetGain(T gain) { m_Gain = gain; }
    void SetLacunarity(T lacunarity) { m_Lacunarity = lacunarity; }

private:
    PerlinNoise<T> m_PerlinNoise;
    T m_Scale{ 0 };
    T m_Offset{ 0 };

    uint32_t m_Octaves{ 0 };
    T m_Gain{ 0 };
    T m_Lacunarity{ 0 };
};
