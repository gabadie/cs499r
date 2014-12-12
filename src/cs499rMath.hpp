
#ifndef _H_CS499R_MATH
#define _H_CS499R_MATH

#ifndef __CS499R_OPENCL_PREPROCESSOR
/*
 * We are not preprocessing code for OpenCL...
 */

#include <math.h>
#include "cs499rPrefix.hpp"


namespace CS499R
{

    // ------------------------------------------------------------------------- CONSTANTS

    float const kPI = 3.14159265359f;


    // ------------------------------------------------------------------------- VECTORS
    /*
     * 2 dimensions vector
     */
    template <typename T>
    class vec2
    {
    public:
        T x;
        T y;

        inline
        vec2()
        { }

        inline
        vec2(T s)
        {
            x = y = s;
        }

        inline
        vec2(T ix, T iy)
        {
            x = ix;
            y = iy;
        }

        inline
        vec2(vec2<T> const & v)
        {
            x = v.x;
            y = v.y;
        }
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

        inline
        vec3()
        { }

        inline
        vec3(T s)
        {
            x = y = z = s;
        }

        inline
        vec3(T ix, T iy, T iz)
        {
            x = ix;
            y = iy;
            z = iz;
        }

        inline
        vec3(vec3<T> const & v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
        }
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

        inline
        vec4()
        { }

        inline
        vec4(T s)
        {
            x = y = z = w = s;
        }

        inline
        vec4(T ix, T iy, T iz, T iw)
        {
            x = ix;
            y = iy;
            z = iz;
            w = iw;
        }

        inline
        vec4(vec3<T> const & v)
        {
            x = v.x;
            y = v.y;
            z = v.z;
            w = v.w;
        }
    };

}


// ----------------------------------------------------------------------------- MATHEMATICS OPERATOR
#define define_vector_bi_operator(op) \
    template <typename T> \
    inline \
    CS499R::vec2<T> \
    operator op (CS499R::vec2<T> const & a, CS499R::vec2<T> const & b) \
    { \
        return CS499R::vec2<T>(a.x op b.x, a.y op b.y); \
    } \
    \
    template <typename T> \
    inline \
    CS499R::vec2<T> \
    operator op (CS499R::vec2<T> const & a, T const & b) \
    { \
        return CS499R::vec2<T>(a.x op b, a.y op b); \
    } \
    \
    template <typename T> \
    inline \
    CS499R::vec2<T> \
    operator op (T const & a, CS499R::vec2<T> const & b) \
    { \
        return CS499R::vec2<T>(a op b.x, a op b.y); \
    } \
    \
    \
    template <typename T> \
    inline \
    CS499R::vec3<T> \
    operator op (CS499R::vec3<T> const & a, CS499R::vec3<T> const & b) \
    { \
        return CS499R::vec3<T>(a.x op b.x, a.y op b.y, a.z op b.z); \
    } \
    \
    template <typename T> \
    inline \
    CS499R::vec3<T> \
    operator op (CS499R::vec3<T> const & a, T const & b) \
    { \
        return CS499R::vec3<T>(a.x op b, a.y op b, a.z op b); \
    } \
    \
    template <typename T> \
    inline \
    CS499R::vec3<T> \
    operator op (T const & a, CS499R::vec3<T> const & b) \
    { \
        return CS499R::vec3<T>(a op b.x, a op b.y, a op b.z); \
    } \
    \
    \
    template <typename T> \
    inline \
    CS499R::vec4<T> \
    operator op (CS499R::vec4<T> const & a, CS499R::vec4<T> const & b) \
    { \
        return CS499R::vec4<T>(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w); \
    } \
    \
    template <typename T> \
    inline \
    CS499R::vec4<T> \
    operator op (CS499R::vec4<T> const & a, T const & b) \
    { \
        return CS499R::vec4<T>(a.x op b, a.y op b, a.z op b, a.w op b); \
    } \
    \
    template <typename T> \
    inline \
    CS499R::vec4<T> \
    operator op (T const & a, CS499R::vec4<T> const & b) \
    { \
        return CS499R::vec4<T>(a op b.x, a op b.y, a op b.z, a op b.w); \
    } \
    \

#define define_vector_mono_operator(op) \
    template <typename T> \
    inline \
    CS499R::vec2<T> \
    operator op (CS499R::vec2<T> const & v) \
    { \
        return CS499R::vec2<T>(op v.x, op v.y); \
    } \
    \
    \
    template <typename T> \
    inline \
    CS499R::vec3<T> \
    operator op (CS499R::vec3<T> const & v) \
    { \
        return CS499R::vec3<T>(op v.x, op v.y, op v.z); \
    } \
    \
    \
    template <typename T> \
    inline \
    CS499R::vec4<T> \
    operator op (CS499R::vec4<T> const & v) \
    { \
        return CS499R::vec4<T>(op v.x, op v.y, op v.z, op v.w); \
    } \
    \

