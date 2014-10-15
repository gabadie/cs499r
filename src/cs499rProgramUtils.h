
#ifndef _CLH_CS499R_PROGRAM_UTILS
#define _CLH_CS499R_PROGRAM_UTILS

#include "cs499rProgramSampleContext.h"


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
