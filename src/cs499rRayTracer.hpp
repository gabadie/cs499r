
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
        // the GPU's device id
        cl_device_id mDeviceId;

        // the GPU's context
        cl_context mContext;

        // the main computing queue
        cl_command_queue mCmdQueue;

        // the ray tracing program
        cl_program mProgram;

        // the ray tracing kernels
        struct
        {
            cl_kernel dispatch;
        } mKernel;


        // --------------------------------------------------------------------- METHODES
        /*
         * Build programs and its kernels
         */
        void buildProgram();


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;
        friend class RenderTarget;
        friend class SceneBuffer;

    };

}

#endif // _H_CS499R_RAYTRACER
