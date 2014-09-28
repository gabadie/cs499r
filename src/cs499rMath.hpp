
#ifndef _H_CS499R_MATH
#define _H_CS499R_MATH

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
    inline
    bool
    isPow2(T x)
    {
        return (x - 1 & x) == 0;
    }


    // ------------------------------------------------------------------------- DOT PRODUCT
    template <typename T>
    inline
    T
    dot(vec2<T> const & a, vec2<T> const & b)
    {
        return a.x * b.x + a.y * b.y;
    }

    template <typename T>
    inline
    T
    dot(vec3<T> const & a, vec3<T> const & b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    template <typename T>
    inline
    T
    dot(vec4<T> const & a, vec4<T> const & b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
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

}


#endif // _H_CS499R_MATH
