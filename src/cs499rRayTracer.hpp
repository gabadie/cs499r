
#ifndef _H_CS499R_RAYTRACER
#define _H_CS499R_RAYTRACER

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * The ray tracer is actually the GPU program ready to render a 3D scene
     */
    class RayTracer
    {
    public:

        // --------------------------------------------------------------------- IDLE

        RayTracer(cl_device_id const & device);
        ~RayTracer();

        // the = operator is illegal
        void operator = (RayTracer const &) = delete;

        // the copy is also illegal
        RayTracer(RayTracer const &) = delete;


    private:
        // --------------------------------------------------------------------- MEMBERS
        // the GPU's context
        cl_context mContext;

        // the main computing queue
        cl_command_queue mCmdQueue;

#if 0
        // the ray tracing program
        struct
        {
            cl_program program;
            cl_kernel kernel;
        } mProgram;
#endif

    };

}

#endif // _H_CS499R_RAYTRACER
