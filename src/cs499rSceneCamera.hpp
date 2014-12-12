
#ifndef _H_CS499R_SCENECAMERA
#define _H_CS499R_SCENECAMERA

#include "cs499rCamera.hpp"
#include "cs499rSceneObject.hpp"


namespace CS499R
{

    /*
     * Define a camera used in a scene
     */
    class SceneCamera : public SceneObject, public Camera
    {
    public:
        // --------------------------------------------------------------------- OPERATORS

        SceneMesh &
        operator = (SceneMesh const &) = delete;


    private:
        // --------------------------------------------------------------------- IDLE

        inline
        SceneCamera(
            Scene * const scene,
            std::string const & objectName
        ) : SceneObject(scene, objectName)
        {}
        SceneCamera(SceneMesh const &) = delete;


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class Scene;

    };

}

#endif // _H_CS499R_SCENEMESH
