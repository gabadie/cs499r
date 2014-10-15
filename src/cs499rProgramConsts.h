
#ifndef _CLH_CS499R_PROGRAM_CONSTS
#define _CLH_CS499R_PROGRAM_CONSTS

#include "cs499rProgramPrefix.h"


// ----------------------------------------------------------------------------- CONSTANTS

__constant float32_t kEPSILONE = 0.00001f;
__constant float32_t kPi = 3.14159265359f;

__constant uint32_t const kOctreeNodeSubdivisonCount = 8;
__constant uint32_t const kOctreeSubNodeMask = 0x7;
__constant uint32_t const kOctreeNodeStackSize = 32;


#endif
