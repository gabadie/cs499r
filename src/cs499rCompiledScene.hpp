
#ifndef _H_CS499R_COMPILEDSCENE
#define _H_CS499R_COMPILEDSCENE

#include <map>

#include "cs499rPrefix.hpp"


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

        using SceneMeshOffsetMap = std::map<SceneMesh const *, size_t>;

        /*
         * Context when building up CompiledScene
         */
        struct CompilationCtx
        {
            // global primitive offset for each meshes
            SceneMeshOffsetMap meshPrimitivesGlobalOffsets;

            // global node id of the mesh's octree root
            SceneMeshOffsetMap meshOctreeRootGlobalId;
        };


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
        createPrimitivesBuffer(CompilationCtx & compilationCtx);

        /*
         * Creates mBuffer.meshInstances
         */
        void
        createMeshInstancesBuffer(CompilationCtx const & compilationCtx);

        /*
         * Creates mBuffer.meshOctreeNodes
         */
        void
        createMeshOctreeNodesBuffer(CompilationCtx & compilationCtx);

        /*
         * Releases GPU side buffers
         */
        void
        releaseBuffers();


        // --------------------------------------------------------------------- FRIENDSHIP
        friend class RenderState;
        friend class SceneMesh;
        friend class SceneMeshInstance;

    };

}

#endif // _H_CS499R_SCENEBUFFER
