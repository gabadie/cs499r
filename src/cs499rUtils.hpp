
#ifndef _H_CS499R_UTILS
#define _H_CS499R_UTILS

/*
 * Crashes the program in the state of the art
 */
#define CS499R_CRASH() \
    { \
        * ((volatile int *) 0) = 0; \
    }

/*
 * Crashes the program doesn't verify the given condition
 */
#define CS499R_ASSERT(condition) \
    if (!(condition)) \
    { \
        CS499R_CRASH(); \
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
