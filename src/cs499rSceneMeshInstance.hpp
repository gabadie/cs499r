
#ifndef _H_CS499R_SCENEMESHINSTANCE
#define _H_CS499R_SCENEMESHINSTANCE

#include "cs499rCommonStruct.hpp"
#include "cs499rCompiledScene.hpp"
#include "cs499rSceneMesh.hpp"


namespace CS499R
{

    /*
     * Define a mesh instance used in a scene
     */
    class SceneMeshInstance : public SceneObject
    {
    public:
        // --------------------------------------------------------------------- MEMBERS

        // instance's matrice from mesh coordinate to scene coordinate
        float32x3x3_t mMeshSceneMatrix;

        // instance's mesh's center's position in the scene space
        float32x3_t mScenePosition;

        // assossiated scene mesh
        SceneObjectRef<SceneMesh> const mSceneMesh;

        // mesh instance's colors
        float32x3_t mColorDiffuse;
        float32x3_t mColorEmit;


        // --------------------------------------------------------------------- FUNCTION

        /*
         * Returns the mesh instance's bounding box
         */
        void
        computeBoundingBox(float32x3_t * outLowerBound, float32x3_t * outUpperBound) const;


        // --------------------------------------------------------------------- OPERATORS

        SceneMesh &
        operator = (SceneMesh const &) = delete;


    private:
        // --------------------------------------------------------------------- IDLE

        SceneMeshInstance(
            Scene * const scene,
            std::string const & objectName,
            SceneMesh * sceneMesh
        );
        SceneMeshInstance(SceneMeshInstance const &) = delete;
        ~SceneMeshInstance();


        // --------------------------------------------------------------------- METHODS

        /*
         * Export to common mesh instance
         */
        void
        exportToCommonMeshInstance(
            CompiledScene::CompilationCtx const & compilationCtx,
            common_mesh_instance_t * outMeshInstance
        ) const;

        /*
         * Export the anonymous mesh instance to a common mesh instance
         */
        static
        void
        exportAnonymousToCommonMeshInstance(
            common_mesh_instance_t * outAnonymousMesh
        );


        // --------------------------------------------------------------------- FRIENDSHIP
        friend class Scene;
        friend class CompiledScene;

    };

}

#endif // _H_CS499R_SCENEMESHINSTANCE
