
#ifndef _CLH_CS499R_PROGRAM_PREFIX
#define _CLH_CS499R_PROGRAM_PREFIX

#define __CS499R_OPENCL_FILE

#include "cs499rCommonConfig.h"
#include "cs499rCommonConsts.h"
#include "cs499rCommonStruct.hpp"

#ifndef __CS499R_OPENCL_PREPROCESSOR
#define __constant
#define __kernel
#define __global
#define __local
#define __private
#define get_local_id(x) 0
#define get_local_size(x) 0
#define get_group_id(x) 0
#define get_group_size(x) 0
#define get_global_id(x) 0
#define get_global_size(x) 0
#endif


#endif //_CLH_CS499R_PROGRAM_DEBUG
