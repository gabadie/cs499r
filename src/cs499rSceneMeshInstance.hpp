
#ifndef _H_CS499R_SCENEMESHINSTANCE
#define _H_CS499R_SCENEMESHINSTANCE

#include "cs499rSceneObject.hpp"


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


        // --------------------------------------------------------------------- FRIENDSHIP
        friend class Scene;

    };

}

#endif // _H_CS499R_SCENEMESHINSTANCE