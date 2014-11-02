
#ifndef _H_CS499R_COMPILEDSCENE
#define _H_CS499R_COMPILEDSCENE

#include "cs499rSceneMesh.hpp"


namespace CS499R
{

    /*
     * The scene buffer old all the scene information in buffers on the GPU side
     */
    class CompiledScene
    {
    public:
        // --------------------------------------------------------------------- IDLE

        CompiledScene(Scene const * scene, RayTracer const * rayTracer);
        ~CompiledScene();


    private:
        // --------------------------------------------------------------------- MEMBERS

        // the scene source
        Scene const * const mScene;

        // the binded ray tracer
        RayTracer const *  const mRayTracer;

        // the buffers
        struct
        {
            cl_mem primitives;
            cl_mem meshInstances;
            cl_mem meshOctreeNodes;
        } mBuffer;


        // --------------------------------------------------------------------- METHODES

        /*
         * Creates GPU side buffers
         */
        void
        createBuffers();

        /*
         * Creates mBuffer.primitives
         */
        void
        createPrimitivesBuffer(SceneMesh::CompiledSceneCtx & ctx);

        /*
         * Creates mBuffer.meshInstances
         */
        void
        createMeshInstancesBuffer(SceneMesh::CompiledSceneCtx const & ctx);

        /*
         * Creates mBuffer.meshOctreeNodes
         */
        void
        createMeshOctreeNodesBuffer(SceneMesh::CompiledSceneCtx & ctx);

        /*
         * Releases GPU side buffers
         */
        void
        releaseBuffers();


        // --------------------------------------------------------------------- FRIENDSHIP
        friend class RenderState;

    };

}

#endif // _H_CS499R_SCENEBUFFER
