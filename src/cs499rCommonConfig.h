
#ifndef _H_CS499R_COMMON_CONFIG
#define _H_CS499R_COMMON_CONFIG

/*
 * Caution: this file will be also included in OpenCL programs' preprocessor
 */

/*
 * Configures the pixels gathering strategy per warp
 */
#define CS499R_CONFIG_PIXELPOS_DUMMY 0
#define CS499R_CONFIG_PIXELPOS_CPT 1
#define CS499R_CONFIG_PIXELPOS_ICPT 2

#ifndef CS499R_CONFIG_PIXELPOS
# define CS499R_CONFIG_PIXELPOS CS499R_CONFIG_PIXELPOS_ICPT
#endif

#endif // _H_CS499R_COMMON_CONFIG
