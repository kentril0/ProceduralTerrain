/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <vector>
#include <algorithm>
#include <random>

#include <glm/glm.hpp>


/**
 * @brief Perlin noise
 *  Ken Perlin. JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE. 2004. 
 *  [online]. https://mrl.cs.nyu.edu/~perlin/noise/.
 *  @see Sascha Willems. Vulkan. https://github.com/SaschaWillems/Vulkan.
 */
template <typename T>
class PerlinNoise
{
public:
    PerlinNoise()
    {
        // Generate random lookup for permutations containing all numbers 
        //  from 0..255
        std::vector<uint8_t> permutation;
        permutation.resize(PerlinNoise::PERMUTATION_COUNT);

        std::iota(permutation.begin(), permutation.end(), 0);
        std::default_random_engine rndEngine(std::random_device{}());
        std::shuffle(permutation.begin(), permutation.end(), rndEngine);

        for (uint32_t i = 0; i < PerlinNoise::PERMUTATION_COUNT; ++i)
        {
            m_P[i] = m_P[PerlinNoise::PERMUTATION_COUNT + i] = permutation[i];
        }
    }

    T Noise(T x, T y, T z)
    {
        int32_t X = (int32_t)glm::floor(x) & 255;   // Find unit cube
        int32_t Y = (int32_t)glm::floor(y) & 255;   // that contains a point.
        int32_t Z = (int32_t)glm::floor(z) & 255;
        x -= glm::floor(x);                         // Find relative x,y,z
        y -= glm::floor(y);                         // of point in cube.
        z -= glm::floor(z);
        T u = Fade(x);                              // Compute fade curves
        T v = Fade(y);                              // for each x,y,z.
        T w = Fade(z);
        // Hash coordinates of the 8 cube corners.
        uint32_t A = m_P[X    ] + Y, AA = m_P[A] + Z, AB = m_P[A + 1] + Z;
        uint32_t B = m_P[X + 1] + Y, BA = m_P[B] + Z, BB = m_P[B + 1] + Z;

        // And add blended results from 8 corners of the cube
        return Lerp(w, Lerp(v, Lerp(u, Grad(m_P[AA], x, y, z),
                                       Grad(m_P[BA], x - 1, y, z)),
                               Lerp(u, Grad(m_P[AB], x, y - 1, z),
                                       Grad(m_P[BB], x - 1, y - 1, z))),
                       Lerp(v, Lerp(u, Grad(m_P[AA + 1], x, y, z - 1),
                                       Grad(m_P[BA + 1], x - 1, y, z - 1)),
                               Lerp(u, Grad(m_P[AB + 1], x, y - 1, z - 1),
                                       Grad(m_P[BB + 1], x - 1, y - 1, z - 1))));
    }
private:
    static const size_t PERMUTATION_COUNT = 256;
    uint32_t m_P[512];

private:
    T Fade(T t)
    {
        return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
    }

    T Lerp(T t, T a, T b)
    {
        return a + t * (b - a);
    }

    T Grad(int hash, T x, T y, T z)
    {
        // Convert LO 4 bits of hash code into 12 gradient directions
        int h = hash & 15;
        T u = h < 8 ? x : y;
        T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
};

