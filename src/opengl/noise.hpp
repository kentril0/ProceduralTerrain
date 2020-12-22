/**********************************************************
 * < Procedural Terrain Generator >
 * @author Martin Smutny, xsmutn13@stud.fit.vutbr.cz
 * @date 20.12.2020
 * @file noise.hpp
 * @brief Object for generating pseudorandom noise
 *********************************************************/

#pragma once

#include <random>


/**
 * @brief Singleton class for generating values based on noise functions
 */
class Noise
{
public:
    typedef float value_t;

    enum Type
    {
        Random,
        Perlin2D,
        OctavesPerlin2D
    };

    Noise();

    /** @return Pseudorandom value from uniform distribution, in <0.0, 1.0> */
    value_t random() { return m_distrib(m_gen); }

    /** 
     * @brief Perlin noise in 3D
     * @return Noise value in <-1.0, 1.0>
     */
    value_t perlin(value_t x, value_t y, value_t z) const;

    /** 
     * @brief Normalized Perlin noise in 3D
     * @return Noise value in <0.0, 1.0>
     */
    value_t perlinNorm(value_t x, value_t y, value_t z) const
    {
        return (perlin(x, y, z) + 1) * 0.5;
    }

    value_t perlin2D(value_t x, value_t y) const { return perlin(x, y, 0); }

    value_t perlin2DNorm(value_t x, value_t y) const { return perlinNorm(x, y, 0); }
    /**
     * @brief Sample perlin noise with various properties and sum it together
     * @param octaves Number of octaves to sum
     * @param persistence Influence of each successive octave i
     *          amplitutde = persistence ^ i
     * @return Summed noise value in <-1.0, 1.0>
     */
    value_t octavesPerlin(value_t x, value_t y, value_t z) const;

    value_t octavesPerlin2D(value_t x, value_t y) const;

    static constexpr value_t inverseLerp(value_t min, value_t max, value_t v) noexcept
    {
        return (v - min) / (max - min);
    }

    void reseed(uint32_t seed);

private:
    //@brief Linear interpolation
    static constexpr value_t lerp(value_t a, value_t b, value_t t) noexcept
    {
        return a + t * (b - a);
    }

    /** 
     * @brief Fade function defined by Ken Perlin, smoothes the output
     *        6t^5 - 15t^4 + 10t^3
     */
    static constexpr value_t fade(value_t t) noexcept
    {
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    /**
     * @brief Gradient function by Ken Perlin, faster implementation
     *        Tested, about 1/3 faster.
     *  Source: http://riven8192.blogspot.com/2010/08/calculate-perlinnoise-twice-as-fast.html
     * @return Picks a random vector from the 12 selected vectors and computes
     *         a dot product
     */
    static constexpr value_t grad(int hash, value_t x, value_t y, value_t z) noexcept
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

public:
    static const uint32_t PERM_SIZE = 512;      ///< Size of the permutation table 
    static const uint32_t RANGE = 256;          ///< Range of values in the perm. table

    uint32_t m_seed;
    std::default_random_engine m_gen;
    std::uniform_real_distribution<value_t> m_distrib;

    int32_t p[PERM_SIZE];                ///< Permutation table

    // Accumulated Perlin noise global values
    uint32_t m_octaves;
    float    m_persistence;
    float    m_lacunarity;

};

