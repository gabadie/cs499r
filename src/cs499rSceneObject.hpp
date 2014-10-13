
#ifndef _H_CS499R_SCENEOBJECT
#define _H_CS499R_SCENEOBJECT

#include <string>
#include "cs499rPrefix.hpp"


namespace CS499R
{

    /*
     * All scene's object must inherit SceneObject
     */
    class SceneObject
    {
    public:
        // --------------------------------------------------------------------- METHOD

        /*
         * object's retain/release scene references mechanism
         */
        inline
        void
        retain()
        {
            mSceneReferences++;

            CS499R_ASSERT(mSceneReferences > 0);
        }

        inline
        void
        release()
        {
            CS499R_ASSERT(mSceneReferences > 0);

            mSceneReferences--;
        }

        /*
         * Gets the object's scene it is included to
         */
        inline
        Scene *
        scene()
        {
            return mScene;
        }

        /*
         * Gets the object's scene name
         */
        inline
        std::string
        name() const
        {
            return mSceneObjectName;
        }


        // --------------------------------------------------------------------- IDLE

        inline
        SceneObject(Scene * const scene, std::string const & objectName)
            : mScene(scene)
            , mSceneObjectName(objectName)
            , mSceneReferences(0)
        { }

        virtual
        ~SceneObject()
        {
            CS499R_ASSERT(mSceneReferences == 0);
        }


    private:
        // --------------------------------------------------------------------- MEMBERS

        // the object's scene it is into
        Scene * const mScene;

        // scene object's name in the scene
        std::string const mSceneObjectName;

        // number of references in the scene
        size_t mSceneReferences;


        // --------------------------------------------------------------------- FRIENDSHIPS
        friend class Scene;

    };


    /*
     * Pointer to a SceneObject that automatically retain/release it.
     */
    template <typename T>
    class SceneObjectRef
    {
    public:
        // --------------------------------------------------------------------- ACCESS

        inline
        T *
        operator -> () const
        {
            return mObject;
        }

        inline
        T &
        operator * () const
        {
            return *mObject;
        }

        inline
        operator T * () const
        {
            return mObject;
        }

        inline
        SceneObjectRef<T> const &
        operator = (SceneObjectRef<T> const &) = delete;


        // --------------------------------------------------------------------- IDLE

        inline
        SceneObjectRef(T * object)
            : mObject(object)
        {
            CS499R_ASSERT(mObject != nullptr);

            mObject->retain();
        }

        inline
        SceneObjectRef(SceneObjectRef<T> const & objectRef)
            : mObject(objectRef.mObject)
        {
            CS499R_ASSERT(mObject != nullptr);

            mObject->retain();
        }

        inline
        SceneObjectRef(SceneObjectRef<T> const && objectRef)
            : mObject(objectRef.mObject)
        {
            CS499R_ASSERT(mObject != nullptr);
        }

        inline
        ~SceneObjectRef()
        {
            mObject->release();
        }


    private:
        // --------------------------------------------------------------------- MEMBERS

        T * const mObject;

    };

}

#endif // _H_CS499R_SCENEOBJECT
