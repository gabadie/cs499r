
#ifndef _H_CS499R_ENUMS
#define _H_CS499R_ENUMS

#include "cs499rPrefix.hpp"


namespace CS499R
{
    // ------------------------------------------------------------------------- ENUMS

    /*
     * Selects the different ray tracer algorithm.
     */
    enum RayAlgorithm
    {
        kRayAlgorithmPathTracer,
        kRayAlgorithmDebugNormal,

        kRayAlgorithmCount
    };

}

#endif //_H_CS499R_ENUMS