define_vector_bi_operator(+);
define_vector_bi_operator(-);
define_vector_bi_operator(*);
define_vector_bi_operator(/);
define_vector_bi_operator(%);
define_vector_bi_operator(&);
define_vector_bi_operator(|);
define_vector_bi_operator(^);
define_vector_mono_operator(-);
define_vector_mono_operator(!);


namespace CS499R
{

    // ------------------------------------------------------------------------- ARITHMETICS
    template <typename T>
    constexpr
    inline
    bool
    isPow2(T x)
    {
        return (((x - 1) & x) == 0) && (x != 0);
    }


    // ------------------------------------------------------------------------- DOT PRODUCT
    template <typename T, typename VEC>
    inline
    VEC
    dot(vec2<VEC> const & a, vec2<T> const & b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template <typename T, typename VEC>
    inline
    VEC
    dot(vec3<VEC> const & a, vec3<T> const & b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <typename T, typename VEC>
    inline
    VEC
    dot(vec4<VEC> const & a, vec4<T> const & b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }


    // ------------------------------------------------------------------------- MIN/MAX
    template <typename T>
    inline
    T
    min(T const & a, T const & b)
    {
        return a > b ? b : a;
    }

    template <typename T>
    inline
    vec2<T>
    min(vec2<T> const & a, vec2<T> const & b)
    {
        return vec2<T>(
            min(a.x, b.x),
            min(a.y, b.y)
        );
    }

    template <typename T>
    inline
    vec3<T>
    min(vec3<T> const & a, vec3<T> const & b)
    {
        return vec3<T>(
            min(a.x, b.x),
            min(a.y, b.y),
            min(a.z, b.z)
        );
    }

    template <typename T>
    inline
    vec4<T>
    min(vec4<T> const & a, vec4<T> const & b)
    {
        return vec4<T>(
            min(a.x, b.x),
            min(a.y, b.y),
            min(a.z, b.z),
            min(a.w, b.w)
        );
    }

    template <typename T>
    inline
    T
    min(vec2<T> const & v)
    {
        return min(v.x, v.y);
    }

    template <typename T>
    inline
    T
    min(vec3<T> const & v)
    {
        return min(v.x, min(v.y, v.z));
    }

    template <typename T>
    inline
    T
    min(vec4<T> const & v)
    {
        return min(min(v.x, v.y), min(v.z, v.w));
    }


    template <typename T>
    inline
    T
    max(T const & a, T const & b)
    {
        return a > b ? a : b;
    }

    template <typename T>
    inline
    vec2<T>
    max(vec2<T> const & a, vec2<T> const & b)
    {
        return vec2<T>(
            max(a.x, b.x),
            max(a.y, b.y)
        );
    }

    template <typename T>
    inline
    vec3<T>
    max(vec3<T> const & a, vec3<T> const & b)
    {
        return vec3<T>(
            max(a.x, b.x),
            max(a.y, b.y),
            max(a.z, b.z)
        );
    }

    template <typename T>
    inline
    vec4<T>
    max(vec4<T> const & a, vec4<T> const & b)
    {
        return vec4<T>(
            max(a.x, b.x),
            max(a.y, b.y),
            max(a.z, b.z),
            max(a.w, b.w)
        );
    }

    template <typename T>
    inline
    T
    max(vec2<T> const & v)
    {
        return max(v.x, v.y);
    }

    template <typename T>
    inline
    T
    max(vec3<T> const & v)
    {
        return max(v.x, max(v.y, v.z));
    }

    template <typename T>
    inline
    T
    max(vec4<T> const & v)
    {
        return max(max(v.x, v.y), max(v.z, v.w));
    }


    // ------------------------------------------------------------------------- ABS
    template <typename T>
    inline
    T
    abs(T const v)
    {
        return max(v, -v);
    }

    template <typename T>
    inline
    vec2<T>
    abs(vec2<T> const & v)
    {
        return vec2<T>(
            abs(v.x),
            abs(v.y)
        );
    }

    template <typename T>
    inline
    vec3<T>
    abs(vec3<T> const & v)
    {
        return vec3<T>(
            abs(v.x),
            abs(v.y),
            abs(v.z)
        );
    }

    template <typename T>
    inline
    vec4<T>
    abs(vec4<T> const & v)
    {
        return vec4<T>(
            abs(v.x),
            abs(v.y),
            abs(v.z),
            abs(v.w)
        );
    }




    // ------------------------------------------------------------------------- CROSS PRODUCT
    template <typename T>
    inline
    vec3<T>
    cross(vec3<T> const & a, vec3<T> const & b)
    {
        return vec3<T>(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }


    // ------------------------------------------------------------------------- LENGTH
    template <typename T>
    inline
    T
    length(vec2<T> const & v)
    {
        return ::sqrt(dot(v, v));
    }

    template <typename T>
    inline
    T
    length(vec3<T> const & v)
    {
        return ::sqrt(dot(v, v));
    }

    template <typename T>
    inline
    T
    length(vec4<T> const & v)
    {
        return ::sqrt(dot(v, v));
    }


    // ------------------------------------------------------------------------- NORMALIZE
    template <typename T>
    inline
    T
    normalize(T const & v)
    {
        return v * (1.0f / length(v));
    }


    // ------------------------------------------------------------------------- NORMALIZE
    template <typename T>
    inline
    T
    ceilPow2(T x)
    {
        return T(1 << size_t(ceil(log2(double(x)))));
    }

    template <typename T>
    inline
    T
    ceilSquarePow2(T x)
    {
        T y = ceilPow2(sqrt(double(x)));
        return y * y;
    }

}

// ----------------------------------------------------------------------------- OPENCL COMPATIBILITY
typedef CS499R::vec2<int32_t> int32x2_t;
typedef CS499R::vec3<int32_t> int32x3_t;
typedef CS499R::vec4<int32_t> int32x4_t;

typedef CS499R::vec2<uint32_t> uint32x2_t;
typedef CS499R::vec3<uint32_t> uint32x3_t;
typedef CS499R::vec4<uint32_t> uint32x4_t;

typedef CS499R::vec2<size_t> size2_t;
typedef CS499R::vec3<size_t> size3_t;
typedef CS499R::vec4<size_t> size4_t;

typedef float float32_t;
typedef CS499R::vec2<float32_t> float32x2_t;
typedef CS499R::vec3<float32_t> float32x3_t;
typedef CS499R::vec4<float32_t> float32x4_t;

typedef CS499R::vec2<float32x2_t> float32x2x2_t;
typedef CS499R::vec3<float32x3_t> float32x3x3_t;
typedef CS499R::vec4<float32x4_t> float32x4x4_t;

namespace CS499R
{

    // ------------------------------------------------------------------------- MATRICES
    template <typename T>
    inline
    T
    identity();

    template <>
    inline
    float32x2x2_t
    identity<float32x2x2_t>()
    {
        return float32x2x2_t(
            float32x2_t(1.0f, 0.0f),
            float32x2_t(0.0f, 1.0f)
        );
    }

    template <>
    inline
    float32x3x3_t
    identity<float32x3x3_t>()
    {
        return float32x3x3_t(
            float32x3_t(1.0f, 0.0f, 0.0f),
            float32x3_t(0.0f, 1.0f, 0.0f),
            float32x3_t(0.0f, 0.0f, 1.0f)
        );
    }

    template <>
    inline
    float32x4x4_t
    identity<float32x4x4_t>()
    {
        return float32x4x4_t(
            float32x4_t(1.0f, 0.0f, 0.0f, 0.0f),
            float32x4_t(0.0f, 1.0f, 0.0f, 0.0f),
            float32x4_t(0.0f, 0.0f, 1.0f, 0.0f),
            float32x4_t(0.0f, 0.0f, 0.0f, 1.0f)
        );
    }

    inline
    float32x2x2_t
    transpose(float32x2x2_t const & m)
    {
        return float32x2x2_t(
            float32x2_t(m.x.x, m.y.x),
            float32x2_t(m.x.y, m.y.y)
        );
    }

    inline
    float32x3x3_t
    transpose(float32x3x3_t const & m)
    {
        return float32x3x3_t(
            float32x3_t(m.x.x, m.y.x, m.z.x),
            float32x3_t(m.x.y, m.y.y, m.z.y),
            float32x3_t(m.x.z, m.y.z, m.z.z)
        );
    }

    inline
    float32x4x4_t
    transpose(float32x4x4_t const & m)
    {
        return float32x4x4_t(
            float32x4_t(m.x.x, m.y.x, m.z.x, m.w.x),
            float32x4_t(m.x.y, m.y.y, m.z.y, m.w.y),
            float32x4_t(m.x.z, m.y.z, m.z.z, m.w.z),
            float32x4_t(m.x.w, m.y.w, m.z.w, m.w.w)
        );
    }

}


#else // __CS499R_OPENCL_PREPROCESSOR
/*
 * We are preprocessing code for OpenCL...
 */

typedef char int8_t;
typedef char2 int8x2_t;
typedef char3 int8x3_t;
typedef char4 int8x4_t;

typedef uchar uint8_t;
typedef uchar2 uint8x2_t;
typedef uchar3 uint8x3_t;
typedef uchar4 uint8x4_t;

typedef short int16_t;
typedef short2 int16x2_t;
typedef short3 int16x3_t;
typedef short4 int16x4_t;

typedef ushort uint16_t;
typedef ushort2 uint16x2_t;
typedef ushort3 uint16x3_t;
typedef ushort4 uint16x4_t;

typedef int int32_t;
typedef int2 int32x2_t;
typedef int3 int32x3_t;
typedef int4 int32x4_t;

typedef uint uint32_t;
typedef uint2 uint32x2_t;
typedef uint3 uint32x3_t;
typedef uint4 uint32x4_t;

typedef float float32_t;
typedef float2 float32x2_t;
typedef float3 float32x3_t;
typedef float4 float32x4_t;

#endif // __CS499R_OPENCL_PREPROCESSOR

#endif // _H_CS499R_MATH
