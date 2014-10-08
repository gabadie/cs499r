
#ifndef _H_CS499R_MEMORY
#define _H_CS499R_MEMORY

#include <stdlib.h>
#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * Allocates memory with the correct alignment
     */
    template <typename T>
    inline
    T *
    alloc(size_t arraySize)
    {
        CS499R_ASSERT(arraySize != 0);

        T * allocatedMemory = nullptr;

        int error = posix_memalign((void **) &allocatedMemory, __alignof(T), sizeof(T) * arraySize);

        CS499R_ASSERT(error == 0);
        CS499R_ASSERT(allocatedMemory != nullptr);
        CS499R_ASSERT_ALIGNMENT(allocatedMemory);

        return allocatedMemory;
    }

    template <typename T>
    inline
    T *
    alloc()
    {
        return alloc<T>(1);
    }

}


#endif // _H_CS499R_MEMORY
