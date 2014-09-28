
#include "cs499rScene.hpp"

namespace CS499R
{

    Scene::Scene()
    {

    }

    Scene::~Scene()
    {
        auto map = reinterpret_cast<Map<SceneObject> *>(&mObjectsMap);

        for (size_t i = 0; i < sizeof(mObjectsMap) / sizeof(map[0]); i++)
        {
            destroyMap(map[i]);
        }
    }

    void
    Scene::destroyMap(Map<SceneObject> & map)
    {
        for (auto it : map)
        {
            delete it.second;
        }

        map.clear();
    }


}
