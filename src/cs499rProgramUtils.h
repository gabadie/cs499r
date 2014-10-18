
#ifndef _CLH_CS499R_PROGRAM_UTILS
#define _CLH_CS499R_PROGRAM_UTILS

#include "cs499rProgramSampleContext.h"


// ----------------------------------------------------------------------------- CONSTANTS

// the latin hyper cube size
__constant uint32_t const kLatinHypercubeSize = 16;

/*
 * The orthogonal latin hypercube A can be generated from B (kLatinHypercubeBiject)
 * as follow:
 *
 *      A(x, y) = z <=> B(z, y) = x
 */
__constant uint8_t const kLatinHypercubeBiject[kLatinHypercubeSize * kLatinHypercubeSize] = {
    10,  2,  12,  7,  3,  5,  14,  11,  8,  1,  15,  6,  9,  13,  0,  4,
    2,  10,  7,  12,  11,  14,  5,  3,  1,  8,  6,  15,  0,  4,  9,  13,
    7,  12,  2,  10,  14,  11,  3,  5,  6,  15,  1,  8,  4,  0,  13,  9,
    12,  7,  10,  2,  5,  3,  11,  14,  15,  6,  8,  1,  13,  9,  4,  0,
    9,  0,  13,  4,  1,  6,  15,  8,  11,  3,  14,  5,  10,  12,  2,  7,
    0,  9,  4,  13,  8,  15,  6,  1,  3,  11,  5,  14,  2,  7,  10,  12,
    4,  13,  0,  9,  15,  8,  1,  6,  5,  14,  3,  11,  7,  2,  12,  10,
    13,  4,  9,  0,  6,  1,  8,  15,  14,  5,  11,  3,  12,  10,  7,  2,
    8,  1,  15,  6,  0,  4,  13,  9,  10,  2,  12,  7,  11,  14,  3,  5,
    1,  8,  6,  15,  9,  13,  4,  0,  2,  10,  7,  12,  3,  5,  11,  14,
    6,  15,  1,  8,  13,  9,  0,  4,  7,  12,  2,  10,  5,  3,  14,  11,
    15,  6,  8,  1,  4,  0,  9,  13,  12,  7,  10,  2,  14,  11,  5,  3,
    11,  3,  14,  5,  2,  7,  12,  10,  9,  0,  13,  4,  8,  15,  1,  6,
    3,  11,  5,  14,  10,  12,  7,  2,  0,  9,  4,  13,  1,  6,  8,  15,
    5,  14,  3,  11,  12,  10,  2,  7,  4,  13,  0,  9,  6,  1,  15,  8,
    14,  5,  11,  3,  7,  2,  10,  12,  13,  4,  9,  0,  15,  8,  6,  1,
};


// ----------------------------------------------------------------------------- FUNCTIONS

inline
float32_t
noise(uint32_t seed)
{
    float32_t s = sin((float32_t)seed * 12.9898f) * 43758.5453f;

    return s - floor(s);
}

inline
float32_t
random(sample_context_t * sampleCx)
{
    uint32_t seed = sampleCx->randomSeed++;

    return noise(seed);
}

#endif
