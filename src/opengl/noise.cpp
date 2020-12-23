/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, kentril.despair@gmail.com
 * @date 20.12.2020
 * @file noise.cpp
 * @brief Object for generating pseudorandom noise
 *********************************************************/

#include "core/pch.hpp"
#include "noise.hpp"
#include <algorithm>


Noise::Noise()
  : m_seed(std::default_random_engine::default_seed),
    m_gen(m_seed),
    m_distrib(0.0, 1.0),
    m_octaves(4),
    m_persistence(0.5),
    m_lacunarity(2.)
{
    // Dynamic seed init approach
    // https://github.com/Reputeless/PerlinNoise/blob/master/PerlinNoise.hpp
    reseed(m_seed);
}

/**
 * @brief Improved Perlin noise in 3D
 * @title Understanding Perlin Noise
 * @author Adrian Biagioli
 * @date 09 August 2014
 * @url https://adrianb.io/2014/08/09/perlinnoise.html
 */
Noise::value_t Noise::perlin(Noise::value_t x, Noise::value_t y, Noise::value_t z) const
{
    // Get the coordinates of the unit cube, in range
    //  Perlin noise always repeats every 256 coordinates
    const int32_t xi = static_cast<int32_t>(std::floor(x)) & (RANGE-1);
    const int32_t yi = static_cast<int32_t>(std::floor(y)) & (RANGE-1);
    const int32_t zi = static_cast<int32_t>(std::floor(z)) & (RANGE-1);

    // Location in the unit cube - fractions
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    // Fade the locations to smoothen the results
    const Noise::value_t u = fade(x);
    const Noise::value_t v = fade(y);
    const Noise::value_t w = fade(z);

    // Get unique value for every coordinate using hash function
    //  hashes all 8 unit cube coordinates
    const int32_t A = p[xi] + yi;
    const int32_t AA = p[A] + zi;
    const int32_t AB = p[A + 1] + zi;
    const int32_t B = p[xi + 1] + yi;
    const int32_t BA = p[B] + zi;
    const int32_t BB = p[B + 1] + zi;

    // Weighted average based on the faded (u,v,w)
    return lerp(lerp(lerp(grad(p[AA], x  , y  , z),
                          grad(p[BA], x-1, y  , z),
                          u), 
                     lerp(grad(p[AB], x  , y-1, z),
                          grad(p[BB], x-1, y-1, z),
                          u), 
                     v), 
                lerp(lerp(grad(p[AA+1], x  , y  , z-1),
                          grad(p[BA+1], x-1, y  , z-1),
                          u), 
                     lerp(grad(p[AB+1], x  , y-1, z-1),
                          grad(p[BB+1], x-1, y-1, z-1),
                          u), 
                     v), 
                w);
} 

Noise::value_t Noise::octavesPerlin2D(Noise::value_t x, Noise::value_t y) const
{
    Noise::value_t total = 0;           ///< Noise value
    Noise::value_t amplitude = 1;
    Noise::value_t frequency = 1;

    for(uint32_t i = 0; i < m_octaves; ++i)
    {
        total += perlin2D(x * frequency, y * frequency) * amplitude;
        
        // Apply persistence in <0., 1.>
        amplitude *= m_persistence;
        frequency *= m_lacunarity;
    }
    
    return total;
}

Noise::value_t Noise::octavesPerlin2DNorm(Noise::value_t x, Noise::value_t y) const
{
    Noise::value_t total = 0;           ///< Noise value
    Noise::value_t max_value = 0;       ///< Normalize
    Noise::value_t amplitude = 1;
    Noise::value_t frequency = 1;

    for(uint32_t i = 0; i < m_octaves; ++i)
    {
        total += perlin2D(x * frequency, y * frequency) * amplitude;
        
        max_value += amplitude;
        
        // Apply persistence in <0., 1.>
        amplitude *= m_persistence;
        frequency *= m_lacunarity;
    }
    
    // Normalize to <0.0, 1.0>
    return total / max_value;
}

Noise::value_t Noise::octavesPerlin(Noise::value_t x, Noise::value_t y, 
                                    Noise::value_t z) const
{
    Noise::value_t total = 0;
    Noise::value_t frequency = 1;
    Noise::value_t amplitude = 1;

    for(uint32_t i = 0; i < m_octaves; ++i)
    {
        total += perlin(x * frequency, y * frequency, z * frequency) * amplitude;
        
        amplitude *= m_persistence;
        frequency *= m_lacunarity;
    }
    
    return total;
}

Noise::value_t Noise::octavesPerlinNorm(Noise::value_t x, Noise::value_t y, 
                                        Noise::value_t z) const
{
    Noise::value_t total = 0;
    Noise::value_t max_value = 0; 
    Noise::value_t frequency = 1;
    Noise::value_t amplitude = 1;

    for(uint32_t i = 0; i < m_octaves; ++i)
    {
        total += perlin(x * frequency, y * frequency, z * frequency) * amplitude;
        
        max_value += amplitude;
        
        amplitude *= m_persistence;
        frequency *= m_lacunarity;
    }
    
    return total / max_value;
}

void Noise::reseed(uint32_t seed)
{
    m_seed = seed;
    m_gen.seed(seed);

    for (uint32_t i = 0; i < RANGE; ++i)
		p[i] = i;

    std::shuffle(std::begin(p), std::begin(p) + RANGE, m_gen);

    for (uint32_t i = 0; i < RANGE; ++i)
        p[RANGE + i] = p[i];
}

