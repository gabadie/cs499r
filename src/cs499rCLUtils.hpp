
#ifndef _H_CS499R_CLUTILS
#define _H_CS499R_CLUTILS

#include "cs499rPrefix.hpp"


namespace CS499R
{
    /*
     * Gets the OpenCL error name
     */
    char const *
    findClError(cl_int error);
}


#define CS499R_ASSERT_NO_CL_ERROR(error) \
    if (error != CL_SUCCESS) \
    { \
        fprintf(stderr, "OPENCL ERROR(%s:%i) IN `%s`: %s\n", __FILE__, __LINE__, __func__, CS499R::findClError(error)); \
        __CS499R_CRASH(); \
    }


#endif // _H_CS499R_CLUTILS
