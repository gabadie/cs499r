
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

#endif // _H_CS499R_UTILS
