
#ifndef _H_CS499R_RAYTRACER
#define _H_CS499R_RAYTRACER

#include "cs499rEnums.hpp"


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
        // --------------------------------------------------------------------- ENUMS

        enum PrivateProgram
        {
            kProgramTragetDownscale = kRayAlgorithmCount,
            kProgramTragetMultiply,

            kProgramCount
        };


        // --------------------------------------------------------------------- STRUCTS

        /*
         * Contain all a program
         */
        struct Program
        {
            // the ray tracing program
            cl_program program;

            // the ray tracing kernel
            cl_kernel kernel;
        };


        // --------------------------------------------------------------------- MEMBERS
        // the GPU's device id
        cl_device_id mDeviceId;

        // the GPU's context
        cl_context mContext;

        // the main computing queue
        cl_command_queue mCmdQueue;

        // the ray tracing kernels
        Program mProgram[kProgramCount];


        // --------------------------------------------------------------------- METHODES
        /*
         * Build programs and its kernels
         */
        void buildPrograms();


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;
        friend class RenderTarget;
        friend class SceneBuffer;

    };

}

#endif // _H_CS499R_RAYTRACER
