
#ifndef _H_CS499R_SCENE
#define _H_CS499R_SCENE

#include <map>
#include <vector>
#include <string>
#include "cs499rCommonStruct.hpp"
#include "cs499rSceneCamera.hpp"
#include "cs499rSceneMesh.hpp"
#include "cs499rSceneMeshInstance.hpp"


namespace CS499R
{

    /*
     * The scene old all the scene information on the CPU side
     */
    class Scene
    {
    public:
        // --------------------------------------------------------------------- METHODES

        /*
         * Adds stuff into the scene
         */
        SceneCamera *
        addCamera(std::string const & cameraName)
        {
            auto sceneCamera = new SceneCamera(this, cameraName);

            mObjectsMap.cameras.insert({cameraName, sceneCamera});

            return sceneCamera;
        }

        SceneMesh *
        addMesh(std::string const & meshName, Mesh const & mesh)
        {
            auto sceneMesh = new SceneMesh(this, meshName, mesh);

            mObjectsMap.meshes.insert({meshName, sceneMesh});

            return sceneMesh;
        }

        SceneMeshInstance *
        addMeshInstance(std::string const & meshInstanceName, SceneMesh * sceneMesh)
        {
            CS499R_ASSERT(sceneMesh->mScene == this);

            auto sceneMeshInstance = new SceneMeshInstance(this, meshInstanceName, sceneMesh);

            mObjectsMap.meshInstances.insert({meshInstanceName, sceneMeshInstance});

            return sceneMeshInstance;
        }

        /*
         * Finds stuff in the scene
         */
        SceneCamera *
        findCamera(std::string const & cameraName) const
        {
            auto result = mObjectsMap.cameras.find(cameraName);

            CS499R_ASSERT(result != mObjectsMap.cameras.end());

            return result->second;
        }

        SceneMesh *
        findMesh(std::string const & meshName) const
        {
            auto result = mObjectsMap.meshes.find(meshName);

            CS499R_ASSERT(result != mObjectsMap.meshes.end());

            return result->second;
        }

        SceneMeshInstance *
        findMeshInstance(std::string const & meshName) const
        {
            auto result = mObjectsMap.meshInstances.find(meshName);

            CS499R_ASSERT(result != mObjectsMap.meshInstances.end());

            return result->second;
        }

        /*
         * Returns the scene's bounding box
         */
        void
        computeBoundingBox(float32x3_t * outLowerBound, float32x3_t * outUpperBound) const;


        // --------------------------------------------------------------------- IDLE

        Scene();
        ~Scene();


    private:
        // --------------------------------------------------------------------- STRUCTS

        template <typename T>
        using Map = std::map<std::string, T *>;


        // --------------------------------------------------------------------- MEMBERS

        /*
         * scene's objects' maps
         *
         * Notes: they shall be sorted by order of destruction
         */
        struct
        {
            // all scene's cameras
            Map<SceneCamera> cameras;

            // all scene's mesh instances
            Map<SceneMeshInstance> meshInstances;

            // all scene's meshes
            Map<SceneMesh> meshes;
        } mObjectsMap;


        // --------------------------------------------------------------------- METHODES

        static
        void
        destroyMap(Map<SceneObject> & map);


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class RenderState;
        friend class CompiledScene;

    };

}

#endif // _H_CS499R_SCENE
