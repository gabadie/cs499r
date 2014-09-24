
#ifndef _H_CS499R_MATH
#define _H_CS499R_MATH

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * 2 dimensions vector
     */
    template <typename T>
    class vec2
    {
    public:
        T x;
        T y;

    };

    /*
     * 3 dimensions vector
     */
    template <typename T>
    class vec3
    {
    public:
        T x;
        T y;
        T z;

    };

    /*
     * 4 dimensions vector
     */
    template <typename T>
    class vec4
    {
    public:
        T x;
        T y;
        T z;
        T w;

    };

}

typedef CS499R::vec2<float> float2;
typedef CS499R::vec3<float> float3;
typedef CS499R::vec4<float> float4;


#endif // _H_CS499R_MATH
