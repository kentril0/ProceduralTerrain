/**
 *  Copyright (c) 2022 ProceduralTerrain authors Distributed under MIT License 
 * (http://opensource.org/licenses/MIT)
 */

#pragma once

#include <array>
#include <algorithm>
#include <random>

#include <glm/glm.hpp>

// TODO to calculate gradient using a switch case, should be faster
// #define CALC_GRAD_SWITCH

/**
 * @brief Perlin noise
 *  Ken Perlin. JAVA REFERENCE IMPLEMENTATION OF IMPROVED NOISE. 2004. 
 *  [online]. https://mrl.cs.nyu.edu/~perlin/noise/.
 */
template <typename T>
class PerlinNoise
{
public:
    PerlinNoise(int32_t seed = std::random_device{}())
        : m_Seed(seed)
    {
        GeneratePermutations();
    }

    int32_t GetSeed() const { return m_Seed; }
    /** @brief Just sets the seed, generate the permutations to see diff. */
    void SetSeed(int32_t seed) { m_Seed = seed; }

    /** @brief Generate random lookup for permutations containing all numbers
     * from 0..255 */
    void GeneratePermutations()
    {
        std::iota(m_P.begin(), m_P.begin() + PerlinNoise::PERMUTATION_COUNT, 0);

        std::default_random_engine rndEngine(m_Seed);
        std::shuffle(m_P.begin(), m_P.begin() + PerlinNoise::PERMUTATION_COUNT,
                     rndEngine);

        for (uint32_t i = 0; i < PerlinNoise::PERMUTATION_COUNT; ++i)
            m_P[PerlinNoise::PERMUTATION_COUNT + i] = m_P[i];
    }

    /** @return Perlin 3D noise value in [-1,1] */
    T Noise(T x, T y, T z) const
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
    // Fade fnc, qunitic fnc: 6t^5 - 15t^4 + 10t^3
    constexpr T Fade(T t) const
    {
        return t * t * t * (t * (t * (T)6 - (T)15) + (T)10);
    }

    constexpr T Lerp(T t, T a, T b) const
    {
        return a + t * (b - a);
    }

#ifndef CALC_GRAD_SWITCH
    constexpr T Grad(int hash, T x, T y, T z) const
    {
        // Convert LO 4 bits of hash code into 12 gradient directions
        int h = hash & 15;
        T u = h < 8 ? x : y;
        T v = h < 4 ? y : h == 12 || h == 14 ? x : z;
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
#else
    /** @see http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html */
    constexpr T Grad(int hash, T x, T y, T z) const
    {
        switch(hash & 0xF)
        {
            case 0x0: return  x + y;
            case 0x1: return -x + y;
            case 0x2: return  x - y;
            case 0x3: return -x - y;
            case 0x4: return  x + z;
            case 0x5: return -x + z;
            case 0x6: return  x - z;
            case 0x7: return -x - z;
            case 0x8: return  y + z;
            case 0x9: return -y + z;
            case 0xA: return  y - z;
            case 0xB: return -y - z;
            case 0xC: return  y + x;
            case 0xD: return -y + z;
            case 0xE: return  y - x;
            case 0xF: return -y - z;
            default: return 0; // never happens
        }
    }
#endif // CALC_GRAD_SWITCH

private:
    static const size_t PERMUTATION_COUNT = 256;

    std::array<uint32_t, PERMUTATION_COUNT*2> m_P;    ///< Permutations
    int32_t m_Seed{ 0 };
};

