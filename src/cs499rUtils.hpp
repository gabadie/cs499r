
#ifndef _H_CS499R_UTILS
#define _H_CS499R_UTILS

#include <stdio.h>


/*
 * gdb//lldb break point
 */
#define CS499R_BREAK_POINT() \
    __builtin_trap()

/*
 * Crashes the program in the state of the art
 */
#define __CS499R_CRASH() \
    { \
        fflush(stdout); \
        fflush(stderr); \
        * ((volatile int *) 0) = 0; \
    }

#define CS499R_CRASH() \
    { \
        fprintf(stderr, "CRASH(%s:%i) IN `%s`\n", __FILE__, __LINE__, __func__); \
        __CS499R_CRASH(); \
    }

/*
 * Crashes the program doesn't verify the given condition
 */
#define CS499R_ASSERT(condition) \
    if (!(condition)) \
    { \
        fprintf(stderr, "ASSERT(%s:%i) IN `%s`: %s\n", __FILE__, __LINE__, __func__, #condition); \
        __CS499R_CRASH(); \
    }

/*
 * Crashes the program doesn't verify the given condition
 */
#define CS499R_ASSERT_ALIGNMENT(ptr) \
    if (size_t(ptr) % __alignof(*(ptr))) \
    { \
        fprintf(stderr, "MEMORY ALIGNMENT FAILURE(%s:%i) IN `%s`: %s\n", __FILE__, __LINE__, __func__, #ptr); \
        __CS499R_CRASH(); \
    }

/*
 * Transform code line into a C string
 */
#define CS499R_CODE(code) \
    ((char const *) #code)

/*
 * Gets the array's size
 */
#define CS499R_ARRAY_SIZE(array) \
    (sizeof(array) / sizeof((array)[0]))


#endif // _H_CS499R_UTILS
