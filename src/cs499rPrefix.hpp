
#ifndef _H_CS499R_PREFIX
#define _H_CS499R_PREFIX

// ----------------------------------------------------------------------------- Standard libraries

#include <stdint.h>
#include <OpenCL/cl.h>


// ----------------------------------------------------------------------------- CS499R's utils

#include "cs499rMath.hpp"
#include "cs499rUtils.hpp"
#include "cs499rCLUtils.hpp"


// ----------------------------------------------------------------------------- CS499R's classes

namespace CS499R
{

    struct RenderProfiling;

    class Camera;
    class Image;
    class RayTracer;
    class RenderState;
    class RenderTarget;
    class Scene;
    class SceneBuffer;

}

#endif // _H_CS499R_PREFIX
