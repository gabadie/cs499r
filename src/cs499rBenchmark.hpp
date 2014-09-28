
#ifndef _H_CS499R_BENCHMARK
#define _H_CS499R_BENCHMARK

#include <sys/time.h>
#include "cs499rPrefix.hpp"


namespace CS499R
{

    using timestamp_t = uint64_t;

    inline
    timestamp_t
    timestamp()
    {
        struct timeval t;
        gettimeofday(&t, nullptr);
        return timestamp_t(t.tv_sec) * 1000000 + timestamp_t(t.tv_usec);
    }

}


#endif // _H_CS499R_BENCHMARK
