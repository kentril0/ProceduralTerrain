#pragma once

#include <random>


/**
 * @brief Singleton class for generating values based on noise functions
 */
class Noise
{
public:
    typedef float value_t;

    static void reseed(uint32_t seed)
    {
        m_gen.seed(seed);
        m_seed = seed;
    }

    /** @return Pseudorandom value from uniform distribution, in <0.0, 1.0> */
    static value_t random() const { return m_distrib(m_gen); }

    /** 
     * @brief 3D Perlin noise
     * @return Noise value in <0.0, 1.0>
     */
    static value_t perlin(value_t x, value_t y, value_t z) const;

    static value_t perlin2D(value_t x, value_t y) const { return perlin(x, y, 0); }

    /**
     * @brief Sample perlin noise with various properties and sum it together
     * @param octaves Number of octaves to sum
     * @param persistence Influence of each successive octave i
     *          amplitutde = persistence ^ i
     * @return Summed noise value in <0.0, 1.0>
     */
    static value_t octave_perlin(value_t x, value_t y, value_t z, 
                                 uint32_t octaves, value_t persistence) const;

private:
    Noise();

    /** 
     * @brief Fade function defined by Ken Perlin, smoothes the output
     *        6t^5 - 15t^4 + 10t^3
     */
    static value_t fade(value_t t) const
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
    static value_t grad(int hash, value_t x, value_t y, value_t z) const
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

private:
    static Noise instance;

    static uint32_t m_seed;
    static std::default_random_engine m_gen;
    static std::uniform_real_distribution<value_t> m_distrib;

    static const uint32_t PERM_SIZE = 512;      ///< Size of the permutation table 
    static const uint32_t RANGE = 256;          ///< Range of values in the perm. table

    static int32_t p[PERM_SIZE];                ///< Permutation table

};
