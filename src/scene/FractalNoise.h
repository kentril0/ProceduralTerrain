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
    
    FractalNoise(const PerlinNoise<T>& perlinNoise,
                 T scale = (T)1,
                 T offset = (T)0,
                 uint32_t octaves = 6,
                 T gain = (T)0.5,
                 T lacunarity = (T)2)
        : perlinNoise(perlinNoise),
          scale(scale),
          offset(offset),
          octaveCount(octaves),
          gain(gain),
          lacunarity(lacunarity) {}

    /** @return 3D Fractal noise value in [0,1] */
    T Noise(T x, T y, T z)
    {
        T sum = 0;
        T max = (T)0;
        T frequency = (T)1;
        T amplitude = (T)1;

        for (uint32_t i = 0; i < octaveCount; ++i)
        {
            T noiseVal = perlinNoise.Noise(
                (x + offset) / scale * frequency,
                (y + offset) / scale * frequency,
                (z + offset) / scale * frequency); // * (T)2 - (T)1 ;

            sum += noiseVal * amplitude;
            max += amplitude;

            amplitude *= gain;
            frequency *= lacunarity;
        }

        sum = sum / max;
        return (sum + (T)1.0) / (T)2.0;
    }

public:
    PerlinNoise<T> perlinNoise;

    uint32_t octaveCount{ 0 };  ///< Number of octaves
    T scale{ 0 };               ///< Scales the sample
    T offset{ 0 };              ///< Offsets the sample
    T gain{ 0 };                ///< Scales amplitude, influence of each successive octave
    T lacunarity{ 0 };          ///< Scales frequency with each successive octave
};
