#include "noise.hpp"
#include <cmath>

uint32_t Noise::m_seed = std::default_random_engine::default_seed;

std::default_random_engine Noise::m_gen(m_seed);
std::uniform_real_distribution<Noise::value_t> Noise::m_distrib(0.0, 1.0);

int32_t Noise::p[PERM_SIZE];


Noise::Noise()
{
    // Initialize permutation table
    
    // Hash lookup table as defiend by Ken Perlin, 
    //  array of random numbers from 0-255 inclusive
    int32_t permutation[] = { 151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

    for (uint32_t i = 0; i < PERM_SIZE; ++i)
        p[i] = permutation[i % RANGE];
}

/**
 * @brief Improved Perlin noise in 3D
 * @title Understanding Perlin Noise
 * @author Adrian Biagioli
 * @date 09 August 2014
 * @url https://adrianb.io/2014/08/09/perlinnoise.html
 */
value_t Noise::perlin(value_t x, value_t y, value_t z) const
{
    // Get the coordinates of the unit cube, in range
    //  Perlin noise always repeats every 256 coordinates
    const int32_t xi = static_cast<int32_t>(std::floor(x)) & RANGE-1;
    const int32_t yi = static_cast<int32_t>(std::floor(y)) & RANGE-1;
    const int32_t zi = static_cast<int32_t>(std::floor(z)) & RANGE-1;

    // Location in the unit cube - fractions
    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    // Fade the locations to smoothen the results
    const value_t u = fade(x);
    const value_t v = fade(y);
    const value_t w = fade(z);

    // Get unique value for every coordinate using hash function
    //  hashes all 8 unit cube coordinates
    int aaa, aba, aab, abb, baa, bba, bab, bbb;
    aaa = p[ p[p[xi  ] + yi  ] + zi  ];
    aba = p[ p[p[xi  ] + yi+1] + zi  ];
    aab = p[ p[p[xi  ] + yi  ] + zi+1];
    abb = p[ p[p[xi  ] + yi+1] + zi+1];
    baa = p[ p[p[xi+1] + yi  ] + zi  ];
    bba = p[ p[p[xi+1] + yi+1] + zi  ];
    bab = p[ p[p[xi+1] + yi  ] + zi+1];
    bbb = p[ p[p[xi+1] + yi+1] + zi+1];

    //const int32_t A = p[xi] + yi;
    //const int32_t AA = p[A] + zi;
    //const int32_t AB = p[A + 1] + zi;
    //const int32_t B = p[xi + 1] + yi;
    //const int32_t BA = p[B] + zi;
    //const int32_t BB = p[B + 1] + zi;

    // Weighted average based on the faded (u,v,w)
    value_t x1, x2, y1, y2;

    x1 = std::lerp(grad(aaa, x  , y  , z),
                   grad(baa, x-1, y  , z),
                   u);
    x2 = std::lerp(grad(aba, x  , y-1, z),
                   grad(bba, x-1, y-1, z),
                   u);
    y1 = std::lerp(x1, x2, v);

    x1 = std::lerp(grad(aab, x  , y  , z-1),
                   grad(bab, x-1, y  , z-1),
                   u);
    x2 = std::lerp(grad(abb, x  , y-1, z-1),
                   grad(bbb, x-1, y-1, z-1),
                   u);
    y2 = std::lerp(x1, x2, v);

    // For convenience, bind the result to 0 - 1 (theoretical min/max before is [-1, 1])
    return (std::lerp(y1, y2, w) +1) / 2;
} 

value_t octave_perlin(value_t x, value_t y, value_t z,
                      uint32_t octaves, value_t persistence)
{
    value_t total = 0;
    value_t frequency = 1;
    value_t amplitude = 1;
    value_t max_value = 0; 

    for(uint32_t i = 0; i < octaves; ++i)
    {
        total += perlin(x * frequency, y * frequency, z * frequency) * amplitude;
        
        max_value += amplitude;
        
        amplitude *= persistence;
        frequency *= 2;
    }
    
    // Normalize to <0.0, 1.0>
    return total / max_value;
}


