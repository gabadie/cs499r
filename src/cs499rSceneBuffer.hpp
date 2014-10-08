
#ifndef _H_CS499R_SCENEBUFFER
#define _H_CS499R_SCENEBUFFER

#include <map>

#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * The scene buffer old all the scene information in buffers on the GPU side
     */
    class SceneBuffer
    {
    public:
        // --------------------------------------------------------------------- IDLE

        SceneBuffer(Scene const * scene, RayTracer const * rayTracer);
        ~SceneBuffer();


    private:
        // --------------------------------------------------------------------- STRUCTS

        using SceneMeshOffsetMap = std::map<SceneMesh *, size_t>;


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
        createPrimitivesBuffer(SceneMeshOffsetMap & meshPrimitivesGlobalOffsets);

        /*
         * Creates mBuffer.meshInstances
         */
        void
        createMeshInstancesBuffer(SceneMeshOffsetMap const & meshPrimitivesGlobalOffsets);

        /*
         * Creates mBuffer.meshOctreeNodes
         */
        void
        createMeshOctreeNodesBuffer(SceneMeshOffsetMap & meshOctreeRootGlobalId);

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
